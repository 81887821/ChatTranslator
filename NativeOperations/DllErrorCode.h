#pragma once
#include <cstdint>

namespace ChatTranslator::NativeOperations
{
	enum DllErrorCode : int32_t
	{
		Success = 0,
		InternalError = -1,
		MemoryPointNotFound = -2,
		MultipleMemoryPointsFound = -3,
		InsufficientBufferSize = -4,
		InvalidMacroNumber = -5,
		InvalidMacroLine = -6,
		InsufficientMacroLineCapacity = -7,
		NoWindowFound = -8,
	};
}
