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

		explicit inline WindowsError(DWORD errorCode, const string &errorMessage = string()) : errorCode(errorCode)
		{
			LPSTR messageBuffer = nullptr;

			if (!errorMessage.empty())
			{
				message = errorMessage;
				message.append(": ");
			}

			if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr))
			{
				message.append(messageBuffer);
				LocalFree(messageBuffer);
			}
			else
			{
				message.append("Failed to get error message.");
			}
		}

		~WindowsError()
		{
		}

		const char *what() const noexcept override
		{
			return message.c_str();
		}

	protected:
		string message;
	};
}
