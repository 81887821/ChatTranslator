#include <memory>
#include "stdafx.h"
#include "ProcessHelper.h"
#include "UniqueHandle.h"
#include "WindowsError.h"
using namespace std;

vector<DWORD> ChatTranslator::NativeOperations::ProcessHelper::FindProcessIdsByName(const wstring &processName)
{
	auto processes = vector<DWORD>();
	auto snapshotHandle = UniqueHandle(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0), CloseHandle);
	PROCESSENTRY32 processEntry;

	// Take a snapshot of all processes in the system.
	if (snapshotHandle == INVALID_HANDLE_VALUE)
	{
		throw runtime_error("CreateToolhelp32Snapshot failed.");
	}

	processEntry.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(snapshotHandle, &processEntry))
	{
		throw runtime_error("Process32First failed.");
	}

	do
	{
		if (processName == processEntry.szExeFile)
		{
			processes.emplace_back(processEntry.th32ProcessID);
		}
	}
	while (Process32Next(snapshotHandle, &processEntry));

	return processes;
}

HWND ChatTranslator::NativeOperations::ProcessHelper::GetWindow(HANDLE processHandle, const wstring &className, const wstring &windowName)
{
	DWORD pid = GetProcessId(processHandle);
	HWND windowHandle = nullptr;

	while (windowHandle = FindWindowExW(nullptr, windowHandle, className.c_str(), windowName.c_str()))
	{
		DWORD windowOwnerPid;

		GetWindowThreadProcessId(windowHandle, &windowOwnerPid);

		if (windowOwnerPid == pid)
		{
			return windowHandle;
		}
	}

	throw WindowNotFoundException();
}
