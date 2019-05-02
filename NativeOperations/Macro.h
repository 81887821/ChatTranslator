#pragma once
#include <string>
#include "DllException.h"

namespace ChatTranslator::NativeOperations
{
	using namespace std;

	class Macro
	{
	public:
		static string GetMacroName(HANDLE ffxivProcessHandle, void *macroBase, int macroNumber);
		static size_t GetMacroLineCapacity(HANDLE ffxivProcessHandle, void *macroBase, int macroNumber, int macroLine);
		static void WriteMacro(HANDLE ffxivProcessHandle, void *macroBase, int macroNumber, int macroLine, const void *content, size_t contentSize);

	protected:
		static const size_t MACRO_BASE_TO_FIRST_MACRO = 0x52;
		static const size_t MACRO_INTERVAL = 0x688;
		static const size_t MACRO_LINE_INTERVAL = 0x68;
		static const size_t MACRO_LINE_TO_LINE_CAPACITY = 0x8;
		static const size_t MACRO_LINE_TO_LINE_LENGTH = 0x10;

		static void *GetMacroNamePointerPosition(void *macroBase, int macroNumber);
		static void *GetMacroLinePointerPosition(void *macroNamePointerPosition, int line);
		static void *GetLineCapacityPosition(void *macroLinePointerPosition);
		static void *GetLineLengthPosition(void *macroLinePointerPosition);
	};

	class InvalidMacroNumberException : public DllException
	{
	};

	class InvalidMacroLineException : public DllException
	{
	};

	class InsufficientMacroLineCapacityException : public DllException
	{
	};
}
