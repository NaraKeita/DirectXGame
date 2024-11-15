#pragma once



class WinApp {
public:
	// 静的メンバ関数
	static LRESULT CALLBACK WnidowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	//初期化
	void Initialize();
	//更新
	void Update();


};
