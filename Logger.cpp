//#include "Logger.h"



namespace Logger {
    void log(const std::string& message) {
		OutputDebugStringA(message.c_str());
	}
}