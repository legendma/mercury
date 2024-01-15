#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

namespace D3D12 { namespace Core
{
typedef ID3D12Device9   IMDevice;
typedef IDXGIFactory7   IMDXGI;
typedef IDXGISwapChain4 IMSwapChain;
typedef ID3D12GraphicsCommandList6
                        IMGraphicsCommandList;
typedef ID3D12CommandQueue
                        IMCommandQueue;
typedef ID3D12CommandAllocator
                        IMCommandAllocator;
typedef ID3D12DescriptorHeap
                        IMDescriptorHeap;
typedef ID3D12Resource2 IMResource;
typedef ID3D12Heap1     IMHeap;
typedef ID3D12RootSignature
                        IMRootSignature;
typedef ID3D12PipelineState
                        IMPipelineState;
typedef ID3D12Fence1    IMFence;


}} /* namspace D3D12::Core */