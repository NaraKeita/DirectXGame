#include "Logger.h"
#include "StringUtility.h"
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <format>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include "externals/DirectXTex/DirectXTex.h"

//ImGui
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#include <dxcapi.h>

#include "WinApp.h"
#include <array>
#include <d3d12.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <string>
#include <wrl.h>
#include <chrono>
#include "externals/DirectXTex/DirectXTex.h"
#include <thread>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// DirectX基盤
class DirectXCommon {
public:
	void Initialize(WinApp* winApp);

	// 指定番号のCPUデスクリプタハンドルを取得する
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	// 指定番号のGPUデスクリプタハンドルを取得する
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

	// テクスチャファイル読み込み関数
	static DirectX::ScratchImage LoadTexture(const std::string& filePath);

	void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	// Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	// 指定番号のCPUデスクリプタハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUDescriptorHandle(uint32_t index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);
	// 指定番号のGPUデスクリプタハンドルを取得する
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriptorHandle(uint32_t index);

	ID3D12DescriptorHeap* GetSrvDescriptorHeap() const { return srvDescriptorHeap.Get(); }
	ID3D12DescriptorHeap* GetRtvDescriptorHeap() const { return rtvDescriptorHeap.Get(); }

	// getter
	ID3D12Device* GetDevice() const { return device.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }

	// リソース生成関数
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	// ComplierShader関数
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile /*, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler*/);

	// デスクリプタヒープを生成する
	uint32_t descriptorSizeSRV;
	uint32_t descriptorSizeRTV;
	uint32_t descriptorSizeDSV;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap2;

	//記録時間(FPS固定用)
	std::chrono::steady_clock::time_point reference_;

	void Finalize();

	// 描画前処理
	void PreDraw();
	// 描画後処理
	void PostDraw();

private:
	void DeviceInitialize(); // デバイス

	void CommandInitialize();   // コマンド関連
	void SwapChainInitialize(); // スワップチェイン
	// void ZBufferInitialize();
	void DescriptorHeapInitialize(); // デスクリプタヒープ

	// void CreateAllDescriptorHeap();

	void RenderTargetInitialize(); // レンダーターゲットビュー

	// void GetSRVCPUDescriptorHandle();     //SRV専用デスクリプタハンドル取得関数
	// void GetSRVGPUDescriptorHandle();

	void ZBufferStencilViewInitialize(); // 深度ステンシルビュー
	void FenceInitialize();              // フェンスの生成
	void ViewportInitialize();           // ビューポート矩形の初期化
	void ScissoringInitialize();         // シザリング矩形の生成
	void DXCCompilerInitialize();        // DCXコンパイラの生成
	void ImGuiInitialize();              // ImGuiの初期化
	void ZBufferInitialize();            // 深度バッファ

private: // メンバ変数
	HRESULT hr;
	//------------------------------Device--------------------------------------//
	// DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device /* = nullptr*/;
	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory /* = nullptr*/;
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
	// Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;

	//------------------------------------------------------------------//

	//------------------------------------------------------------------//

	//メンバ関数
	//FPS固定初期化
	void InitializeFixFPS();
	//FPS固定更新
	void UpdateFixFPS();

	//------------------------------------------------------------------//

	//--------------------------------fence-------------------------------//
	// フェンスの初期化
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	uint32_t fenceValue = 0;

	//---------------------------------------------------------------------//

	// ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU;

	//-----------------------------------------------------------//

	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	//------------------------------------------------------------//

	// ビューポート
	D3D12_VIEWPORT viewport;

	D3D12_RECT scissorRect{};

	//----------------------------------------------------------------------------------------------//

	//----------------------------------------------------------------------------------------------//

	//----------------------------------------------------------------------------------------------//

	//----------------------------------------------------------------------------------------------//

	// ID3D12Resource* resource = nullptr;

	// DXC
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource2;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	// マテリアル
	// Material* materialDateSphere = nullptr;

	// スワップチェーンリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

	

	//----------------------------------------------------------------------------------------------//

	// 頂点シェーダを作る
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// 実際に頂点リソースを作る
	ID3D12Resource* vertexResource = nullptr;

	//----------------------------------------------------------------------------------------------//
};
