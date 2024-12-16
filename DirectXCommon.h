#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

//DirectX基盤
class DirectXCommon {
public:
	void Initialize();

public:
	void DeviceInitialize(); // デバイス

	void CommandInitialize();   // コマンド関連
	void SwapChainInitialize(); // スワップチェイン
	void ZBufferInitialize();   // 深度バッファ
	void DescriptorHeapInitialize(); // デスクリプタヒープ
	void RenderTargetInitialize();   // レンダーターゲットビュー
	void ZBufferStencilViewInitialize(); // 深度ステンシルビュー
	void FenceInitialize();              // フェンスの生成
	void ViewportInitialize();           // ビューポート矩形の初期化
	void ScissoringInitialize();         // シザリング矩形の生成
	void DXCCompilerInitialize();        // DCXコンパイラの生成
	void ImGuiInitialize();              // ImGuiの初期化
//	void CreateAllDescriptorHeap();

};
