#include "DirectXCommon.h"

using namespace Logger;
using namespace Microsoft::WRL;

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height) {

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
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
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

void DirectXCommon::Initialize(WinApp* winApp) {
	// NULL検出
	assert(winApp);
	// メンバ変数に記録
	this->winApp = winApp;
	//FPS固定初期化
	InitializeFixFPS();

	DeviceInitialize();
	CommandInitialize();
	SwapChainInitialize();
	ZBufferInitialize();
	DescriptorHeapInitialize();
	//CreateAllDescriptorHeap();
	RenderTargetInitialize();
	
	/*GetSRVCPUDescriptorHandle();
	GetSRVGPUDescriptorHandle();*/
	ZBufferStencilViewInitialize();
	FenceInitialize();
	ViewportInitialize();
	ScissoringInitialize();
	DXCCompilerInitialize();
	ImGuiInitialize();
	
	
	/*D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0};
	const char* featureLevelStrings[] = {"12.2", "12.1", "12,0"};

	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr)) {
			log(std::format("FEatureLevel : {}\n", featureLevelStrings[i]));InitializeDevice
			break;
		}
	}*/

	/*assert(device != nullptr);
	log("Complete create D3D12Device!!!\n");*/

	
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
	// ディスクリプターヒープの生成
	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;

	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
	return descriptorHeap;
	
}

void DirectXCommon::DeviceInitialize() {
	
#ifdef _DEBUG
	Microsoft::WRL::ComPtr < ID3D12Debug1> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// デバッグレイヤーを有効にする
		debugController->EnableDebugLayer();
		// さらにGPU側でもチェックを行うようにする
		debugController->SetEnableSynchronizedCommandQueueValidation(TRUE);
	}

#endif

#pragma region Factoryの生成
	// DZGIファクトリーの生成

	// HREUSLTはWindouws系のエラーコード
	// 関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	// 初期化の標本的な部分でエラーが出た場合はぷろぐらむがまちがっているか、
	//  どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr)); // 甲であることを保証　そうでないと止まる
#pragma endregion

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
			log(StringUtility::ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
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
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
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
	}
#endif
}

void DirectXCommon::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages) {
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		HRESULT hr = texture->WriteToSubresource(
		    UINT(mipLevel),
		    nullptr,              // 全領域へコピー
		    img->pixels,          // 元データアドレス
		    UINT(img->rowPitch),  // 1ラインサイズ
		    UINT(img->slicePitch) // 1枚サイズ
		);
		assert(SUCCEEDED(hr));
	}
}

//コマンド関連の初期化
void DirectXCommon::CommandInitialize() {
#pragma region コマンドアロケータ
	// コマンドアロケータ生成
	 hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
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

//スワップ
void DirectXCommon::SwapChainInitialize() {
#pragma region Swap Chainの生成
	// スワップチェイン生成
	swapChainDesc.Width = WinApp::kClientWidth;
	swapChainDesc.Height = WinApp::kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// スワップチェーン生成
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));

	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	assert(SUCCEEDED(hr));

	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));
#pragma endregion
}

//デスクリプタヒープ
void DirectXCommon::DescriptorHeapInitialize() {
	descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
#pragma region ディスクリプターヒープの生成
	rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
#pragma endregion
}

//レンダーターゲット
void DirectXCommon::RenderTargetInitialize() {
	 hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResource[0]));
	assert(SUCCEEDED(hr));

	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResource[1]));
	assert(SUCCEEDED(hr));

	// RTV
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// RTV用の設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); 

	//rtvHandles[0] = rtvStartHandle;

	//device->CreateRenderTargetView(swapChainResource[0].Get(), &rtvDesc, rtvHandles[0]);

	//スワップチェーンからリソースを引っ張ってくる
	

	//裏表の2つ分
	for (uint32_t i = 0; i < 2; ++i) {
		rtvHandles[0] = rtvStartHandle;
		rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		device->CreateRenderTargetView(swapChainResource[i].Get(), &rtvDesc, rtvHandles[i]);

		//rtvHandles[1] = GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 0); // おかしいかも
		//rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		//device->CreateRenderTargetView(swapChainResource[1].Get(), &rtvDesc, rtvHandles[1]);
#pragma region
		//// BlendState
		//// D3D12_BLEND_DESC blendDesc{};
		//// blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		//D3D12_BLEND_DESC blendDesc{};
		//blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		//blendDesc.RenderTarget[0].BlendEnable = TRUE;

		//// 通常
		//blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		//blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		//blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

		//////加算合成
		////   blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		//// blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		//// blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

		//////減算合成
		//// blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		//// blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		//// blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

		//// 乗算合成
		//// blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
		//// blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		//// blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;

		//// スクリーン合成
		//// blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
		////.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		//// blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

		//// ここは変えない
		//blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		//blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		//blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
#pragma endregion

	}

}

//void DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index) {
//	return GetSRVGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
//}

//深度ステンシルビュー
void DirectXCommon::ZBufferStencilViewInitialize() {
	
	depthStencilResource = CreateDepthStencilTextureResource(device.Get(), WinApp::kClientWidth, WinApp::kClientHeight);
	
#pragma region
	// DSVようのヒープでディスクリプタの数1、shader内で触らないのでfalse
	//dsvDescriptorHeap = CreateDescriptorHeap(,D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
	// DSV生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dscDesc2{};
	dscDesc2.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dscDesc2.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	
#pragma endregion

	// DSV生成WIN
	 D3D12_DEPTH_STENCIL_VIEW_DESC dscDesc{};
	 dscDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	 dscDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	// DSVHeapの先頭
	device->CreateDepthStencilView(depthStencilResource.Get(), &dscDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

}

//フェンス
void DirectXCommon::FenceInitialize() { 
	
	 hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	assert(fenceEvent != nullptr);
}

//ビューポート
void DirectXCommon::ViewportInitialize() {
	
	viewport.Width = WinApp::kClientWidth;
	viewport.Height = WinApp::kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	commandList->RSSetViewports(1, &viewport);
}

//シザリング矩形
void DirectXCommon::ScissoringInitialize() {
	
	scissorRect.left = 0;
	scissorRect.right = WinApp::kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::kClientHeight;

	commandList->RSSetScissorRects(1, &scissorRect);
}

//DXCコンパイラの生成
void DirectXCommon::DXCCompilerInitialize() {

	dxcUtils;
	dxcCompiler;

	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

}

void DirectXCommon::ImGuiInitialize() {
	// ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_Init(device.Get(),swapChainDesc.BufferCount,
		rtvDesc.Format, srvDescriptorHeap.Get(),
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
	    srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
	);
}

//コンパイルシェーダー
Microsoft::WRL::ComPtr<IDxcBlob> DirectXCommon::CompileShader(const std::wstring& filePath, const wchar_t* profile) {
		// 1.hlslファイル
		log(StringUtility::ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));
	
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
	
		log(StringUtility::ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));
	
		shaderSource->Release();
		shaderResult->Release();
	
		return shaderBlob;
	
	
	// shaderのコンパイラ
	//Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(L"resource/shaders/Object3d.VS.hlsl", L"vs_6_0"/*, dxcUtils, dxcCompiler, includeHandler*/);

	//assert(vertexShaderBlob != nullptr);
	//Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(L"resource/shaders/Object3d.PS.hlsl", L"ps_6_0"/*, dxcUtils, dxcCompiler, includeHandler*/);

	//assert(pixelShaderBlob != nullptr);

	//return Microsoft::WRL::ComPtr<IDxcBlob>();
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(size_t sizeInBytes) {
	// VertexResource
	// 頂点シェーダを作る
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを作る
	ID3D12Resource* vertexResource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));
	return vertexResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateTextureResource(const DirectX::TexMetadata& metadata) {

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

	// Resouceの生成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

DirectX::ScratchImage DirectXCommon::LoadTexture(const std::string& filePath) { 
	// テクスチャファイル // byte関連
	DirectX::ScratchImage image{};
	 std::wstring filePathW = StringUtility::ConvertString(filePath);
	 HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	 assert(SUCCEEDED(hr));

	// ミップマップ　//拡大縮小で使う
	DirectX::ScratchImage mipImages{};
	 hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	 assert(SUCCEEDED(hr));

	return mipImages;
}

//CPU
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index) {
	return GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
}
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetRTVCPUDescriptorHandle(uint32_t index) {
	return GetCPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, index); 
}
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetDSVCPUDescriptorHandle(uint32_t index) {
	return GetCPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, index);
}

//GPU
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index) {
	return GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
}
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetRTVGPUDescriptorHandle(uint32_t index) {
	return GetGPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, index);
}
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetDSVGPUDescriptorHandle(uint32_t index) {
	return GetGPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, index);
}

//深度バッファ
void DirectXCommon::ZBufferInitialize() {
	//深度バッファリソース設定
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

	CreateDepthStencilTextureResource(device.Get(), WinApp::kClientWidth, WinApp::kClientHeight);

	// resourceの生成
	 hr = device->CreateCommittedResource(
		&heapProperties, 
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&resource));

	assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeFixFPS()
{
	//現在時間を記録する
	reference_ = std::chrono::steady_clock::now();
}


void DirectXCommon::UpdateFixFPS()
{
	// 1/60秒ぴったりの時間
	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
	// 1/60秒よりわずかに短い時間
	const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

	//現在時間を取得する
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	//現在記録からの経過時間を取得する
	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	// 1/60秒(よりわずかに短い時間)経っていない場合
	if (elapsed < kMinCheckTime) {
		// 1/60秒経過するまで微小なスリープを繰り返す
		while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
			//1マイクロ秒スリープ
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
	//現在の時間を記録する
	reference_ = std::chrono::steady_clock::now();
}



void DirectXCommon::Finalize()
{
	CloseHandle(fenceEvent);
}

void DirectXCommon::PreDraw() {
#pragma region バックバッファの番号取得
	// これから書き込みバックバッファのインデックスを取得
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
#pragma endregion

#pragma region リソースバリアで書き込み可能に変更
	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを貼る対象のリソース。現在のバッファに対して行う
	barrier.Transition.pResource = swapChainResource[backBufferIndex].Get();
	// 前の(現在の)ResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);
#pragma endregion

#pragma region 描画先のRTVとDSVを指定する
	// 描画先のRTVの設定をする
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
	// DSV
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
#pragma endregion

#pragma region 画面全体の色をクリア
	// 指定した色で画面をクリアする　
	float clearColor[] = {0.1f, 0.25f, 0.5f, 1.0f};
#pragma endregion

#pragma region 画面全体の深度をクリア
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	// コマンド蓄積
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
#pragma endregion

#pragma region SRV用のデスクリプタヒープを指定する
// SRVを作成するDescriptorHeap場所決め
D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, 2);
D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, 2);

barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
#pragma endregion

#pragma region SRV用のデスクリプタヒープを指定する
	// 描画用のDescriptorHeap
	ID3D12DescriptorHeap* descriptorHeaps[] = {srvDescriptorHeap.Get()};
	commandList->SetDescriptorHeaps(1, descriptorHeaps);
#pragma endregion

#pragma region ビューポート領域の設定
	commandList->RSSetViewports(1, &viewport);
#pragma endregion

#pragma region シザー矩形の設定
	commandList->RSSetScissorRects(1, &scissorRect);
#pragma endregion
}

void DirectXCommon::PostDraw() {
#pragma region バックバッファの番号取得
	// これから書き込みバックバッファのインデックスを取得
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
#pragma endregion

#pragma region リソースバリアで表示状態に変更
	// TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);
#pragma endregion

#pragma region グラフィックコマンドをクローズ//		
	// コマンドリストの内容を確定させる。全てのコマンドを積んでからclearする
	hr = commandList->Close();
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region GPUコマンドの実行
	// GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = {commandList.Get()};
	commandQueue->ExecuteCommandLists(1, commandLists);
#pragma endregion

#pragma region GPU画面の交換を通知
	// GPUとOSに画面の交換を行うように通知する
	swapChain->Present(1, 0);
#pragma endregion

#pragma region Fenceの値を通知
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);
#pragma endregion

#pragma region コマンドキューにシグナルを送る
	commandQueue->Signal(fence.Get(), fenceValue);
#pragma endregion

#pragma region コマンド完了待ち
	// FENCEを更新する
	fenceValue++;
	commandQueue->Signal(fence.Get(), fenceValue);
	if (fence->GetCompletedValue() < fenceValue) {

		fence->SetEventOnCompletion(fenceValue, fenceEvent);

		WaitForSingleObject(fenceEvent, INFINITE);
	}
#pragma endregion

	//FPS固定更新
	UpdateFixFPS();

#pragma region コマンドアロケータのリセット
	// 次のフレームのコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region コマンドリストのリセット
	// 次のフレームのコマンドリストを準備
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));
#pragma endregion
}
