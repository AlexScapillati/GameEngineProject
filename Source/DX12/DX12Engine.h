#pragma once
#include "..\Engine.h"

#include "CDX12Common.h"

// https://www.3dgep.com/learning-directx-12-1/#GPU_Synchronization

class CDX12Scene;
class CDX12Gui;

class CDX12Engine : public IEngine
{
public:

	CDX12Engine(HINSTANCE hInstance, int nCmdShow);

	// Inherited via IEngine
	bool Update() override;

	// Inherited via IEngine
	void Resize(UINT x, UINT y) override;

	void Finalize() override;


	//--------------------------
	// Setters / Getters
	//--------------------------

	auto GetDevice() const { return mDevice.Get(); }
	auto GetScene() const { return mMainScene.get(); }


	//--------------------------
	// DirectX 12 Variables
	//--------------------------

	static const uint32_t mNumFrames = 3;

	ComPtr<ID3D12Device2> mDevice;

	ComPtr<IDXGISwapChain4> mSwapChain;

	ComPtr<ID3D12Resource> mBackBuffers[mNumFrames];

	/*
	 * A Command List is used to issue copy, compute (dispatch), or draw commands.
	 * In DirectX 12 commands issued to the command list are not executed immediately -
	 * like they are with the DirectX 11 immediate context.
	 * All command lists in DirectX 12 are deferred; that is, the commands in a command list -
	 * are only run on the GPU after they have been executed on a command queue.
	 */
	ComPtr<ID3D12GraphicsCommandList2> mCommandList;

	ComPtr<ID3D12CommandQueue> mCommandQueue;

	/*
	 * The ID3D12CommandAllocator serves as the backing memory for recording the GPU commands into a command list.
	 * Unlike the command list, a command allocator cannot be reused
	 * unless all of the commands that have been recorded into the command allocator have finished executing on the GPU.
	 * Attempting to reset a command allocator before the command queue has finished executing
	 * those commands will result in a COMMAND_ALLOCATOR_SYNC error by the debug layer.
	 * The mCommandAllocators array variable is used to store the reference to the command allocators.
	 * There must be at least one command allocator per render frame that is “in-flight”
	 * (at least one per back buffer of the swap chain).
	 */
	ComPtr<ID3D12CommandAllocator> mCommandAllocators[mNumFrames];

	/*
	 * In previous versions of DirectX, RTVs were created one at a time.
	 * Since DirectX 12, RTVs are now stored in descriptor heaps.
	 * A descriptor heap can be visualized as an array of descriptors(views).
	 * A view simply describes a resource that resides in GPU memory.
	 * The mRTVDescriptorHeap variable is used to store the descriptor heap that
	 * contains the render target views for the swap chain back buffers.
	*/
	ComPtr<ID3D12DescriptorHeap> mRTVDescriptorHeap;

	/*
	 * The size of a descriptor in a descriptor heap is vendor specific
	 * (Intel, NVidia, and AMD may store descriptors differently).
	 * In order to correctly offset the index into the descriptor heap,
	 * the size of a single element in the descriptor heap needs to be queried during initialization.
	 */
	UINT mRTVDescriptorSize;

	/*
	 * The index holding the current back buffer that will need to be presented is stored here
	 */

	UINT mCurrentBackBufferIndex;

	/*
	 * The software rasterizer allows the graphics programmer
	 * to access the full set of advanced rendering features that
	 * may not be available in the hardware (for example, when running on older GPUs).
	 * The WARP device can also be used to verify the results of a rendering technique
	 * if the quality of the vendor supplied display driver is in question.
	*/
	bool mUseWarp = false;


	//-------------------------
	// Synchronization objects
	//-------------------------

	/*
	 * The Fence object is used to synchronize commands issued to the Command Queue.
	 * The fence stores a single value that indicates the last value that was used to signal the fence.
	 * Although it is possible to use the same fence object with multiple command queues,
	 * it is not reliable to ensure the proper synchronization of commands across command queues.
	 * Therefore, it is advised to create at least one fence object for each command queue.
	 * Multiple command queues can wait on a fence to reach a specific value,
	 * but the fence should only be allowed to be signaled from a single command queue.
	 * In addition to the fence object, the application must also track a fence value that is used to signal the fence.
	 */
	ComPtr<ID3D12Fence> mFence;

	// The next fence value to signal the command queue next is stored in the mFenceValue variable.
	uint64_t mFenceValue = 0;

	/*
	 * For each rendered frame that could be “in-flight” on the command queue,
	 * the fence value that was used to signal the command queue needs to be tracked to guarantee
	 * that any resources that are still being referenced by the command queue are not overwritten.
	 * The g_FrameFenceValues array variable is used to keep track of the fence values
	 * that were used to signal the command queue for a particular frame.
	 */

	uint64_t mFrameFenceValues[mNumFrames] = {};

	// The mFenceEvent variable is a handle to an OS event object that will be used to receive the notification that the fence has reached a specific value.
	HANDLE mFenceEvent;


	CommandListQueue mCommandListQueue;
	CommandAllocatorQueue mCommandAllocatorQueue;


	//----------------------------------------
	// Shaders
	//-----------------------------------------


	SShader mPbrPixelShader;
	SShader mPbrVertexShader;
	SShader mPbrNormalPixelShader;
	SShader mPbrNormalVertexShader;
	SShader mDepthOnlyPixelShader;
	SShader mDepthOnlyNormalPixelShader;
	SShader mBasicTransformVertexShader;
	SShader mTintedTexturePixelShader;
	SShader mSkyPixelShader;
	SShader mSkyVertexShader;

	void LoadDefaultShaders();


	//----------------------------------------
	// DirectX Functions
	//-----------------------------------------

	void InitD3D();

	void CheckRayTracingSupport() const;

	void EnableDebugLayer();

	void CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type);

	void CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);

	void UpdateRenderTargetViews();

	ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type) const;

	void CreateCommandList(ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type);

	void Flush();

	uint64_t Signal();

	Resource LoadTexture(std::string& filename) const;

	// Execute a command list.
	// Returns the fence value to wait for for this command list.
	uint64_t ExecuteCommandList(ID3D12GraphicsCommandList2* commandList);

	std::unique_ptr<CDX12Scene> mMainScene;
	std::unique_ptr<CDX12Gui> mGui;
};
