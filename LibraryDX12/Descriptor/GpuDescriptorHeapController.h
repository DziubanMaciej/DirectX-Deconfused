#pragma once

#include "CommandList/ResourceBindingType.h"
#include "Descriptor/DescriptorAllocation.h"

#include "DXD/Utility/NonCopyableAndMovable.h"

#include <ExternalHeaders/Wrappers/d3dx12.h>
#include <map>
#include <set>
#include <vector>

class RootSignature;
class CommandList;
struct D3D12_CPU_DESCRIPTOR_HANDLE;

/// \brief Manages descriptors in GPU visible descriptor heap
///
/// This class is created per CommandList for each type of gpu visible descriptor
/// heap, which is SRV_UAV_CBV and SAMPLER. It's responsible for copying descriptors
/// from already created non gpu visible heaps. This works in 3 phases, which are further described below:
/// 1. Setting root signature.
/// 2. Staging.
/// 3. Commit.
///
/// It is possible do the set-stage-commit cycle a couple of time for the same CommandList. This may however
/// Create descriptor heap overflow which would require allocating a new one. Changing descriptor heap in the
/// middle of command list  may have a performance penalty on some hardware and should be avoided.
///
/// Currently descriptor heap handling is not ideal
/// TODO: No support for heap overflow
/// TODO: validation if all of the required descriptors where staged could be useful
class GpuDescriptorHeapController : DXD::NonCopyableAndMovable {
private:
    struct DescriptorTableInfo {
        UINT offsetInStagingDescriptors;
        UINT descriptorCount;
    };
    using RootParameterIndex = UINT;

public:
    GpuDescriptorHeapController(CommandList &commandList, D3D12_DESCRIPTOR_HEAP_TYPE heapType);

    /// This is to prepare all allocations and validate descriptors passed in staging phase. By
    /// parsing the root signature it is possible to calculate how big the descriptor heap needs
    /// to be to fit all the descriptors.
    void setRootSignature(const RootSignature &rootSignature);

    /// This means passing non gpu visible descriptors for copy to the gpu visible heap. No actual copy
    /// is done during this phase, only saving the work to do in commit phase. Validation against table
    /// size is done here
    /// \param indexOfTable root parameter index set in root signature, not which table is it in whole signature
    /// \param offsetInTable offset in descriptors from the start of the table
    /// \param firstDescriptor handle to the first source descriptor to copy
    /// \param descriptorCount how many descriptors, starting with the firstDescriptor, should be copied
    void stage(RootParameterIndex indexOfTable, UINT offsetInTable, D3D12_CPU_DESCRIPTOR_HANDLE firstDescriptor, UINT descriptorCount);

    /// This is where the actual copy to gpu visible descriptor heap occurs. Copy is made on CPU, which means
    /// it's immediate, no need to wait for it. Also, calls to setGraphicsRootSignature is performed on the
    /// CommandList. Call to commit() should be done as late as possible to batch the operations. Just before
    /// draw/dispatch call is ideal.
    /// \param resourceBindingType specifies, whether a compute or graphics root signature should be bound
    void commit(ResourceBindingType::ResourceBindingType resourceBindingType);

    auto &getGpuDescriptorAllocations() { return gpuDescriptorAllocations; }

private:
    bool isRootParameterCompatibleTable(const D3D12_ROOT_PARAMETER1 &parameter) const;
    UINT calculateStagedDescriptorsCount() const;

    // Constant data
    CommandList &commandList;
    const D3D12_DESCRIPTOR_HEAP_TYPE heapType;
    const UINT descriptorIncrementSize;

    // Data created while parsing root signature
    std::map<RootParameterIndex, DescriptorTableInfo> descriptorTableInfos = {};
    std::set<RootParameterIndex> stagedDescriptorTables;

    // Data created while staging
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> stagingDescriptors = {};

    // Committing data
    std::vector<DescriptorAllocation> gpuDescriptorAllocations = {};
};
