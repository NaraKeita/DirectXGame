#include "Input.h"
#include <cassert>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void Input::Initialize(HINSTANCE hInstance, HWND hwnd) {
	
	LRESULT result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));
	
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));
	//入力データ形式のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	//排他制御レベルのセット
	result = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void Input::Update() {
	//---------2_2_input更新-----------//
	

	//-----------------2_3_固有の処理編----------------//
	//前回のキー入力を保存
	memcpy(prekey, key, sizeof(key));
	//-----------------------------------------------//
	
	//キーボード情報の取得開始
	keyboard->Acquire();
	//全キーの入力情報を取得する
	//BYTE key[256] = {};
	keyboard->GetDeviceState(sizeof(key), key);

	//-----------------------------------//
}

//-----------------2_3_固有の処理編-----------------//

bool Input::PusuKey(BYTE keyNumber) {
	//指定キーで押していればtrueで返す
	if (key[keyNumber]) {
		return true;
	}
	//そうでなければfalseを返す
	return false;
}

bool Input::TriggerKey(BYTE keyNumber) { 
	if (!prekey[keyNumber] && key[keyNumber]) {
		return true;
	}
	// そうでなければfalseを返す
	return false;
}

//-----------------------------------------------//