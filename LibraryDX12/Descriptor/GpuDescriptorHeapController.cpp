#include "GpuDescriptorHeapController.h"

#include "CommandList/CommandList.h"
#include "Descriptor/CpuDescriptorAllocation.h"
#include "PipelineState/RootSignature.h"
#include "Utility/ThrowIfFailed.h"

#include <cassert>

GpuDescriptorHeapController::GpuDescriptorHeapController(CommandList &commandList, D3D12_DESCRIPTOR_HEAP_TYPE heapType)
    : commandList(commandList),
      heapType(heapType),
      descriptorIncrementSize(commandList.getDevice()->GetDescriptorHandleIncrementSize(heapType)) {}

void GpuDescriptorHeapController::setRootSignature(const RootSignature &rootSignature) {
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
    this->stagedDescriptorTables.clear();
}

void GpuDescriptorHeapController::stage(RootParameterIndex indexOfTable, UINT offsetInTable, D3D12_CPU_DESCRIPTOR_HANDLE firstDescriptor, UINT descriptorCount) {
    const auto descriptorTableInfo = descriptorTableInfos[indexOfTable];

    assert(offsetInTable + descriptorCount <= descriptorTableInfo.descriptorCount);

    CD3DX12_CPU_DESCRIPTOR_HANDLE currentDescriptor{firstDescriptor};
    for (auto descriptorIndex = 0u; descriptorIndex < descriptorCount; descriptorIndex++) {
        const auto offset = descriptorTableInfo.offsetInStagingDescriptors + descriptorIndex;
        this->stagingDescriptors[offset] = currentDescriptor;
        currentDescriptor.Offset(descriptorIncrementSize);
    }

    stagedDescriptorTables.insert(indexOfTable);
}

void GpuDescriptorHeapController::commit() {
    // Do nothing if we haven't staged anything new since last commit
    if (stagedDescriptorTables.size() == 0) {
        return;
    }

    auto device = commandList.getDevice();

    // TODO check if we have enough available handles

    // Create and set descriptor heap if it's not present
    if (descriptorHeap == nullptr) {
        D3D12_DESCRIPTOR_HEAP_DESC heapDescription = {};
        heapDescription.Type = heapType;
        heapDescription.NumDescriptors = static_cast<UINT>(this->stagingDescriptors.size()); // TODO align up?
        heapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        heapDescription.NodeMask = 1;
        throwIfFailed(device->CreateDescriptorHeap(&heapDescription, IID_PPV_ARGS(&this->descriptorHeap)));
        commandList.setDescriptorHeap(this->heapType, this->descriptorHeap);
        currentCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE{descriptorHeap->GetCPUDescriptorHandleForHeapStart()};
        currentGpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE{descriptorHeap->GetGPUDescriptorHandleForHeapStart()};
    }

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
        device->CopyDescriptors(destinationRangesCount, &currentCpuHandle, &destinationRangesCount,
                                sourceRangesCount, sourceRangeStarts, sourceRangeSizes, this->heapType);

        // Bind the table to command list
        commandList.getCommandList()->SetGraphicsRootDescriptorTable(rootParameterIndex, currentGpuHandle);

        // Increment GPU visible heap handles
        currentCpuHandle.Offset(descriptorTableInfo.descriptorCount, descriptorIncrementSize);
        currentGpuHandle.Offset(descriptorTableInfo.descriptorCount, descriptorIncrementSize);
    }
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
        unreachableCode();
    }
}
