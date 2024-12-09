//#pragma once
//#include <d3d12.h>
//#include <dxgi1_6.h>
//#include <wrl.h>
#include "WinApp.h"
//#include "externals/imgui/imgui.h"
//#include "externals/imgui/imgui_impl_dx12.h"
//#include "externals/imgui/imgui_impl_win32.h"
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

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	// 指定番号のCPUデスクリプタハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
	// 指定番号のGPUデスクリプタハンドルを取得する
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);
	
private:
	void DeviceInitialize();              //デバイス
	
	void CommandInitialize();             // コマンド関連
	void SwapChainInitialize();           // スワップチェイン
	void ZBufferInitialize();             // 深度バッファ
	void DescriptorHeapInitialize();      // デスクリプタヒープ

	void CreateAllDescriptorHeap();

	void RenderTargetInitialize();        // レンダーターゲットビュー

	//void GetSRVCPUDescriptorHandle();     //SRV専用デスクリプタハンドル取得関数
	//void GetSRVGPUDescriptorHandle();

	void ZBufferStencilViewInitialize();  //深度ステンシルビュー
	void FenceInitialize();               //フェンスの生成
	void ViewportInitialize();            //ビューポート矩形の初期化
	void ScissoringInitialize(); // シザリング矩形の生成
	void DXCCompilerInitialize();         //DCXコンパイラの生成
	void ImGuiInitialize();               //ImGuiの初期化

		

private://メンバ変数
	//DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	//DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	//WindowsAPI
	WinApp* winApp_ = nullptr;

	// コマンドアロケータ生成
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	// コマンドリスト生成
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	// コマンドキュー生成
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	// 深度バッファの生成
	ID3D12Resource* zBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
	// フェンスの初期化
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	//デスクリプタヒープを生成する
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap2;

	// DXC
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource2;

	// スワップチェイン生成
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;

	// SwapchainからResourceを引っ張ってくる
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResource[2] = {nullptr};

	uint32_t fenceValue = 0;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	// スワップチェーンリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

	uint32_t descriptorSizeSRV = 1280;
	uint32_t descriptorSizeRTV = 1280;
	uint32_t descriptorSizeDSV = 1280;

	// 指定番号のCPUデスクリプタハンドルを取得する
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	// 指定番号のGPUデスクリプタハンドルを取得する
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	
};
