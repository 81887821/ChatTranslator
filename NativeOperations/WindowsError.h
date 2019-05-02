#pragma once
#include "stdafx.h"
#include "DllException.h"

namespace ChatTranslator::NativeOperations
{
	using namespace std;

	class WindowsError : public DllException
	{
	public:
		const DWORD errorCode;

		explicit WindowsError(DWORD errorCode) : errorCode(errorCode)
		{
			FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr);
		}

		~WindowsError()
		{
			LocalFree(messageBuffer);
		}

		const char *what() const noexcept override
		{
			if (messageBuffer)
			{
				return messageBuffer;
			}
			else
			{
				return "Failed to get error message.";
			}
		}

	protected:
		LPSTR messageBuffer = nullptr;
	};
}
