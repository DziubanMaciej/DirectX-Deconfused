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

// ----------------------------------------------------------------- Creation and destruction

std::unique_ptr<Mesh> Mesh::createFromObj(DXD::Application &application, const std::wstring &filePath,
                                          bool loadTextureCoordinates, bool computeTangents,
                                          bool asynchronousLoading) {
    return std::unique_ptr<Mesh>(new MeshImpl(*static_cast<ApplicationImpl *>(&application),
                                              filePath, loadTextureCoordinates, computeTangents, asynchronousLoading));
}
} // namespace DXD

MeshImpl::MeshImpl(ApplicationImpl &application, const std::wstring &filePath,
                   bool loadTextureCoordinates, bool computeTangents, bool asynchronousLoading)
    : application(application) {
    const MeshCpuLoadArgs cpuLoadArgs{filePath, loadTextureCoordinates, computeTangents};
    cpuGpuLoad(cpuLoadArgs, asynchronousLoading);
}

MeshImpl::~MeshImpl() {
    terminateBackgroundProcessing(true);
}

// ----------------------------------------------------------------- AsyncLoadableObject overrides

MeshCpuLoadResult MeshImpl::cpuLoad(const MeshCpuLoadArgs &args) {
    // Initial validation
    const auto fullFilePath = std::wstring{RESOURCES_PATH} + args.filePath;
    std::fstream inputFile{fullFilePath, std::ios::in};
    if (!inputFile.good()) {
        return std::move(MeshCpuLoadResult{});
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
        if (shouldBackgroundProcessingTerminate()) {
            return std::move(MeshCpuLoadResult{});
        }

        if (line.empty()) {
            continue;
        }

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
            if (!strs.eof()) {
                strs >> f4;
                if (!f4.empty()) {
                    // Lines can and with a space, hence this check
                    indexTokens.push_back(f1);
                    indexTokens.push_back(f3);
                    indexTokens.push_back(f4);
                }
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
    MeshCpuLoadResult result{};
    this->meshType = MeshImpl::computeMeshType(normalCoordinates, textureCoordinates, args.loadTextureCoordinates, args.computeTangents);
    this->vertexSizeInBytes = computeVertexSize(this->meshType);
    if (this->meshType == MeshImpl::UNKNOWN) {
        return std::move(MeshCpuLoadResult{});
    }

    const bool hasTextureCoordinates = this->meshType & TEXTURE_COORDS;
    const bool hasNormals = normalCoordinates.size() > 0;
    const bool computeNormals = this->meshType & NORMALS && !hasNormals;
    const bool computeTangents = this->meshType & TANGENTS;
    const bool usesIndexBuffer = !hasTextureCoordinates && !hasNormals && !computeNormals && !computeTangents;
    if (usesIndexBuffer) {
        // Index buffer path - vertices are unmodified, we push indices to index buffer to define polygons
        result.vertexElements = std::move(vertexElements);
        for (int indexTokenIndex = 0; indexTokenIndex < indexTokens.size(); indexTokenIndex += 3) {
            UINT vertexIndices[3] = {};
            processIndexToken(indexTokens[indexTokenIndex + 0], false, false, vertexIndices + 0, nullptr, nullptr);
            processIndexToken(indexTokens[indexTokenIndex + 1], false, false, vertexIndices + 1, nullptr, nullptr);
            processIndexToken(indexTokens[indexTokenIndex + 2], false, false, vertexIndices + 2, nullptr, nullptr);
            result.indices.push_back(vertexIndices[0]);
            result.indices.push_back(vertexIndices[1]);
            result.indices.push_back(vertexIndices[2]);
        }
    } else {
        // No index buffer path - we interleave all vertex attributesm, so they're next to each other
        for (int indexTokenIndex = 0; indexTokenIndex < indexTokens.size(); indexTokenIndex += 3) {
            // Get all vertex attributes
            UINT vertexIndices[3] = {};
            UINT textureCoordinateIndices[3] = {};
            UINT normalIndices[3] = {};
            processIndexToken(indexTokens[indexTokenIndex + 0], hasTextureCoordinates, hasNormals, vertexIndices + 0, textureCoordinateIndices + 0, normalIndices + 0);
            processIndexToken(indexTokens[indexTokenIndex + 1], hasTextureCoordinates, hasNormals, vertexIndices + 1, textureCoordinateIndices + 1, normalIndices + 1);
            processIndexToken(indexTokens[indexTokenIndex + 2], hasTextureCoordinates, hasNormals, vertexIndices + 2, textureCoordinateIndices + 2, normalIndices + 2);

            // If user wants per-vertex tangents, we calculate them (per triangle)
            XMFLOAT3 computedTangent = {};
            if (computeTangents) {
                computeVertexTangent(vertexElements, textureCoordinates, vertexIndices, textureCoordinateIndices, computedTangent);
            }

            // Normals are mandatory, if the obj doesn't have them, we compute from vertices positions
            XMFLOAT3 computedNormal = {};
            if (computeNormals) {
                computeVertexNormal(vertexElements, vertexIndices, computedNormal);
            }

            // Append interleaved attributes to the vertex buffer memory
            for (int vertexInTriangleIndex = 0; vertexInTriangleIndex < 3; vertexInTriangleIndex++) {
                result.vertexElements.push_back(vertexElements[3 * (vertexIndices[vertexInTriangleIndex]) + 0]);
                result.vertexElements.push_back(vertexElements[3 * (vertexIndices[vertexInTriangleIndex]) + 1]);
                result.vertexElements.push_back(vertexElements[3 * (vertexIndices[vertexInTriangleIndex]) + 2]);
                if (computeNormals) {
                    result.vertexElements.push_back(computedNormal.x);
                    result.vertexElements.push_back(computedNormal.y);
                    result.vertexElements.push_back(computedNormal.z);
                }
                if (hasNormals) {
                    result.vertexElements.push_back(normalCoordinates[3 * (normalIndices[vertexInTriangleIndex]) + 0]);
                    result.vertexElements.push_back(normalCoordinates[3 * (normalIndices[vertexInTriangleIndex]) + 1]);
                    result.vertexElements.push_back(normalCoordinates[3 * (normalIndices[vertexInTriangleIndex]) + 2]);
                }
                if (computeTangents) {
                    result.vertexElements.push_back(computedTangent.x);
                    result.vertexElements.push_back(computedTangent.y);
                    result.vertexElements.push_back(computedTangent.z);
                }
                if (hasTextureCoordinates) {
                    result.vertexElements.push_back(textureCoordinates[2 * (textureCoordinateIndices[vertexInTriangleIndex]) + 0]);
                    result.vertexElements.push_back(textureCoordinates[2 * (textureCoordinateIndices[vertexInTriangleIndex]) + 1]);
                }
            }
        }
    }

    this->verticesCount = static_cast<UINT>(result.vertexElements.size() * sizeof(FLOAT) / this->vertexSizeInBytes);
    this->indicesCount = static_cast<UINT>(result.indices.size());
    this->pipelineStateIdentifier = computePipelineStateIdentifier(this->meshType);
    this->shadowMapPipelineStateIdentifier = computeShadowMapPipelineStateIdentifier(this->meshType);
    return std::move(result);
}

bool MeshImpl::isCpuLoadSuccessful(const MeshCpuLoadResult &result) {
    return this->meshType != UNKNOWN && result.vertexElements.size() > 0;
}

MeshGpuLoadArgs MeshImpl::createArgsForGpuLoad(const MeshCpuLoadResult &cpuLoadResult) {
    return std::move(MeshGpuLoadArgs{
        cpuLoadResult.vertexElements,
        cpuLoadResult.indices});
}

MeshGpuLoadResult MeshImpl::gpuLoad(const MeshGpuLoadArgs &args) {
    MeshGpuLoadResult results = {};
    const bool useIndexBuffer = args.indices.size() > 0;

    // Context
    ID3D12DevicePtr device = application.getDevice();
    auto &commandQueue = application.getCopyCommandQueue();

    // Record command list for GPU upload
    CommandList commandList{commandQueue};
    this->vertexBuffer = std::make_unique<VertexBuffer>(device, commandList, args.vertexElements.data(), this->verticesCount, this->vertexSizeInBytes);
    if (useIndexBuffer) {
        this->indexBuffer = std::make_unique<IndexBuffer>(device, commandList, args.indices.data(), static_cast<UINT>(args.indices.size()));
    }
    commandList.close();

    // Execute and register obtained allocator and lists to the manager
    const uint64_t fenceValue = commandQueue.executeCommandListAndSignal(commandList);

    // Register upload status for buffers
    this->vertexBuffer->addGpuDependency(commandQueue, fenceValue);
    if (useIndexBuffer) {
        this->indexBuffer->addGpuDependency(commandQueue, fenceValue);
    }

    return std::move(results);
}

bool MeshImpl::hasGpuLoadEnded() {
    const bool vertexInProgress = this->vertexBuffer->isWaitingForGpuDependencies();
    const bool indexInProgress = this->indexBuffer != nullptr && this->indexBuffer->isWaitingForGpuDependencies();
    const bool bothEnded = !vertexInProgress && !indexInProgress;
    return bothEnded;
}

// ----------------------------------------------------------------- Helpers

MeshImpl::MeshType MeshImpl::computeMeshType(const std::vector<FLOAT> &normals, const std::vector<FLOAT> &textureCoordinates,
                                             bool loadTextureCoordinates, bool computeTangents) {
    MeshType meshType = TRIANGLE_STRIP | NORMALS;
    if (loadTextureCoordinates) {
        if (textureCoordinates.size() > 0) {
            meshType |= TEXTURE_COORDS;
        } else {
            return UNKNOWN; // user wants texture coords which .obj doesn't provide - error
        }
    }
    if (computeTangents) {
        meshType |= TANGENTS;
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
    if (meshType & TANGENTS) {
        vertexSize += 3;
    }
    if (meshType & NORMALS) {
        vertexSize += 3;
    }
    return vertexSize * sizeof(FLOAT);
}

void MeshImpl::processIndexToken(const std::string &indexToken, bool textures, bool normals,
                                 UINT *outVertexIndex, UINT *outTextureCoordinateIndex, UINT *outNormalIndex) {
    size_t leftPosition{};
    size_t rightPosition{};

    rightPosition = indexToken.find('/', 0u);
    *outVertexIndex = std::stoi(indexToken.substr(leftPosition, rightPosition)) - 1;

    leftPosition = rightPosition + 1;
    rightPosition = indexToken.find('/', rightPosition + 1);
    if (textures && leftPosition != rightPosition) {
        *outTextureCoordinateIndex = std::stoi(indexToken.substr(leftPosition, rightPosition)) - 1;
    }

    leftPosition = rightPosition + 1;
    rightPosition = indexToken.find('/', rightPosition + 1);
    assert(rightPosition == std::string::npos);
    if (normals && leftPosition < indexToken.size()) {
        *outNormalIndex = std::stoi(indexToken.substr(leftPosition, rightPosition)) - 1;
    }
}

XMFLOAT3 MeshImpl::getVertexVector(const std::vector<FLOAT> &vertices, UINT vertexIndex) {
    const float x = vertices[3 * vertexIndex + 0];
    const float y = vertices[3 * vertexIndex + 1];
    const float z = vertices[3 * vertexIndex + 2];
    return XMFLOAT3{x, y, z};
}

XMFLOAT2 MeshImpl::getTextureCoordinateVector(const std::vector<FLOAT> &textureCoordinates, UINT textureCoordinateIndex) {
    const float u = textureCoordinates[2 * textureCoordinateIndex + 0];
    const float v = textureCoordinates[2 * textureCoordinateIndex + 1];
    return XMFLOAT2{u, v};
}

void MeshImpl::computeVertexTangent(const std::vector<FLOAT> &vertices, const std::vector<FLOAT> &textureCoordinates,
                                    const UINT vertexIndices[3], UINT textureCoordinateIndices[3], XMFLOAT3 &outTangent) {
    // Get position and texture coordinate deltas (edges)
    XMFLOAT3 pos1 = getVertexVector(vertices, vertexIndices[0]);
    XMFLOAT3 pos2 = getVertexVector(vertices, vertexIndices[1]);
    XMFLOAT3 pos3 = getVertexVector(vertices, vertexIndices[2]);
    XMFLOAT3 posEdge1 = XMFLOAT3{pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z};
    XMFLOAT3 posEdge2 = XMFLOAT3{pos3.x - pos1.x, pos3.y - pos1.y, pos3.z - pos1.z};
    XMFLOAT2 uv1 = getTextureCoordinateVector(textureCoordinates, textureCoordinateIndices[0]);
    XMFLOAT2 uv2 = getTextureCoordinateVector(textureCoordinates, textureCoordinateIndices[1]);
    XMFLOAT2 uv3 = getTextureCoordinateVector(textureCoordinates, textureCoordinateIndices[2]);
    XMFLOAT2 uvEdge1 = XMFLOAT2{uv2.x - uv1.x, uv2.y - uv1.y};
    XMFLOAT2 uvEdge2 = XMFLOAT2{uv3.x - uv1.x, uv3.y - uv1.y};

    // Calculate tangent
    float f = 1.0f / (uvEdge1.x * uvEdge2.y - uvEdge2.x * uvEdge1.y);
    outTangent.x = f * (uvEdge2.y * posEdge1.x - uvEdge1.y * posEdge2.x);
    outTangent.y = f * (uvEdge2.y * posEdge1.y - uvEdge1.y * posEdge2.y);
    outTangent.z = f * (uvEdge2.y * posEdge1.z - uvEdge1.y * posEdge2.z);

    // normalize
    float len = sqrtf(outTangent.x * outTangent.x + outTangent.y * outTangent.y + outTangent.z * outTangent.z);
    outTangent.x /= len;
    outTangent.y /= len;
    outTangent.z /= len;
}

void MeshImpl::computeVertexNormal(const std::vector<FLOAT> &vertexElements, const UINT vertexIndices[3], XMFLOAT3 &outNormal) {
    XMFLOAT3 _p1{vertexElements[3 * (vertexIndices[0]) + 0],
                 vertexElements[3 * (vertexIndices[0]) + 1],
                 vertexElements[3 * (vertexIndices[0]) + 2]};
    XMFLOAT3 _p2{vertexElements[3 * (vertexIndices[1]) + 0],
                 vertexElements[3 * (vertexIndices[1]) + 1],
                 vertexElements[3 * (vertexIndices[1]) + 2]};
    XMFLOAT3 _p3{vertexElements[3 * (vertexIndices[2]) + 0],
                 vertexElements[3 * (vertexIndices[2]) + 1],
                 vertexElements[3 * (vertexIndices[2]) + 2]};

    XMVECTOR p1 = XMLoadFloat3(&_p1);
    XMVECTOR p2 = XMLoadFloat3(&_p2);
    XMVECTOR p3 = XMLoadFloat3(&_p3);

    XMVECTOR e1 = XMVectorSubtract(p1, p2);
    XMVECTOR e2 = XMVectorSubtract(p3, p2);

    XMVECTOR normal = XMVector3Cross(e2, e1);
    outNormal = XMStoreFloat3(normal);
}

std::map<MeshImpl::MeshType, PipelineStateController::Identifier> MeshImpl::getPipelineStateIdentifierMap() {
    std::map<MeshImpl::MeshType, PipelineStateController::Identifier> map = {};
    map[TRIANGLE_STRIP | NORMALS] = PipelineStateController::Identifier::PIPELINE_STATE_NORMAL;
    map[TRIANGLE_STRIP | NORMALS | TEXTURE_COORDS] = PipelineStateController::Identifier::PIPELINE_STATE_TEXTURE_NORMAL;
    map[TRIANGLE_STRIP | NORMALS | TEXTURE_COORDS | TANGENTS] = PipelineStateController::Identifier::PIPELINE_STATE_TEXTURE_NORMAL_MAP;
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
    map[TRIANGLE_STRIP | NORMALS | TEXTURE_COORDS | TANGENTS] = PipelineStateController::Identifier::PIPELINE_STATE_SM_TEXTURE_NORMAL_MAP;
    return std::move(map);
}

PipelineStateController::Identifier MeshImpl::computeShadowMapPipelineStateIdentifier(MeshType meshType) {
    static const auto map = getShadowMapPipelineStateIdentifierMap();
    auto it = map.find(meshType);
    if (it == map.end()) {
        UNREACHABLE_CODE();
    }
    return it->second;
}
