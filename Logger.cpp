
#include "DirectXCommon.h"
#include <format>

namespace Logger {
    void log(const std::string& message) {
		OutputDebugStringA(message.c_str());
	}
}