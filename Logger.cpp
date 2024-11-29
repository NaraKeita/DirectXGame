
#include "DirectXCommon.h"
#include <format>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")


namespace Logger {
    void log(const std::string& message) {
		OutputDebugStringA(message.c_str());
	}
}