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

std::unique_ptr<Mesh> Mesh::createFromObj(DXD::Application &application, const std::string &filePath, bool useTextures) {
    return std::unique_ptr<Mesh>(new MeshImpl(*static_cast<ApplicationImpl *>(&application), filePath, useTextures));
}
} // namespace DXD

MeshImpl::MeshImpl(ApplicationImpl &application, const std::string &filePath, bool useTextures)
    : application(application) {
    loadAndUploadObj(application, filePath, useTextures);
}

void MeshImpl::loadAndUploadObj(ApplicationImpl &application, const std::string &filePath, bool useTextures) {
    const LoadResults loadResults = loadObj(filePath, useTextures);
    const auto meshType = loadResults.meshType;
    assert(meshType != UNKNOWN);
    const auto &vertexElements = loadResults.vertices;
    const auto &indices = loadResults.indices;
    const auto vertexSizeInBytes = computeVertexSize(loadResults.meshType);
    const auto verticesCount = static_cast<UINT>(vertexElements.size() * sizeof(FLOAT) / vertexSizeInBytes);
    assert(vertexSizeInBytes * verticesCount == vertexElements.size() * sizeof(FLOAT));
    const auto indicesCount = static_cast<UINT>(indices.size());

    UploadResults uploadResults = uploadToGPU(application, loadResults.vertices, loadResults.indices, verticesCount, vertexSizeInBytes);
    auto &vertexBuffer = uploadResults.vertexBuffer;
    auto &indexBuffer = uploadResults.indexBuffer;

    setData(meshType, vertexSizeInBytes, verticesCount, indicesCount, std::move(vertexBuffer), std::move(indexBuffer));
}

MeshImpl::LoadResults MeshImpl::loadObj(const std::string &filePath, bool useTextures) {

    const auto fullFilePath = std::string{RESOURCES_PATH} + filePath;
    std::fstream inputFile{fullFilePath, std::ios::in};
    if (!inputFile.good()) {
        return {};
    }

    std::vector<FLOAT> vertexElements;     //temp
    std::vector<std::string> faces;        //temp
    std::vector<FLOAT> normals;            //temp
    std::vector<FLOAT> textureCoordinates; //temp

    std::string lineType;
    FLOAT x, y, z;
    std::string f1, f2, f3, f4;

    std::string line;
    while (getline(inputFile, line).good()) {

        std::istringstream strs(line);

        strs >> lineType;

        if (lineType == "#") { //comment
            continue;
        } else if (lineType == "v") { //vertices
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
            faces.push_back(f1);
            faces.push_back(f2);
            faces.push_back(f3);

            try {
                const auto slashCount = std::count(line.begin(), line.end(), '/');
                if (slashCount > 6 && strs.good() && !strs.eof()) {

                    strs >> f4;
                    if (!f4.empty()) {
                        faces.push_back(f1);
                        faces.push_back(f3);
                        faces.push_back(f4);
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
            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);
        } else if (lineType == "vt") { //texture vector
            strs >> x;
            strs >> y;
            strs >> z;
            textureCoordinates.push_back(x);
            textureCoordinates.push_back(y);
            textureCoordinates.push_back(z);
        }
    }

    const auto meshType = MeshImpl::computeMeshType(normals, textureCoordinates, useTextures);
    if (meshType == MeshImpl::UNKNOWN) {
        return {};
    }

    // No need to rearrange vertices if there are no additional elements in them
    const bool usesIndexBuffer = normals.size() == 0 && textureCoordinates.size() == 0;
    std::vector<FLOAT> outVertexElements = {};
    std::vector<UINT> indices = {};
    if (usesIndexBuffer) {
        outVertexElements = std::move(vertexElements);
    }
    for (std::string face : faces) {
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
            indices.push_back(vertexElementIdx - 1);
        } else {
            // We have to embed all vertex attributes into one array
            outVertexElements.push_back(vertexElements[3 * (vertexElementIdx - 1)]);
            outVertexElements.push_back(vertexElements[3 * (vertexElementIdx - 1) + 1]);
            outVertexElements.push_back(vertexElements[3 * (vertexElementIdx - 1) + 2]);
            if (meshType & MeshImpl::NORMALS) {
                outVertexElements.push_back(normals[3 * (normalIdx - 1)]);
                outVertexElements.push_back(normals[3 * (normalIdx - 1) + 1]);
                outVertexElements.push_back(normals[3 * (normalIdx - 1) + 2]);
            }
            if (meshType & MeshImpl::TEXTURE_COORDS) {
                outVertexElements.push_back(textureCoordinates[3 * (textCoordIdx - 1)]);
                outVertexElements.push_back(textureCoordinates[3 * (textCoordIdx - 1) + 1]);
                outVertexElements.push_back(textureCoordinates[3 * (textCoordIdx - 1) + 2]);
            }
        }
    }

    LoadResults result = {};
    result.meshType = meshType;
    result.vertices = std::move(outVertexElements);
    result.indices = std::move(indices);
    return std::move(result);
}

bool MeshImpl::isUploadInProgress() {
    const bool vertexInProgress = this->vertexBuffer->isUploadInProgress();
    const bool indexInProgress = this->indexBuffer != nullptr && this->indexBuffer->isUploadInProgress();
    const bool inProgress = vertexInProgress || indexInProgress;
    return inProgress;
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
        vertexSize += 3;
    }
    if (meshType & NORMALS) {
        vertexSize += 3;
    }
    return vertexSize * sizeof(FLOAT);
}

MeshImpl::UploadResults MeshImpl::uploadToGPU(ApplicationImpl &application, const std::vector<FLOAT> &vertexElements, const std::vector<UINT> &indices, UINT verticesCount, UINT vertexSizeInBytes) {
    UploadResults results = {};
    const bool useIndexBuffer = indices.size() > 0;

    // Context
    ID3D12DevicePtr device = application.getDevice();
    auto &commandQueue = application.getCopyCommandQueue();

    // Record command list for GPU upload
    CommandList commandList{commandQueue.getCommandAllocatorManager(), nullptr};
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
                       std::unique_ptr<VertexBuffer> &&vertexBuffer, std::unique_ptr<IndexBuffer> &&indexBuffer) {
    this->meshType = meshType;
    this->vertexSizeInBytes = vertexSizeInBytes;
    this->verticesCount = verticesCount;
    this->indicesCount = indicesCount;
    this->vertexBuffer = std::move(vertexBuffer);
    this->indexBuffer = std::move(indexBuffer);
}
