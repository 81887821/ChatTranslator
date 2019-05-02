#pragma once
#include <vector>
#include <string>
#include "stdafx.h"
#include "DllException.h"

namespace ChatTranslator::NativeOperations
{
	using namespace std;

	class ProcessHelper
	{
	public:
		static vector<DWORD> FindProcessIdsByName(const wstring &processName);
		static HWND GetWindow(HANDLE processHandle, const wstring &className, const wstring &windowName);

	private:
		ProcessHelper() = delete;
	};

	class WindowNotFoundException : public DllException
	{
	};
}
