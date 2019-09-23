#include "GpuDescriptorHeapController.h"

#include "CommandList/CommandList.h"
#include "Descriptor/DescriptorAllocation.h"
#include "PipelineState/RootSignature.h"
#include "Utility/ThrowIfFailed.h"

#include <cassert>

GpuDescriptorHeapController::GpuDescriptorHeapController(CommandList &commandList, D3D12_DESCRIPTOR_HEAP_TYPE heapType)
    : commandList(commandList),
      heapType(heapType),
      descriptorIncrementSize(commandList.getDevice()->GetDescriptorHandleIncrementSize(heapType)) {}

void GpuDescriptorHeapController::setRootSignature(const RootSignature &rootSignature) {
    // TODO this check could be useful because uncommitted tables here means redundant change of pipeline state
    // rendering has to be reworked though, so it changes pipeline state only when necessary
    //assert(stagedDescriptorTables.size() == 0); // There shouldn't be any uncommitted tables
    stagedDescriptorTables.clear();

    this->descriptorTableInfos.clear();
    UINT currentOffsetInStagingDescriptors = 0u;

    const auto rootParameters = rootSignature.getRootParameters();
    for (auto rootParameterIndex = 0u; rootParameterIndex < rootParameters.size(); rootParameterIndex++) {
        const auto rootParameter = rootParameters[rootParameterIndex];
        if (!isRootParameterCompatibleTable(rootParameter)) {
            continue;
        }

        const auto descriptorRanges = rootParameter.DescriptorTable.pDescriptorRanges;
        const auto descriptorRangesCount = rootParameter.DescriptorTable.NumDescriptorRanges;
        UINT currentDescriptorCount = 0u;
        for (auto rangeIndex = 0u; rangeIndex < descriptorRangesCount; rangeIndex++) {
            const auto &range = descriptorRanges[rangeIndex];
            currentDescriptorCount += range.NumDescriptors;
        }

        this->descriptorTableInfos[rootParameterIndex] = DescriptorTableInfo{currentOffsetInStagingDescriptors, currentDescriptorCount};
        currentOffsetInStagingDescriptors += currentDescriptorCount;
    }

    this->stagingDescriptors.clear();
    this->stagingDescriptors.resize(currentOffsetInStagingDescriptors);
}

void GpuDescriptorHeapController::stage(RootParameterIndex indexOfTable, UINT offsetInTable, D3D12_CPU_DESCRIPTOR_HANDLE firstDescriptor, UINT descriptorCount) {
    const auto descriptorTableInfo = descriptorTableInfos[indexOfTable];

    assert(offsetInTable + descriptorCount <= descriptorTableInfo.descriptorCount);

    CD3DX12_CPU_DESCRIPTOR_HANDLE currentDescriptor{firstDescriptor};
    for (auto descriptorIndex = 0u; descriptorIndex < descriptorCount; descriptorIndex++) {
        const auto offset = descriptorTableInfo.offsetInStagingDescriptors + offsetInTable + descriptorIndex;
        this->stagingDescriptors[offset] = currentDescriptor;
        currentDescriptor.Offset(descriptorIncrementSize);
    }

    stagedDescriptorTables.insert(indexOfTable);
}

void GpuDescriptorHeapController::commit(ResourceBindingType::ResourceBindingType resourceBindingType) {
    // Do nothing if we haven't staged anything new since last commit
    if (stagedDescriptorTables.size() == 0) {
        return;
    }

    auto device = commandList.getDevice();

    // TODO check if we have enough available handles

    // Allocate space for gpu-visible desciptors
    auto allocationResult = commandList.getDescriptorController().allocateGpu(heapType, calculateStagedDescriptorsCount());
    this->gpuDescriptorAllocations.push_back(std::move(std::get<DescriptorAllocation>(allocationResult)));
    CD3DX12_CPU_DESCRIPTOR_HANDLE currentCpuHandle = gpuDescriptorAllocations.back().getCpuHandle();
    CD3DX12_GPU_DESCRIPTOR_HANDLE currentGpuHandle = gpuDescriptorAllocations.back().getGpuHandle();

    // Bind descriptor heap
    commandList.setDescriptorHeap(heapType, std::get<ID3D12DescriptorHeapPtr>(allocationResult));

    // Process each descriptor table
    for (auto it = stagedDescriptorTables.begin(); it != stagedDescriptorTables.end(); it++) {
        const auto rootParameterIndex = *it;
        const auto descriptorTableInfo = descriptorTableInfos.at(rootParameterIndex);

        // Copy whole table to gpu visible heap
        const UINT destinationRangesCount = 1u;
        const UINT destinationRangeSize = descriptorTableInfo.descriptorCount;
        const UINT sourceRangesCount = descriptorTableInfo.descriptorCount;
        const D3D12_CPU_DESCRIPTOR_HANDLE *sourceRangeStarts = &this->stagingDescriptors[descriptorTableInfo.offsetInStagingDescriptors];
        const UINT *sourceRangeSizes = nullptr; // It means all ranges are of length 1
        device->CopyDescriptors(destinationRangesCount, &currentCpuHandle, &destinationRangeSize,
                                sourceRangesCount, sourceRangeStarts, sourceRangeSizes, this->heapType);

        // Bind the table to command list
        ResourceBindingType::setRootDescriptorTableFunctions[resourceBindingType](commandList.getCommandList().Get(), rootParameterIndex, currentGpuHandle);

        // Increment GPU visible heap handles
        currentCpuHandle.Offset(descriptorTableInfo.descriptorCount, descriptorIncrementSize);
        currentGpuHandle.Offset(descriptorTableInfo.descriptorCount, descriptorIncrementSize);
    }

    this->stagedDescriptorTables.clear();
}

bool GpuDescriptorHeapController::isRootParameterCompatibleTable(const D3D12_ROOT_PARAMETER1 &parameter) const {
    if (parameter.ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
        return false;
    }

    const auto rangeType = parameter.DescriptorTable.pDescriptorRanges[0].RangeType;
    switch (rangeType) {
    case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
    case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
    case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
        return this->heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
        return this->heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    default:
        UNREACHABLE_CODE();
    }
}

UINT GpuDescriptorHeapController::calculateStagedDescriptorsCount() const {
    UINT result = 0u;
    for (auto it = stagedDescriptorTables.begin(); it != stagedDescriptorTables.end(); it++) {
        const auto descriptorTableInfo = descriptorTableInfos.at(*it);
        result += descriptorTableInfo.descriptorCount;
    }
    return result;
}
