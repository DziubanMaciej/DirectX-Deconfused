#include "MeshImpl.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "Utility/ThrowIfFailed.h"

#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>

namespace DXD {

// ----------------------------------------------------------------- Creation and destruction

std::unique_ptr<Mesh> Mesh::createFromObj(DXD::Application &application, const std::wstring &filePath,
                                          bool useTextures, bool asynchronousLoading) {
    return std::unique_ptr<Mesh>(new MeshImpl(*static_cast<ApplicationImpl *>(&application),
                                              filePath, useTextures, asynchronousLoading));
}
} // namespace DXD

MeshImpl::MeshImpl(ApplicationImpl &application, const std::wstring &filePath, bool useTextures, bool asynchronousLoading)
    : application(application) {
    const MeshCpuLoadArgs cpuLoadArgs{filePath, useTextures};
     asynchronousLoading = false;
    cpuGpuLoad(cpuLoadArgs, asynchronousLoading);
}

MeshImpl::~MeshImpl() {
    terminateBackgroundProcessing(true);
}

// ----------------------------------------------------------------- AsyncLoadableObject overrides

#include <chrono>
MeshCpuLoadResult MeshImpl::cpuLoad(const MeshCpuLoadArgs &args) {
    auto start = std::chrono::steady_clock::now();

    // TODO dirty conversion to stdstring
    const auto fullFilePathW = std::wstring{RESOURCES_PATH} + args.filePath;
    std::string fullFilePath;
    fullFilePath.reserve(fullFilePathW.size());
    for (const auto &w : fullFilePathW) {
        fullFilePath.push_back((char)w);
    }

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(fullFilePath, aiProcess_Triangulate);

    assert(!args.useTextures || scene->mMeshes[0]->HasNormals());

    auto meshType = TRIANGLE_STRIP;
    if (scene->mMeshes[0]->HasNormals()) {
        meshType |= NORMALS;
    }
    if (args.useTextures) {
        meshType |= TEXTURE_COORDS;
    }

    MeshCpuLoadResult result = {};
    result.meshType = meshType;
    result.vertexSizeInBytes = computeVertexSize(result.meshType);

    for (auto meshIndex = 0u; meshIndex < scene->mNumMeshes; meshIndex++) {
        const aiMesh *mesh = scene->mMeshes[meshIndex];

        assert(!args.useTextures || mesh->HasNormals());

        auto meshType = TRIANGLE_STRIP;
        if (mesh->HasNormals()) {
            meshType |= NORMALS;
        }
        if (args.useTextures) {
            meshType |= TEXTURE_COORDS;
        }
        assert(result.meshType = meshType);

       
        result.verticesCount += mesh->mNumVertices;

        const auto vertexSizeInComponents = result.vertexSizeInBytes / sizeof(FLOAT);
        const auto vertexElementsCount = mesh->mNumVertices * vertexSizeInComponents;
        result.vertexElements.reserve(result.vertexElements.capacity() + vertexElementsCount);
        result.indices.reserve(result.indices.capacity() + mesh->mNumFaces * 3);
        for (auto i = 0u; i < mesh->mNumVertices; i++) {
            result.vertexElements.push_back(mesh->mVertices[i].x);
            result.vertexElements.push_back(mesh->mVertices[i].y);
            result.vertexElements.push_back(mesh->mVertices[i].z);
            if (mesh->HasNormals()) {
                result.vertexElements.push_back(mesh->mNormals[i].x);
                result.vertexElements.push_back(mesh->mNormals[i].y);
                result.vertexElements.push_back(mesh->mNormals[i].z);
            }
            if (args.useTextures) {
                result.vertexElements.push_back(mesh->mTextureCoords[0][i].x);
                result.vertexElements.push_back(mesh->mTextureCoords[0][i].y);
            }
        }
        for (auto i = 0u; i < mesh->mNumFaces; i++) {
            result.indices.push_back(mesh->mFaces[i].mIndices[0]);
            result.indices.push_back(mesh->mFaces[i].mIndices[1]);
            result.indices.push_back(mesh->mFaces[i].mIndices[2]);
        }
    }

    auto end = std::chrono::steady_clock::now();
    auto time = end - start;
    DXD::log("Loaded in %dms\n", std::chrono::duration_cast<std::chrono::milliseconds>(time).count());

    return std::move(result);
}

bool MeshImpl::isCpuLoadSuccessful(const MeshCpuLoadResult &result) {
    return result.meshType != UNKNOWN;
}

MeshGpuLoadArgs MeshImpl::createArgsForGpuLoad(const MeshCpuLoadResult &cpuLoadResult) {
    return std::move(MeshGpuLoadArgs{
        cpuLoadResult.vertexElements,
        cpuLoadResult.indices,
        cpuLoadResult.verticesCount,
        cpuLoadResult.vertexSizeInBytes});
}

MeshGpuLoadResult MeshImpl::gpuLoad(const MeshGpuLoadArgs &args) {
    MeshGpuLoadResult results = {};
    const bool useIndexBuffer = args.indices.size() > 0;

    // Context
    ID3D12DevicePtr device = application.getDevice();
    auto &commandQueue = application.getCopyCommandQueue();

    // Record command list for GPU upload
    CommandList commandList{commandQueue};
    results.vertexBuffer = std::make_unique<VertexBuffer>(device, commandList, args.vertexElements.data(), args.verticesCount, args.vertexSizeInBytes);
    if (useIndexBuffer) {
        results.indexBuffer = std::make_unique<IndexBuffer>(device, commandList, args.indices.data(), static_cast<UINT>(args.indices.size()));
    }
    commandList.close();

    // Execute and register obtained allocator and lists to the manager
    const uint64_t fenceValue = commandQueue.executeCommandListAndSignal(commandList);

    // Register upload status for buffers
    results.vertexBuffer->registerUpload(commandQueue, fenceValue);
    if (useIndexBuffer) {
        results.indexBuffer->registerUpload(commandQueue, fenceValue);
    }

    return std::move(results);
}

void MeshImpl::writeCpuGpuLoadResults(MeshCpuLoadResult &cpuLoadResult, MeshGpuLoadResult &gpuLoadResult) {
    this->meshType = cpuLoadResult.meshType;
    this->vertexSizeInBytes = cpuLoadResult.vertexSizeInBytes;
    this->verticesCount = cpuLoadResult.verticesCount;
    this->indicesCount = static_cast<UINT>(cpuLoadResult.indices.size());
    this->pipelineStateIdentifier = computePipelineStateIdentifier(meshType);
    this->shadowMapPipelineStateIdentifier = computeShadowMapPipelineStateIdentifier(meshType);
    this->vertexBuffer = std::move(gpuLoadResult.vertexBuffer);
    this->indexBuffer = std::move(gpuLoadResult.indexBuffer);
}

// ----------------------------------------------------------------- Getters

bool MeshImpl::isUploadInProgress() {
    if (!cpuLoadComplete.load()) {
        return true;
    }

    const bool vertexInProgress = this->vertexBuffer->isUploadInProgress();
    const bool indexInProgress = this->indexBuffer != nullptr && this->indexBuffer->isUploadInProgress();
    return vertexInProgress || indexInProgress;
}

// ----------------------------------------------------------------- Helpers

MeshImpl::MeshType MeshImpl::computeMeshType(const std::vector<FLOAT> &normals, const std::vector<FLOAT> &textureCoordinates, bool useTextures) {
    MeshType meshType = TRIANGLE_STRIP;
    if (normals.size() > 0) {
        meshType |= NORMALS;
    }
    if (useTextures) {
        if (textureCoordinates.size() > 0) {
            meshType |= TEXTURE_COORDS;
        } else {
            return UNKNOWN; // user wants texture coords which .obj doesn't provide - error
        }
    }

    return meshType;
}

UINT MeshImpl::computeVertexSize(MeshType meshType) {
    UINT vertexSize = 0;
    if (meshType & TRIANGLE_STRIP) {
        vertexSize += 3;
    }
    if (meshType & TEXTURE_COORDS) {
        vertexSize += 2;
    }
    if (meshType & NORMALS) {
        vertexSize += 3;
    }
    return vertexSize * sizeof(FLOAT);
}

std::map<MeshImpl::MeshType, PipelineStateController::Identifier> MeshImpl::getPipelineStateIdentifierMap() {
    std::map<MeshImpl::MeshType, PipelineStateController::Identifier> map = {};
    map[TRIANGLE_STRIP | NORMALS] = PipelineStateController::Identifier::PIPELINE_STATE_NORMAL;
    map[TRIANGLE_STRIP | NORMALS | TEXTURE_COORDS] = PipelineStateController::Identifier::PIPELINE_STATE_TEXTURE_NORMAL;
    return std::move(map);
}

PipelineStateController::Identifier MeshImpl::computePipelineStateIdentifier(MeshType meshType) {
    static const auto map = getPipelineStateIdentifierMap();
    auto it = map.find(meshType);
    if (it == map.end()) {
        UNREACHABLE_CODE();
    }
    return it->second;
}

std::map<MeshImpl::MeshType, PipelineStateController::Identifier> MeshImpl::getShadowMapPipelineStateIdentifierMap() {
    std::map<MeshImpl::MeshType, PipelineStateController::Identifier> map = {};
    map[TRIANGLE_STRIP | NORMALS] = PipelineStateController::Identifier::PIPELINE_STATE_SM_NORMAL;
    map[TRIANGLE_STRIP | NORMALS | TEXTURE_COORDS] = PipelineStateController::Identifier::PIPELINE_STATE_SM_TEXTURE_NORMAL;
    return std::move(map);
}

PipelineStateController::Identifier MeshImpl::computeShadowMapPipelineStateIdentifier(MeshType meshType) {
    static const auto map = getShadowMapPipelineStateIdentifierMap();
    auto it = map.find(meshType);
    if (it == map.end()) {
        return PipelineStateController::Identifier::PIPELINE_STATE_UNKNOWN;
    }
    return it->second;
}
