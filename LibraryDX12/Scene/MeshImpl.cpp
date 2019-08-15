#include "MeshImpl.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "Utility/ThrowIfFailed.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>

namespace DXD {

std::unique_ptr<Mesh> Mesh::createFromObj(DXD::Application &application, const std::wstring &filePath, bool useTextures) {
    return std::unique_ptr<Mesh>(new MeshImpl(*static_cast<ApplicationImpl *>(&application), filePath, useTextures));
}
} // namespace DXD

MeshImpl::MeshImpl(ApplicationImpl &application, const std::wstring &filePath, bool useTextures)
    : application(application) {

    auto task = [this, &application, filePath, useTextures]() {
        loadAndUploadObj(application, filePath, useTextures);
    };
    application.getBackgroundWorkerManager().pushTask(task, this->loadingComplete);
}

void MeshImpl::loadAndUploadObj(ApplicationImpl &application, const std::wstring &filePath, bool useTextures) {
    const LoadResults loadResults = loadObj(filePath, useTextures);
    const auto meshType = loadResults.meshType;
    assert(meshType != UNKNOWN);
    const auto &vertexElements = loadResults.vertexElements;
    const auto &indices = loadResults.indices;
    const auto vertexSizeInBytes = computeVertexSize(loadResults.meshType);
    const auto verticesCount = static_cast<UINT>(vertexElements.size() * sizeof(FLOAT) / vertexSizeInBytes);
    assert(vertexSizeInBytes * verticesCount == vertexElements.size() * sizeof(FLOAT));
    const auto indicesCount = static_cast<UINT>(indices.size());
    const auto pipelineStateIdentifier = computePipelineStateIdentifier(meshType);
    const auto shadowMapPipelineStateIdentifier = computeShadowMapPipelineStateIdentifier(meshType);

    UploadResults uploadResults = uploadToGPU(application, loadResults.vertexElements, loadResults.indices, verticesCount, vertexSizeInBytes);
    auto &vertexBuffer = uploadResults.vertexBuffer;
    auto &indexBuffer = uploadResults.indexBuffer;

    setData(meshType, vertexSizeInBytes, verticesCount, indicesCount,
            pipelineStateIdentifier, shadowMapPipelineStateIdentifier,
            std::move(vertexBuffer), std::move(indexBuffer));
}

MeshImpl::LoadResults MeshImpl::loadObj(const std::wstring &filePath, bool useTextures) {
    LoadResults result{};

    // Initial validation
    const auto fullFilePath = std::wstring{RESOURCES_PATH} + filePath;
    std::fstream inputFile{fullFilePath, std::ios::in};
    if (!inputFile.good()) {
        return {};
    }

    // Temporary variables for loading
    std::vector<FLOAT> vertexElements;     // vertex element is e.g x coordinate of vertex position
    std::vector<std::string> indexTokens;  // index token is a bundle of 1-based indices of vertex/normal/uv delimeted by slash, e.g. 1//2, 1/3/21
    std::vector<FLOAT> normalCoordinates;  // normal coordinate is e.g. x coordinate of a normal vector
    std::vector<FLOAT> textureCoordinates; // normal coordinate is e.g. u coordinate of a texture coordinate
    std::string lineType;
    FLOAT x, y, z;
    std::string f1, f2, f3, f4;

    // Read all lines
    for (std::string line; getline(inputFile, line).good();) {
        std::istringstream strs(line);
        strs >> lineType;

        if (lineType == "v") { //vertices
            strs >> x;
            strs >> y;
            strs >> z;
            vertexElements.push_back(x);
            vertexElements.push_back(y);
            vertexElements.push_back(z);
        } else if (lineType == "f") { //faces
            strs >> f1;
            strs >> f2;
            strs >> f3;
            indexTokens.push_back(f1);
            indexTokens.push_back(f2);
            indexTokens.push_back(f3);

            try { // TODO
                const auto slashCount = std::count(line.begin(), line.end(), '/');
                if (slashCount > 6 && strs.good() && !strs.eof()) {
                    strs >> f4;
                    if (!f4.empty()) {
                        indexTokens.push_back(f1);
                        indexTokens.push_back(f3);
                        indexTokens.push_back(f4);
                    } else {
                        int t = 0;
                    }
                } else {
                    int r = 0;
                }
            } catch (...) {
            }
        } else if (lineType == "vn") { //normal vector
            strs >> x;
            strs >> y;
            strs >> z;
            normalCoordinates.push_back(x);
            normalCoordinates.push_back(y);
            normalCoordinates.push_back(z);
        } else if (lineType == "vt") { //texture vector
            strs >> x;
            strs >> y;
            strs >> z;
            textureCoordinates.push_back(x);
            textureCoordinates.push_back(y);
        }
    }

    // Compute some fields based on lines that were read
    result.meshType = MeshImpl::computeMeshType(normalCoordinates, textureCoordinates, useTextures);
    const bool usesIndexBuffer = normalCoordinates.size() == 0 && textureCoordinates.size() == 0;
    if (result.meshType == MeshImpl::UNKNOWN) {
        return {};
    }

    // Prepare final buffers for GPU upload
    if (usesIndexBuffer) {
        // No vertex elements interleaving is required - we only have position
        result.vertexElements = std::move(vertexElements);
    }
    for (std::string face : indexTokens) {
        UINT vertexElementIdx;
        UINT normalIdx;
        UINT textCoordIdx;
        size_t pos = 0;
        std::string f = face;
        for (int i = 0; i < 3; i++) {
            pos = f.find("/");
            std::string t = f.substr(0, pos);
            switch (i) {
            case 0:
                vertexElementIdx = std::stoi(t);
                break;
            case 1:
                if (t.length() > 0) {
                    textCoordIdx = std::stoi(t);
                }
                break;
            case 2:
                if (t.length() > 0) {
                    normalIdx = std::stoi(t);
                }
                break;
            }
            f.erase(0, pos + 1);
        }
        if (usesIndexBuffer) {
            // Vertices go unmodified to the vertex buffer, we use an index buffer to define polygons
            result.indices.push_back(vertexElementIdx - 1);
        } else {
            // We have to interleave vertex attributes gather in different arrays into one
            result.vertexElements.push_back(vertexElements[3 * (vertexElementIdx - 1)]);
            result.vertexElements.push_back(vertexElements[3 * (vertexElementIdx - 1) + 1]);
            result.vertexElements.push_back(vertexElements[3 * (vertexElementIdx - 1) + 2]);
            if (result.meshType & MeshImpl::NORMALS) {
                result.vertexElements.push_back(normalCoordinates[3 * (normalIdx - 1)]);
                result.vertexElements.push_back(normalCoordinates[3 * (normalIdx - 1) + 1]);
                result.vertexElements.push_back(normalCoordinates[3 * (normalIdx - 1) + 2]);
            }
            if (result.meshType & MeshImpl::TEXTURE_COORDS) {
                result.vertexElements.push_back(textureCoordinates[2 * (textCoordIdx - 1)]);
                result.vertexElements.push_back(textureCoordinates[2 * (textCoordIdx - 1) + 1]);
            }
        }
    }

    return std::move(result);
}

bool MeshImpl::isUploadInProgress() {
    if (!loadingComplete.load()) {
        return false;
    }

    const bool vertexInProgress = this->vertexBuffer->isUploadInProgress();
    const bool indexInProgress = this->indexBuffer != nullptr && this->indexBuffer->isUploadInProgress();
    return vertexInProgress || indexInProgress;
}

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
    map[TRIANGLE_STRIP] = PipelineStateController::Identifier::PIPELINE_STATE_DEFAULT;
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

MeshImpl::UploadResults MeshImpl::uploadToGPU(ApplicationImpl &application, const std::vector<FLOAT> &vertexElements, const std::vector<UINT> &indices, UINT verticesCount, UINT vertexSizeInBytes) {
    UploadResults results = {};
    const bool useIndexBuffer = indices.size() > 0;

    // Context
    ID3D12DevicePtr device = application.getDevice();
    auto &commandQueue = application.getCopyCommandQueue();

    // Record command list for GPU upload
    CommandList commandList{application.getDescriptorController(), commandQueue.getCommandAllocatorManager(), nullptr};
    results.vertexBuffer = std::make_unique<VertexBuffer>(device, commandList, vertexElements.data(), verticesCount, vertexSizeInBytes);
    if (useIndexBuffer) {
        results.indexBuffer = std::make_unique<IndexBuffer>(device, commandList, indices.data(), static_cast<UINT>(indices.size()));
    }
    commandList.close();

    // Execute and register obtained allocator and lists to the manager
    std::vector<CommandList *> commandLists{&commandList};
    const uint64_t fenceValue = commandQueue.executeCommandListsAndSignal(commandLists);

    // Register upload status for buffers
    results.vertexBuffer->registerUpload(commandQueue, fenceValue);
    if (useIndexBuffer) {
        results.indexBuffer->registerUpload(commandQueue, fenceValue);
    }

    return results;
}

void MeshImpl::setData(MeshType meshType, UINT vertexSizeInBytes, UINT verticesCount, UINT indicesCount,
                       PipelineStateController::Identifier pipelineStateIdentifier, PipelineStateController::Identifier shadowMapPipelineStateIdentifier,
                       std::unique_ptr<VertexBuffer> &&vertexBuffer, std::unique_ptr<IndexBuffer> &&indexBuffer) {
    this->meshType = meshType;
    this->vertexSizeInBytes = vertexSizeInBytes;
    this->verticesCount = verticesCount;
    this->indicesCount = indicesCount;
    this->pipelineStateIdentifier = pipelineStateIdentifier;
    this->shadowMapPipelineStateIdentifier = shadowMapPipelineStateIdentifier;
    this->vertexBuffer = std::move(vertexBuffer);
    this->indexBuffer = std::move(indexBuffer);
}
