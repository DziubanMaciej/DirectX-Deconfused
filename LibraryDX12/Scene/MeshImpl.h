#pragma once

#include "Application/ApplicationImpl.h"
#include "PipelineState/PipelineStateController.h"
#include "Resource/Resource.h"
#include "Resource/VertexOrIndexBuffer.h"

#include "DXD/Mesh.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <utility>
#include <vector>

class MeshImpl : public DXD::Mesh {
public:
    using MeshType = unsigned int;
    constexpr static MeshType UNKNOWN = 0x0;
    constexpr static MeshType TRIANGLE_STRIP = 0x01;
    constexpr static MeshType TEXTURE_COORDS = 0x02;
    constexpr static MeshType NORMALS = 0x04;

protected:
    friend class DXD::Mesh;
    MeshImpl(ApplicationImpl &application, const std::wstring &filePath, bool useTextures);
    ~MeshImpl() {
        // TODO busy waiting, so the object is not deallocated while reference on a worker thread
        while (!loadingComplete.load())
            ;
    }

public:
    UINT getVertexSizeInBytes() const { return vertexSizeInBytes; }
    UINT getVerticesCount() const { return verticesCount; }
    UINT getIndicesCount() const { return indicesCount; }
    MeshType getMeshType() const { return meshType; }
    PipelineStateController::Identifier getPipelineStateIdentifier() const { return pipelineStateIdentifier; }

    bool isUploadInProgress();

    auto &getVertexBuffer() { return vertexBuffer; }
    auto &getIndexBuffer() { return indexBuffer; }

protected:
    // Context
    ApplicationImpl &application;

    // CPU data, set during load time
    std::atomic_bool loadingComplete = false;
    MeshType meshType = UNKNOWN;
    UINT vertexSizeInBytes = 0;
    UINT verticesCount = 0;
    UINT indicesCount = 0;
    PipelineStateController::Identifier pipelineStateIdentifier;

    // GPU data, set during upload time
    std::unique_ptr<VertexBuffer> vertexBuffer = {};
    std::unique_ptr<IndexBuffer> indexBuffer = {};

private:
    // Helpers
    static MeshType computeMeshType(const std::vector<FLOAT> &normals, const std::vector<FLOAT> &textureCoordinates, bool useTextures);
    static UINT computeVertexSize(MeshType meshType);
    static std::map<MeshType, PipelineStateController::Identifier> getPipelineStateIdentifierMap();
    static PipelineStateController::Identifier computePipelineStateIdentifier(MeshType meshType);
    void loadAndUploadObj(ApplicationImpl &application, const std::wstring &filePath, bool useTextures);

    // Loading CPU data
    struct LoadResults {
        LoadResults() = default;

        MeshType meshType = UNKNOWN;
        std::vector<FLOAT> vertexElements = {};
        std::vector<UINT> indices = {};
    };
    static LoadResults loadObj(const std::wstring &filePath, bool useTextures);

    // Uploading GPU data
    struct UploadResults {
        std::unique_ptr<VertexBuffer> vertexBuffer = {};
        std::unique_ptr<IndexBuffer> indexBuffer = {};
    };
    static UploadResults uploadToGPU(ApplicationImpl &application, const std::vector<FLOAT> &vertexElements, const std::vector<UINT> &indices, UINT verticesCount, UINT vertexSizeInBytes);

    // Called after all CPU and GPU data is available
    void setData(MeshType meshType, UINT vertexSizeInBytes, UINT verticesCount, UINT indicesCount,
                 PipelineStateController::Identifier pipelineStateIdentifier,
                 std::unique_ptr<VertexBuffer> &&vertexBuffer, std::unique_ptr<IndexBuffer> &&indexBuffer);
};
