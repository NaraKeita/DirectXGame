#include <cassert>
#include "Logger.h"
#include <format>
#include "StringUtility.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include "externals/DirectXTex/DirectXTex.h"
#include <dxcapi.h>
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#include "WinApp.h"
#include<d3d12.h>
#include<dxgi1_6.h>
#include<wrl.h>
#include<string>
#include<array>
#include<dxcapi.h>

#include"externals/DirectXTex/DirectXTex.h"

//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


//DirectX基盤
class DirectXCommon {
public:
	void Initialize(WinApp* winApp);
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	
	// 指定番号のCPUデスクリプタハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUDescriptorHandle(uint32_t index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);
	// 指定番号のGPUデスクリプタハンドルを取得する
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriptorHandle(uint32_t index);



private:

	void DeviceInitialize();              //デバイス
	
	void CommandInitialize();             // コマンド関連
	void SwapChainInitialize();           // スワップチェイン
	//void ZBufferInitialize();             
	void DescriptorHeapInitialize(); // デスクリプタヒープ

	//void CreateAllDescriptorHeap();

	void RenderTargetInitialize();        // レンダーターゲットビュー

	//void GetSRVCPUDescriptorHandle();     //SRV専用デスクリプタハンドル取得関数
	//void GetSRVGPUDescriptorHandle();

	void ZBufferStencilViewInitialize();  //深度ステンシルビュー
	void FenceInitialize();               //フェンスの生成
	void ViewportInitialize();            //ビューポート矩形の初期化
	void ScissoringInitialize();          // シザリング矩形の生成
	void DXCCompilerInitialize();         //DCXコンパイラの生成
	void ImGuiInitialize();               //ImGuiの初期化
	void ZBufferInitialize();             // 深度バッファ
		

private://メンバ変数
	HRESULT hr;
//------------------------------Device--------------------------------------//
	//DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device/* = nullptr*/;
	//DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory/* = nullptr*/;
//--------------------------------------------------------------------------//

//------------------------------command-----------------------------------------//
	// コマンドアロケータ生成
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	// コマンドリスト生成
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	// コマンドキュー生成
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
//------------------------------------------------------------------------------//

//-----------------------------------------SwapChain---------------------------------//
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	// スワップチェイン生成
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	// SwapchainからResourceを引っ張ってくる
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResource[2] = {nullptr};
//----------------------------------------------------------------------------------//

	// WindowsAPI
	WinApp* winApp = nullptr;

//------------------------------深度バッファ------------------------//
	// 深度バッファの生成
	ID3D12Resource* resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
	//Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;
	
//------------------------------------------------------------------//

//------------------------------------------------------------------//

	// デスクリプタヒープを生成する
	uint32_t descriptorSizeSRV = 1280;
	uint32_t descriptorSizeRTV = 1280;
	uint32_t descriptorSizeDSV = 1280;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap2;

//------------------------------------------------------------------//

//--------------------------------fence-------------------------------//
	// フェンスの初期化
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	uint32_t fenceValue = 0;
	
//---------------------------------------------------------------------//

	//ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU;

	

	//ID3D12Resource* resource = nullptr;

	// DXC
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource2;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	//マテリアル
	//Material* materialDateSphere = nullptr;

	// スワップチェーンリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

	// 指定番号のCPUデスクリプタハンドルを取得する
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	// 指定番号のGPUデスクリプタハンドルを取得する
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	
};
