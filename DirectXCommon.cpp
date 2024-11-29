#include "DirectXCommon.h"
#include <cassert>
#include "Logger.h"
#include <format>
#include "StringUtility.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

//#include <Windows.h>

// #include <cstdint>
#include "externals/DirectXTex/DirectXTex.h"
//#include <d3d12.h>
#include <dxcapi.h>
//#include <dxgi1_6.h>
//#include <dxgidebug.h>

//#include <fstream>
//#include <sstream>
//#include <string>
// #include "externals/imgui/imgui.h"
// #include "externals/imgui/imgui_impl_dx12.h"
// #include "externals/imgui/imgui_impl_win32.h"

//#include "Input.h"
//#include "WinApp.h"

// extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//#define _USE_MATH_DEFINES
//#include <math.h>

//#pragma comment(lib, "dxguid.lib")
//#pragma comment(lib, "dxcompiler.lib")

//using namespace Microsoft::WRL;
using namespace Logger;

std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}

// ComplierShader関数
IDxcBlob* CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler) {
	// 1.hlslファイル
	log(ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));

	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;

	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);

	assert(SUCCEEDED(hr));

	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	// 2.Complie
	LPCWSTR arguments[] = {
	    filePath.c_str(), L"-E", L"main", L"-T", profile, L"-Zi", L"-Qembed_debug", L"-Od", L"-Zpr",
	};

	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;

	hr = dxcCompiler->Compile(&shaderSourceBuffer, arguments, _countof(arguments), includeHandler, IID_PPV_ARGS(&shaderResult));

	assert(SUCCEEDED(hr));

	// 3.警告エラー

	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		log(shaderError->GetStringPointer());
		// 警告エラーダメ絶対
		assert(false);
	}
	// 4.Complie結果
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));

	log(ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));

	shaderSource->Release();
	shaderResult->Release();

	return shaderBlob;
}

DirectX::ScratchImage LoadTexture(const std::string& filePath) {
	// テクスチャファイル // byte関連
	DirectX::ScratchImage image{};
	// std::wstring filePathW = ConvertString(filePath);
	// HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップ　//拡大縮小で使う
	DirectX::ScratchImage mipImages{};
	// hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// ミップマップ付きのデータを返す
	return mipImages;
}

ID3D12Resource* CrateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata) {

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);                             // 幅
	resourceDesc.Height = UINT(metadata.height);                           // 高さ
	resourceDesc.MipLevels = UINT16(metadata.miscFlags);                   // 数
	resourceDesc.DepthOrArraySize = UINT(metadata.arraySize);              // 奥行き　Textureの配置数
	resourceDesc.Format = metadata.format;                                 // format
	resourceDesc.SampleDesc.Count = 1;                                     // サンプリングカウント(1固定)
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // textureの次元数

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	//// Resouceの生成
	// ID3D12Resource* resource = nullptr;
	// HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	// assert(SUCCEEDED(hr));
	// return resource;
}

void DirectXCommon::Initialize() {
	// NULL検出
	assert(winApp);
	// メンバ変数に記録
	this->winApp = winApp;

	DeviceInitialize();
	CommandInitialize();
	SwapChainInitialize();
	ZBufferInitialize();
	DescriptorHeapInitialize();
	RenderTargetInitialize();
	GetSRVCPUDescriptorHandle();
	ZBufferStencilViewInitialize();
	FenceInitialize();
	ScissoringInitialize();
	DXCCompilerInitialize();
	ImGuiInitialize();
	
	
	//ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_Init(device.Get(),
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap.Get(),
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
	    srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	/*D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0};
	const char* featureLevelStrings[] = {"12.2", "12.1", "12,0"};

	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr)) {
			log(std::format("FEatureLevel : {}\n", featureLevelStrings[i]));InitializeDevice
			break;
		}
	}*/

	assert(device != nullptr);
	log("Complete create D3D12Device!!!\n");

}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index) { 
	return GetSRVCPUDescriptorHandle(srvDescriptorHeap, desriptorSizeSRV, index);
}

void DirectXCommon::DeviceInitialize() {
	HRESULT hr;
#ifdef _DEBUG
	Microsoft::WRL::ComPtr < ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// デバッグレイヤーを有効にする
		debugController->EnableDebugLayer();
		// さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}

#endif

	#pragma region Factoryの生成
	// DZGIファクトリーの生成

	// HREUSLTはWindouws系のエラーコード
	// 関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	// 初期化の標本的な部分でエラーが出た場合はぷろぐらむがまちがっているか、
	//  どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr)); // 甲であることを保証　そうでないと止まる

	#pragma region アダプタの作成

	// 使用するアダプタ用の変数。最初にnullptrを入れておく
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;

	// 良い順にアダプタを読む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; i++) {
		// アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));
		// ソフトウェアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 採用したアダプタの情報をログに出力。wstringのほうなので注意
			log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;
	}
	// 適切なアダプタが見つからないので起動しない
	// 適切なアダプタが見つからなかったら起動できなくする
	assert(useAdapter != nullptr);

#pragma endregion

	#pragma region Deviceの生成

	D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0};
	const char* featureLevelStrings[] = {"12.2", "12.1", "12,0"};

	for (size_t i = 0; i < _countof(featureLevels); ++i) {
	    hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
	    if (SUCCEEDED(hr)) {
	        log(std::format("FEatureLevel : {}\n", featureLevelStrings[i]));
	        break;
	    }
	}

	assert(device != nullptr);
	log("Complete create D3D12Device!!!\n");

#pragma endregion

	#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 緊急時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		D3D12_MESSAGE_ID denyIds[] = {D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE};
		D3D12_MESSAGE_SEVERITY severities[] = {D3D12_MESSAGE_SEVERITY_INFO};
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumCategories = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		infoQueue->PushStorageFilter(&filter);

		// 解放
		infoQueue->Release();
	}

#endif


}

//コマンド関連の初期化
void DirectXCommon::CommandInitialize() {
#pragma region コマンドアロケータ
	// コマンドアロケータ生成
	HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	// 生成できない場合
	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region コマンドリスト
	// コマンドリスト生成
	 hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	// 生成できない場合
	 assert(SUCCEEDED(hr));
#pragma endregion

#pragma region コマンドキュー
	  D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	  hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	 // 生成できない場合
	  assert(SUCCEEDED(hr));

#pragma endregion

}

void DirectXCommon::SwapChainInitialize() {
#pragma region Swap Chainの生成
	HRESULT hr;
	// スワップチェイン生成
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WinApp::kClientWidth;
	swapChainDesc.Height = WinApp::kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// コマンドキュー生成
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;

	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));

	// SwapchainからResourceを引っ張ってくる
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResource[2] = {nullptr};

	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResource[0]));
	assert(SUCCEEDED(hr));

	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResource[1]));
	assert(SUCCEEDED(hr));
#pragma endregion
}

void DirectXCommon::DescriptorHeapInitialize() {
	ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device * device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDesciptors, bool shaderVisible) {
		// ディスクリプターヒープの生成
		ID3D12DescriptorHeap* descriptorHeap = nullptr;
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
		descriptorHeapDesc.Type = heapType;
		descriptorHeapDesc.NumDescriptors = numDesciptors;

		descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
		assert(SUCCEEDED(hr));
		return descriptorHeap;
	}

#pragma region ディスクリプターヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

#pragma endregion

	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// DSV生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dscDesc2{};
	dscDesc2.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dscDesc2.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	// textureを読んで転送
	DirectX::ScratchImage mipImages2 = LoadTexture("resource/monsterBall.png");
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = CrateTextureResource(device.Get(), metadata2);

	// metadataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	// SRVの生成
	device->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);
	
	// RTV
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

}

void DirectXCommon::RenderTargetInitialize() {
	device->CreateRenderTargetView(swapChainResource[0].Get(), &rtvDesc, rtvHandles[0]);

	//スワップチェーンからリソースを引っ張ってくる
	// rtvHandles[1] = GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 0); //おかしいかも
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(swapChainResource[1].Get(), &rtvDesc, rtvHandles[1]);

	//RTV用の設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	rtvHandles[0] = rtvStartHandle;

	//裏表の2つ分
	for (uint32_t i = 0; i < rtvHandles[]; i++) {
		// BlendState
		// D3D12_BLEND_DESC blendDesc{};
		// blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_BLEND_DESC blendDesc{};
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		blendDesc.RenderTarget[0].BlendEnable = TRUE;

		// 通常
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

		////加算合成
		//   blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		// blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		// blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

		////減算合成
		// blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		// blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		// blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

		// 乗算合成
		// blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
		// blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		// blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;

		// スクリーン合成
		// blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
		//.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		// blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

		// ここは変えない
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	}

}

void DirectXCommon::ZBufferStencilViewInitialize() {
	// DSV生成WIN
	D3D12_DEPTH_STENCIL_VIEW_DESC dscDesc{};
	dscDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dscDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	// DSVHeapの先頭
	device->CreateDepthStencilView(depthStencilResource.Get(), &dscDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void DirectXCommon::FenceInitialize() { 
	// ビューポート
	D3D12_VIEWPORT viewport;

	viewport.Width = WinApp::kClientWidth;
	viewport.Height = WinApp::kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	commandList->RSSetViewports(1, &viewport); 
}

void DirectXCommon::ScissoringInitialize() {

}

void DirectXCommon::DXCCompilerInitialize() {
	// DXC
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));
	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));
}

void DirectXCommon::ImGuiInitialize() {

}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index) { 
	return GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
}

void DirectXCommon::ZBufferInitialize() {

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = WinApp::kClientWidth;
	resourceDesc.Height = WinApp::kClientHeight;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // 二次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStrencil

	// 利用するHeap
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAN上で作る

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// resourceの生成
	ID3D12Resource* DiptZBuffer = nullptr;
	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&DiptZBuffer));
	assert(SUCCEEDED(hr));
}



//void DirextXCommon::ZBuffer() {
//	HRESULT hr = device->CreateCommittedResource(
//		&heapProperties,
//		D3D12_HEAP_FLAG_NONE,
//		&resourceDesc,
//		D3D12_RESOURCE_STATE_GENERIC_READ,
//		nullptr,
//		IID_PPV_ARGS(&zBuffer));
//	assert(SUCCEEDED(hr));
//}

//void DirectXCommon::DescriptorHeap() {
//	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
//}

//void DirectXCommon::Commond() {
//	
//	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
//	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
//	// 生成できない場合
//	assert(SUCCEEDED(hr));
//
//	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
//	// 生成できない場合
//	assert(SUCCEEDED(hr));
//
//	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
//	// 生成できない場合
//	assert(SUCCEEDED(hr));
//}

//void DirectXCommon::SwapChain() {
//
//	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
//	swapChainDesc.Width = WinApp::kClientWidth;
//	swapChainDesc.Height = WinApp::kClientHeight;
//	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	swapChainDesc.SampleDesc.Count = 1;
//	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//	swapChainDesc.BufferCount = 2;
//	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
//
//	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
//	assert(SUCCEEDED(hr));
//
//	// SwapchainからResourceを引っ張ってくる
//	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResource[2] = {nullptr};
//
//	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResource[0]));
//	assert(SUCCEEDED(hr));
//
//	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResource[1]));
//	assert(SUCCEEDED(hr));
//}

//void DirectXCommon::Fence() {
//	uint64_t fenceValue = 0;
//	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
//	assert(SUCCEEDED(hr));
//	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
//	assert(fenceEvent != nullptr);
//}

//void DirectXCommon::ImGui() {
//
//}
