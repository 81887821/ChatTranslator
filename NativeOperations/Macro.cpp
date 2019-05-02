#include "stdafx.h"
#include "Macro.h"
#include "MemoryUtils.h"
using namespace std;

string ChatTranslator::NativeOperations::Macro::GetMacroName(HANDLE ffxivProcessHandle, void *macroBase, int macroNumber)
{
	void *macroNamePointerPosition = GetMacroNamePointerPosition(macroBase, macroNumber);
	void *macroNamePointer = MemoryUtils::Read<void *>(ffxivProcessHandle, macroNamePointerPosition);
	return MemoryUtils::Read<string>(ffxivProcessHandle, macroNamePointer);
}

size_t ChatTranslator::NativeOperations::Macro::GetMacroLineCapacity(HANDLE ffxivProcessHandle, void * macroBase, int macroNumber, int macroLine)
{
	void *macroNamePointerPosition = GetMacroNamePointerPosition(macroBase, macroNumber);
	void *macroLinePointerPosition = GetMacroLinePointerPosition(macroNamePointerPosition, macroLine);
	void *lineCapacityPosition = GetLineCapacityPosition(macroLinePointerPosition);
	return MemoryUtils::Read<size_t>(ffxivProcessHandle, lineCapacityPosition);
}

void ChatTranslator::NativeOperations::Macro::WriteMacro(HANDLE ffxivProcessHandle, void *macroBase, int macroNumber, int macroLine, const void *content, size_t contentSize)
{
	void *macroNamePointerPosition = GetMacroNamePointerPosition(macroBase, macroNumber);
	void *macroLinePointerPosition = GetMacroLinePointerPosition(macroNamePointerPosition, macroLine);
	void *lineCapacityPosition = GetLineCapacityPosition(macroLinePointerPosition);
	void *lineLengthPosition = GetLineLengthPosition(macroLinePointerPosition);
	void *macroLinePointer = MemoryUtils::Read<void *>(ffxivProcessHandle, macroLinePointerPosition);
	size_t lineCapacity = MemoryUtils::Read<size_t>(ffxivProcessHandle, lineCapacityPosition);

	if (lineCapacity >= contentSize)
	{
		MemoryUtils::Write(ffxivProcessHandle, macroLinePointer, content, contentSize);
		MemoryUtils::Write(ffxivProcessHandle, lineLengthPosition, contentSize);
	}
	else
	{
		throw InsufficientMacroLineCapacityException();
	}
}

void *ChatTranslator::NativeOperations::Macro::GetMacroNamePointerPosition(void *macroBase, int macroNumber)
{
	if (macroNumber >= 0 && macroNumber < 100)
	{
		auto base = reinterpret_cast<uint8_t *>(macroBase);
		return base + MACRO_BASE_TO_FIRST_MACRO + MACRO_INTERVAL * macroNumber;
	}
	else
	{
		throw InvalidMacroNumberException();
	}
}

void *ChatTranslator::NativeOperations::Macro::GetMacroLinePointerPosition(void *macroNamePointerPosition, int line)
{
	if (line >= 0 && line < 15)
	{
		auto name = reinterpret_cast<uint8_t *>(macroNamePointerPosition);
		return name + MACRO_LINE_INTERVAL * (line + 1);
	}
	else
	{
		throw InvalidMacroLineException();
	}
}

void * ChatTranslator::NativeOperations::Macro::GetLineCapacityPosition(void * macroLinePointerPosition)
{
	auto position = reinterpret_cast<uint8_t *>(macroLinePointerPosition);
	return position + MACRO_LINE_TO_LINE_CAPACITY;
}

void *ChatTranslator::NativeOperations::Macro::GetLineLengthPosition(void *macroLinePointerPosition)
{
	auto position = reinterpret_cast<uint8_t *>(macroLinePointerPosition);
	return position + MACRO_LINE_TO_LINE_LENGTH;
}
