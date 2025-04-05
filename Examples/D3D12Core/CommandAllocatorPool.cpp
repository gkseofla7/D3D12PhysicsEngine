//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard
//

#include "CommandAllocatorPool.h"
#include "Engine.h"
#include "Device.h"
namespace dengine {
CommandAllocatorPool::CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE Type) :
    m_cCommandListType(Type)
{
}

CommandAllocatorPool::~CommandAllocatorPool()
{
    Shutdown();
}

void CommandAllocatorPool::Shutdown()
{
    for (size_t i = 0; i < m_AllocatorPool.size(); ++i)
        m_AllocatorPool[i]->Release();

    m_AllocatorPool.clear();
}

ID3D12CommandAllocator * CommandAllocatorPool::RequestAllocator(uint64_t CompletedFenceValue)
{
    std::lock_guard<std::mutex> LockGuard(m_AllocatorMutex);

    ID3D12CommandAllocator* pAllocator = nullptr;

    if (!m_ReadyAllocators.empty())
    {
        std::pair<uint64_t, ID3D12CommandAllocator*>& AllocatorPair = m_ReadyAllocators.front();

        if (AllocatorPair.first <= CompletedFenceValue)
        {
            pAllocator = AllocatorPair.second;
            pAllocator->Reset();
            m_ReadyAllocators.pop();
        }
    }

    // If no allocator's were ready to be reused, create a new one
    if (pAllocator == nullptr)
    {
        DEVICE->CreateCommandAllocator(m_cCommandListType, IID_PPV_ARGS(&pAllocator));
        wchar_t AllocatorName[32];
        swprintf(AllocatorName, 32, L"CommandAllocator %zu", m_AllocatorPool.size());
        pAllocator->SetName(AllocatorName);
        m_AllocatorPool.push_back(pAllocator);
    }

    return pAllocator;
}
/* DNote : DiscardAllocator, fence ������ �޴´�
* �Ƹ� fence Value�� �޴� ������ �ش� ����� �Ϸ�ƴ����� Ȯ���ϱ� ���ؼ��ϵ� �ϴ�.
* �� ������Ʈ���� gpu �۾��� �������� ����ϴ°ͺ��� ������ ���δ�.
*/
void CommandAllocatorPool::DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator * Allocator)
{
    std::lock_guard<std::mutex> LockGuard(m_AllocatorMutex);

    // That fence value indicates we are free to reset the allocator
    m_ReadyAllocators.push(std::make_pair(FenceValue, Allocator));
}
}