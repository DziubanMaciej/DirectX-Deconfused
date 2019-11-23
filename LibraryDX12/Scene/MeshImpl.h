#pragma once

#include "Application/ApplicationImpl.h"
#include "PipelineState/PipelineStateController.h"
#include "Resource/Resource.h"
#include "Resource/VertexOrIndexBuffer.h"
#include "Threading/CpuGpuOperation.h"
#include "Utility/MathHelper.h"

#include "DXD/Mesh.h"

#include <DXD/ExternalHeadersWrappers/DirectXMath.h>
#include <DXD/ExternalHeadersWrappers/d3d12.h>
#include <utility>
#include <vector>

struct MeshCpuLoadArgs {
    const std::wstring filePath;
    bool loadTextureCoordinates;
    bool computeTangents;
};

struct MeshCpuLoadResult {
    std::vector<FLOAT> vertexElements = {};
    std::vector<UINT> indices = {};
};

class ObjLoadCpuGpuOperation : public CpuGpuOperation<MeshCpuLoadArgs, MeshCpuLoadResult> {
public:
    ObjLoadCpuGpuOperation(MeshImpl &mesh) : mesh(mesh) {}

protected:
    // CpuGpuOperation overrides
    MeshCpuLoadResult cpuLoad(const MeshCpuLoadArgs &args) override;
    bool isCpuLoadSuccessful(const MeshCpuLoadResult &result) override;
    void gpuLoad(const MeshCpuLoadResult &args) override;
    bool hasGpuLoadEnded() override;

    // Helpers
    static void processIndexToken(const std::string &indexToken, bool textures, bool normals,
                                  UINT *outVertexIndex, UINT *outTextureCoordinateIndex, UINT *outNormalIndex);
    static XMFLOAT3 getVertexVector(const std::vector<FLOAT> &vertices, UINT vertexIndex);
    static XMFLOAT2 getTextureCoordinateVector(const std::vector<FLOAT> &textureCoordinates, UINT textureCoordinateIndex);
    static void computeVertexTangent(const std::vector<FLOAT> &vertices, const std::vector<FLOAT> &textureCoordinates,
                                     const UINT vertexIndices[3], UINT textureCoordinateIndices[3], XMFLOAT3 &outTangent);
    static void computeVertexNormal(const std::vector<FLOAT> &vertexElements, const UINT vertexIndices[3], XMFLOAT3 &outNormal);

private:
    MeshImpl &mesh;
};

class MeshImpl : public DXD::Mesh {
public:
    using MeshType = unsigned int;
    constexpr static MeshType UNKNOWN = 0x0;
    constexpr static MeshType TRIANGLE_STRIP = 0x01;
    constexpr static MeshType TEXTURE_COORDS = 0x02;
    constexpr static MeshType NORMALS = 0x04;
    constexpr static MeshType TANGENTS = 0x08;

protected:
    friend class DXD::Mesh;
    MeshImpl(const std::wstring &filePath, bool loadTextureCoordinates,
             bool computeTangents, bool asynchronousLoading);
    ~MeshImpl() override;

public:
    // Setters for loaders
    void setCpuData(MeshType meshType, UINT vertexSizeInBytes, UINT verticesCount, UINT indicesCount);
    void setGpuData(std::unique_ptr<VertexBuffer> &vertexBuffer, std::unique_ptr<IndexBuffer> &indexBuffer);

    // Getters
    UINT getVertexSizeInBytes() const { return vertexSizeInBytes; }
    UINT getVerticesCount() const { return verticesCount; }
    UINT getIndicesCount() const { return indicesCount; }
    MeshType getMeshType() const { return meshType; }
    PipelineStateController::Identifier getPipelineStateIdentifier() const { return pipelineStateIdentifier; }
    PipelineStateController::Identifier getShadowMapPipelineStateIdentifier() const { return shadowMapPipelineStateIdentifier; }
    bool isReady() { return loadOperation.isReady(); }

    auto &getVertexBuffer() { return vertexBuffer; }
    auto &getIndexBuffer() { return indexBuffer; }

    // Helpers
    static MeshType computeMeshType(const std::vector<FLOAT> &normals, const std::vector<FLOAT> &textureCoordinates,
                                    bool loadTextureCoordinates, bool computeTangents);
    static UINT computeVertexSize(MeshType meshType);

private:
    static std::map<MeshType, PipelineStateController::Identifier> getPipelineStateIdentifierMap();
    static PipelineStateController::Identifier computePipelineStateIdentifier(MeshType meshType);
    static std::map<MeshType, PipelineStateController::Identifier> getShadowMapPipelineStateIdentifierMap();
    static PipelineStateController::Identifier computeShadowMapPipelineStateIdentifier(MeshType meshType);

protected:
    // Load operation
    ObjLoadCpuGpuOperation loadOperation;

    // CPU data, set during load time
    MeshType meshType = UNKNOWN;
    UINT vertexSizeInBytes = 0;
    UINT verticesCount = 0;
    UINT indicesCount = 0;
    PipelineStateController::Identifier pipelineStateIdentifier;
    PipelineStateController::Identifier shadowMapPipelineStateIdentifier;

    // GPU data, set during upload time
    std::unique_ptr<VertexBuffer> vertexBuffer = {};
    std::unique_ptr<IndexBuffer> indexBuffer = {};
};
