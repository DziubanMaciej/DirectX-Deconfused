#pragma once

#include "Application/ApplicationImpl.h"
#include "PipelineState/PipelineStateController.h"
#include "Resource/Resource.h"
#include "Resource/VertexOrIndexBuffer.h"
#include "Threading/AsyncLoadableObject.h"

#include "DXD/Mesh.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <utility>
#include <vector>

struct MeshCpuLoadArgs {
    const std::wstring filePath;
    bool useTextures;
};

struct MeshCpuLoadResult {
    unsigned int meshType = 0; // TODO
    std::vector<FLOAT> vertexElements = {};
    std::vector<UINT> indices = {};
    UINT verticesCount;
    UINT vertexSizeInBytes;
};

struct MeshGpuLoadArgs {
    const std::vector<FLOAT> &vertexElements;
    const std::vector<UINT> &indices;
    UINT verticesCount;
    UINT vertexSizeInBytes;
};

struct MeshGpuLoadResult {
    std::unique_ptr<VertexBuffer> vertexBuffer = {};
    std::unique_ptr<IndexBuffer> indexBuffer = {};
};
using MeshAsyncLoadableObject = AsyncLoadableObject<MeshCpuLoadArgs, MeshCpuLoadResult, MeshGpuLoadArgs, MeshGpuLoadResult>;

class MeshImpl : public DXD::Mesh, MeshAsyncLoadableObject {
public:
    using MeshType = unsigned int;
    constexpr static MeshType UNKNOWN = 0x0;
    constexpr static MeshType TRIANGLE_STRIP = 0x01;
    constexpr static MeshType TEXTURE_COORDS = 0x02;
    constexpr static MeshType NORMALS = 0x04;

protected:
    friend class DXD::Mesh;
    MeshImpl(ApplicationImpl &application, const std::wstring &filePath, bool useTextures, bool asynchronousLoading);
    ~MeshImpl();

public:
    UINT getVertexSizeInBytes() const { return vertexSizeInBytes; }
    UINT getVerticesCount() const { return verticesCount; }
    UINT getIndicesCount() const { return indicesCount; }
    MeshType getMeshType() const { return meshType; }
    PipelineStateController::Identifier getPipelineStateIdentifier() const { return pipelineStateIdentifier; }
    PipelineStateController::Identifier getShadowMapPipelineStateIdentifier() const { return shadowMapPipelineStateIdentifier; }

    bool isUploadInProgress();

    auto &getVertexBuffer() { return vertexBuffer; }
    auto &getIndexBuffer() { return indexBuffer; }

protected:
    // Context
    ApplicationImpl &application;

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

private:
    // Helpers
    static MeshType computeMeshType(const std::vector<FLOAT> &normals, const std::vector<FLOAT> &textureCoordinates, bool useTextures);
    static UINT computeVertexSize(MeshType meshType);
    static std::map<MeshType, PipelineStateController::Identifier> getPipelineStateIdentifierMap();
    static PipelineStateController::Identifier computePipelineStateIdentifier(MeshType meshType);
    static std::map<MeshType, PipelineStateController::Identifier> getShadowMapPipelineStateIdentifierMap();
    static PipelineStateController::Identifier computeShadowMapPipelineStateIdentifier(MeshType meshType);

    // AsyncLoadableObject overrides
    MeshCpuLoadResult cpuLoad(const MeshCpuLoadArgs &args) override;
    MeshGpuLoadArgs createArgsForGpuLoad(const MeshCpuLoadResult &cpuLoadResult) override;
    MeshGpuLoadResult gpuLoad(const MeshGpuLoadArgs &args) override;
    void writeCpuGpuLoadResults(MeshCpuLoadResult &cpuLoadResult, MeshGpuLoadResult &gpuLoadResult) override;
};
