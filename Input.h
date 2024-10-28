#pragma once
//template<typename T> class ComPtr;

// template<class U> friend class ComPtr;

#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>

#include <Windows.h>

#include <wrl.h>
using namespace Microsoft::WRL;

class Input {
public:
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
	void Initialize(HINSTANCE hInstance,HWND hwnd);
	void Update();

	// 全キーの入力状態を取得する
	BYTE key[256] = {};
	BYTE prekey[256] = {};

private:
	

	// DirectInputのインスタンス生成
	ComPtr<IDirectInput8> directInput = nullptr;

	// キーボードデバイス生成
	ComPtr<IDirectInputDevice8> keyboard;

};
