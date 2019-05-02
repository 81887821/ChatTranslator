#pragma once
#include "stdafx.h"
#include "DllErrorCode.h"

#define DLL_EXPORT __declspec(dllexport)

extern "C"
{
	using namespace ChatTranslator::NativeOperations;

	DLL_EXPORT DllErrorCode SearchMacroLocation(HANDLE ffxivProcessHandle, void **macroPosition);
	DLL_EXPORT DllErrorCode GetMacroName(HANDLE ffxivProcessHandle, void *macroPosition, int32_t macroNumber, void *buffer, int32_t bufferCapacity, int32_t *macroNameLength);
	DLL_EXPORT DllErrorCode GetMacroLineCapacity(HANDLE ffxivProcessHandle, void *macroPosition, int32_t macroNumber, int32_t macroLine, int32_t *capacity);
	DLL_EXPORT DllErrorCode WriteMacro(HANDLE ffxivProcessHandle, void *macroPosition, int32_t macroNumber, int32_t macroLine, const void *content, int32_t contentSize);
	DLL_EXPORT DllErrorCode GetFFXIVWindowHandle(HANDLE ffxivProcessHandle, HWND *windowHandle);
	DLL_EXPORT DllErrorCode SendKeyEvent(HWND windowHandle, int32_t virtualKeyCode, bool keyDownEvent);
}
