#include <vector>
#include "stdafx.h"
#include "BreakPointManager.h"
#include "ModuleUtils.h"
#include "WindowsError.h"
#include "MemoryUtils.h"
#include "UniqueHandle.h"
using namespace std;

const wstring ChatTranslator::NativeOperations::BreakPointManager::MODULE_NAME = L"ffxiv_dx11.exe";
const size_t ChatTranslator::NativeOperations::BreakPointManager::VOICE_TARGET_OFFSET = 0xE770B6;
void* ChatTranslator::NativeOperations::BreakPointManager::voiceTargetInstructionAddress = nullptr;

set<DWORD> ChatTranslator::NativeOperations::BreakPointManager::threadsInDebug = set<DWORD>();

void ChatTranslator::NativeOperations::BreakPointManager::EnableDebugging(HANDLE ffxivProcessHandle)
{
	HMODULE module = ModuleUtils::GetModuleByName(ffxivProcessHandle, MODULE_NAME);
	DWORD ffxivProcessId = GetProcessId(ffxivProcessHandle);
	MODULEINFO moduleInformation;

	if (!GetModuleInformation(ffxivProcessHandle, module, &moduleInformation, sizeof(moduleInformation)))
	{
		throw WindowsError(GetLastError(), "GetModuleInformation failed");
	}

	voiceTargetInstructionAddress = reinterpret_cast<uint8_t*>(moduleInformation.lpBaseOfDll) + VOICE_TARGET_OFFSET;

	if (!DebugActiveProcess(ffxivProcessId))
	{
		throw WindowsError(GetLastError(), "DebugActiveProcess failed");
	}

	if (!DebugSetProcessKillOnExit(true))
	{
		throw WindowsError(GetLastError(), "DebugSetProcessKillOnExit failed");
	}
}

void ChatTranslator::NativeOperations::BreakPointManager::DisableDebugging(HANDLE ffxivProcessHandle)
{
	DWORD ffxivProcessId = GetProcessId(ffxivProcessHandle);
	auto aliveThreads = vector<UniqueHandle>();

	for (DWORD threadId : threadsInDebug)
	{
		if (UniqueHandle threadHandle = OpenThread(THREAD_ALL_ACCESS, false, threadId))
		{
			CONTEXT context{};
			context.ContextFlags = CONTEXT_DEBUG_REGISTERS;

			if (SuspendThread(threadHandle) == static_cast<DWORD>(-1))
			{
				throw WindowsError(GetLastError(), "SuspendThread failed");
			}
			if (!GetThreadContext(threadHandle, &context))
			{
				throw WindowsError(GetLastError(), "GetThreadContext failed");
			}

			context.Dr0 = 0x0;
			context.Dr1 = 0x0;
			context.Dr2 = 0x0;
			context.Dr3 = 0x0;
			context.Dr7 = 0x0;

			if (!SetThreadContext(threadHandle, &context))
			{
				throw WindowsError(GetLastError(), "SetThreadContext failed");
			}

			aliveThreads.emplace_back(move(threadHandle));
		}
	}

	DEBUG_EVENT debugEvent;
	while (WaitForDebugEvent(&debugEvent, 0))
	{
		ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
	}

	for (UniqueHandle& threadHandle : aliveThreads)
	{
		if (ResumeThread(threadHandle) == static_cast<DWORD>(-1))
		{
			throw WindowsError(GetLastError(), "ResumeThread failed");
		}
	}

	if (!DebugSetProcessKillOnExit(false))
	{
		throw WindowsError(GetLastError(), "DebugSetProcessKillOnExit failed");
	}

	if (!DebugActiveProcessStop(ffxivProcessId))
	{
		throw WindowsError(GetLastError(), "DebugActiveProcessStop failed");
	}
}

string ChatTranslator::NativeOperations::BreakPointManager::WaitAndReadString(HANDLE ffxivProcessHandle)
{
	string result;

	while (true)
	{
		DEBUG_EVENT debugEvent;

		if (!WaitForDebugEventEx(&debugEvent, INFINITE))
		{
			throw WindowsError(GetLastError(), "WaitForDebugEventEx failed");
		}

		switch (debugEvent.dwDebugEventCode)
		{
		case CREATE_THREAD_DEBUG_EVENT:
		case CREATE_PROCESS_DEBUG_EVENT:
		{
			HANDLE hThread = debugEvent.dwDebugEventCode == CREATE_THREAD_DEBUG_EVENT ? debugEvent.u.CreateThread.hThread : debugEvent.u.CreateProcessInfo.hThread;

			CONTEXT context{};
			context.ContextFlags = CONTEXT_DEBUG_REGISTERS;

			if (!GetThreadContext(hThread, &context))
			{
				throw WindowsError(GetLastError(), "GetThreadContext failed");
			}

			context.Dr2 = reinterpret_cast<DWORD64>(voiceTargetInstructionAddress);
			context.Dr7 = (1 << 4);

			if (!SetThreadContext(hThread, &context))
			{
				throw WindowsError(GetLastError(), "SetThreadContext failed");
			}

			threadsInDebug.emplace(debugEvent.dwThreadId);

			if (debugEvent.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT)
			{
				CloseHandle(debugEvent.u.CreateProcessInfo.hFile);
			}

			break;
		}
		case EXCEPTION_DEBUG_EVENT:
		{
			UniqueHandle threadHandle = OpenThread(THREAD_ALL_ACCESS, false, debugEvent.dwThreadId);

			if (!threadHandle)
			{
				throw WindowsError(GetLastError(), "OpenThread failed");
			}

			CONTEXT context = {};
			context.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;

			if (!GetThreadContext(threadHandle, &context))
			{
				throw WindowsError(GetLastError(), "GetThreadContext failed");
			}

			if (debugEvent.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_SINGLE_STEP || debugEvent.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT)
			{
				if (debugEvent.u.Exception.ExceptionRecord.ExceptionAddress == voiceTargetInstructionAddress)
				{
					char* stringAddress = reinterpret_cast<char*>(context.Rdx);
					result = MemoryUtils::Read<string>(ffxivProcessHandle, stringAddress);
				}
			}

			context.EFlags |= 1 << 16;

			if (!SetThreadContext(threadHandle, &context))
			{
				throw WindowsError(GetLastError(), "SetThreadContext failed");
			}

			break;
		}
		case LOAD_DLL_DEBUG_EVENT:
			CloseHandle(debugEvent.u.LoadDll.hFile);
			break;
		case EXIT_THREAD_DEBUG_EVENT:
			threadsInDebug.erase(debugEvent.dwThreadId);
			break;
		case EXIT_PROCESS_DEBUG_EVENT:
			threadsInDebug.clear();
			if (!ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE))
			{
				throw WindowsError(GetLastError(), "ContinueDebugEvent failed");
			}
			throw ProcessExitedException();
		default:
			break;
		}

		if (!ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE))
		{
			throw WindowsError(GetLastError(), "ContinueDebugEvent failed");
		}

		if (!result.empty())
		{
			return result;
		}
	}
}
