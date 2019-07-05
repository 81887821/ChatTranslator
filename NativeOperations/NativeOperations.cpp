#include <stdexcept>
#include <vector>
#include <limits>
#include "stdafx.h"
#include "NativeOperations.h"
#include "ProcessHelper.h"
#include "MemoryUtils.h"
#include "Constants.h"
#include "Macro.h"
#include "BreakPointManager.h"
#include <fstream>
#include <iostream>
using namespace std;
using namespace ChatTranslator::NativeOperations;

#undef max

DLL_EXPORT DllErrorCode SearchMacroLocation(HANDLE ffxivProcessHandle, void **macroPosition)
{
	try
	{
		vector<void *> memoryPoints = MemoryUtils::SearchMemory(ffxivProcessHandle, FFXIV::MACRO_MEMORY_PATTERN);

		switch (memoryPoints.size())
		{
		case 0:
			*macroPosition = nullptr;
			return DllErrorCode::MemoryPointNotFound;
		case 1:
			*macroPosition = memoryPoints[0];
			return DllErrorCode::Success;
		default:
			*macroPosition = memoryPoints[0];
			return DllErrorCode::MultipleMemoryPointsFound;
		}
	}
	catch (exception &)
	{
		return DllErrorCode::InternalError;
	}
}

DLL_EXPORT DllErrorCode GetMacroName(HANDLE ffxivProcessHandle, void *macroPosition, int32_t macroNumber, void *buffer, int32_t bufferCapacity, int32_t *macroNameLength)
{
	try
	{
		string macroName = Macro::GetMacroName(ffxivProcessHandle, macroPosition, macroNumber);

		if (macroName.size() <= numeric_limits<int32_t>::max())
		{
			*macroNameLength = static_cast<int32_t>(macroName.size());

			if (bufferCapacity >= 0 && static_cast<size_t>(bufferCapacity) >= macroName.size())
			{
				memcpy(buffer, macroName.data(), macroName.size());
				return DllErrorCode::Success;
			}
			else
			{
				return DllErrorCode::InsufficientBufferSize;
			}
		}
		else
		{
			return DllErrorCode::InternalError;
		}
	}
	catch (InvalidMacroNumberException &)
	{
		return DllErrorCode::InvalidMacroNumber;
	}
	catch (exception &)
	{
		return DllErrorCode::InternalError;
	}
}

DLL_EXPORT DllErrorCode GetMacroLineCapacity(HANDLE ffxivProcessHandle, void *macroPosition, int32_t macroNumber, int32_t macroLine, int32_t *capacity)
{
	try
	{
		size_t macroCapacity = Macro::GetMacroLineCapacity(ffxivProcessHandle, macroPosition, macroNumber, macroLine);

		if (macroCapacity <= numeric_limits<int32_t>::max())
		{
			*capacity = static_cast<int32_t>(macroCapacity);
			return DllErrorCode::Success;
		}
		else
		{
			return DllErrorCode::InternalError;
		}
	}
	catch (InvalidMacroNumberException &)
	{
		return DllErrorCode::InvalidMacroNumber;
	}
	catch (InvalidMacroLineException &)
	{
		return DllErrorCode::InvalidMacroLine;
	}
	catch (exception &)
	{
		return DllErrorCode::InternalError;
	}
}

DLL_EXPORT DllErrorCode WriteMacro(HANDLE ffxivProcessHandle, void *macroPosition, int32_t macroNumber, int32_t macroLine, const void *content, int32_t contentSize)
{
	try
	{
		if (contentSize > 0)
		{
			Macro::WriteMacro(ffxivProcessHandle, macroPosition, macroNumber, macroLine, content, contentSize);
		}
		return DllErrorCode::Success;
	}
	catch (InvalidMacroNumberException &)
	{
		return DllErrorCode::InvalidMacroNumber;
	}
	catch (InvalidMacroLineException &)
	{
		return DllErrorCode::InvalidMacroLine;
	}
	catch (InsufficientMacroLineCapacityException &)
	{
		return DllErrorCode::InsufficientMacroLineCapacity;
	}
	catch (exception &)
	{
		return DllErrorCode::InternalError;
	}
}

DLL_EXPORT DllErrorCode GetFFXIVWindowHandle(HANDLE ffxivProcessHandle, HWND *windowHandle)
{
	try
	{
		auto className = wstring(L"FFXIVGAME");
		auto windowName = wstring(L"FINAL FANTASY XIV");

		*windowHandle = ProcessHelper::GetWindow(ffxivProcessHandle, className, windowName);
		return DllErrorCode::Success;
	}
	catch (WindowNotFoundException &)
	{
		return DllErrorCode::NoWindowFound;
	}
	catch (exception &)
	{
		return DllErrorCode::InternalError;
	}
}

DLL_EXPORT DllErrorCode SendKeyEvent(HWND windowHandle, int32_t virtualKeyCode, bool keyDownEvent)
{
	UINT message = keyDownEvent ? WM_KEYDOWN : WM_KEYUP;
	uint32_t lParam = keyDownEvent ? 0x0 : 0xC0000001;

	if (PostMessage(windowHandle, message, virtualKeyCode, lParam))
	{
		return DllErrorCode::Success;
	}
	else
	{
		return DllErrorCode::InternalError;
	}
}

DLL_EXPORT DllErrorCode StartReading(HANDLE ffxivProcessHandle)
{
	try
	{
		BreakPointManager::EnableDebugging(ffxivProcessHandle);
		return DllErrorCode::Success;
	}
	catch (exception&)
	{
		return DllErrorCode::InternalError;
	}
}

DLL_EXPORT DllErrorCode StopReading(HANDLE ffxivProcessHandle)
{
	try
	{
		BreakPointManager::DisableDebugging(ffxivProcessHandle);
		return DllErrorCode::Success;
	}
	catch (exception&)
	{
		return DllErrorCode::InternalError;
	}
}

DLL_EXPORT DllErrorCode WaitAndReadScenarioString(HANDLE ffxivProcessHandle, void* buffer, int32_t bufferCapacity, int32_t* stringLength)
{
	try
	{
		string string = BreakPointManager::WaitAndReadString(ffxivProcessHandle);

		if (string.size() <= numeric_limits<int32_t>::max())
		{
			*stringLength = static_cast<int32_t>(string.length());

			if (static_cast<size_t>(bufferCapacity) >= string.length())
			{
				memcpy(buffer, string.data(), string.size());
				return DllErrorCode::Success;
			}
			else
			{
				return DllErrorCode::InsufficientBufferSize;
			}
		}
		else
		{
			return DllErrorCode::InternalError;
		}
	}
	catch (ProcessExitedException&)
	{
		return DllErrorCode::ProcessExited;
	}
	catch (exception&)
	{
		return DllErrorCode::InternalError;
	}
}
