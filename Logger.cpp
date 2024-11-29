#include "Logger.h"

void Logger {
	void log(const std::string& message) { OutputDebugStringA(message.c_str()); }
}