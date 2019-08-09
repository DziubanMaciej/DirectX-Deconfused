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
    const auto fullFilePath = std::string{RESOURCES_PATH} + filePath;
    std::fstream inputFile{fullFilePath, std::ios::in};
    if (!inputFile.good()) {
        return nullptr;
    }

    std::vector<FLOAT> vertexElements;     //temp
    std::vector<UINT> indices;             //optional
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
        return nullptr;
    }

    // No need to rearrange vertices if there are no additional elements in them
    const bool usesIndexBuffer = normals.size() == 0 && textureCoordinates.size() == 0;
    std::vector<FLOAT> outputVertexElements;
    if (usesIndexBuffer) {
        outputVertexElements = std::move(vertexElements);
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
            outputVertexElements.push_back(vertexElements[3 * (vertexElementIdx - 1)]);
            outputVertexElements.push_back(vertexElements[3 * (vertexElementIdx - 1) + 1]);
            outputVertexElements.push_back(vertexElements[3 * (vertexElementIdx - 1) + 2]);
            if (meshType & MeshImpl::NORMALS) {
                outputVertexElements.push_back(normals[3 * (normalIdx - 1)]);
                outputVertexElements.push_back(normals[3 * (normalIdx - 1) + 1]);
                outputVertexElements.push_back(normals[3 * (normalIdx - 1) + 2]);
            }
            if (meshType & MeshImpl::TEXTURE_COORDS) {
                outputVertexElements.push_back(textureCoordinates[3 * (textCoordIdx - 1)]);
                outputVertexElements.push_back(textureCoordinates[3 * (textCoordIdx - 1) + 1]);
                outputVertexElements.push_back(textureCoordinates[3 * (textCoordIdx - 1) + 2]);
            }
        }
    }

    const auto vertexSizeInBytes = MeshImpl::computeVertexSize(meshType);
    return std::unique_ptr<Mesh>{new MeshImpl(application, meshType, outputVertexElements, vertexSizeInBytes, indices)};
}
} // namespace DXD

MeshImpl::MeshImpl(DXD::Application &application, MeshType meshType, const std::vector<FLOAT> &vertexElements,
                   UINT vertexSizeInBytes, const std::vector<UINT> &indices)
    : application(*static_cast<ApplicationImpl *>(&application)),
      meshType(meshType),
      vertexSizeInBytes(vertexSizeInBytes),
      verticesCount(vertexElements.size() * sizeof(FLOAT) / vertexSizeInBytes),
      indicesCount(indices.size()) {
    assert(vertexSizeInBytes * verticesCount == vertexElements.size() * sizeof(FLOAT));
    uploadToGPU(vertexElements, indices);
}

#include <cassert>
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

void MeshImpl::uploadToGPU(const std::vector<FLOAT> &vertexElements, const std::vector<UINT> &indices) {
    const bool useIndexBuffer = indices.size() > 0;

    // Context
    ID3D12DevicePtr device = application.getDevice();
    auto &commandQueue = application.getCopyCommandQueue();

    // Record command list for GPU upload
    CommandList commandList{commandQueue.getCommandAllocatorManager(), nullptr};
    this->vertexBuffer = std::make_unique<VertexBuffer>(device, commandList, vertexElements.data(), verticesCount, vertexSizeInBytes);
    if (useIndexBuffer) {
        this->indexBuffer = std::make_unique<IndexBuffer>(device, commandList, indices.data(), static_cast<UINT>(indices.size()));
    }
    commandList.close();

    // Execute and register obtained allocator and lists to the manager
    std::vector<CommandList *> commandLists{&commandList};
    const uint64_t fenceValue = commandQueue.executeCommandListsAndSignal(commandLists);

    // Register upload status for buffers
    this->vertexBuffer->registerUpload(commandQueue, fenceValue);
    if (useIndexBuffer) {
        this->indexBuffer->registerUpload(commandQueue, fenceValue);
    }
}
