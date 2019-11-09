#pragma once
#include <string>
#include <tuple>
#include <set>
#include "DllException.h"

namespace ChatTranslator::NativeOperations
{
	using namespace std;

	class BreakPointManager
	{
	public:
		static void EnableDebugging(HANDLE ffxivProcessHandle);
		static void DisableDebugging(HANDLE ffxivProcessHandle);
		static string WaitAndReadString(HANDLE ffxivProcessHandle);

	protected:
		static const wstring MODULE_NAME;
		static const size_t VOICE_TARGET_OFFSET;
		static void* voiceTargetInstructionAddress;

		static set<DWORD> threadsInDebug;
	};

	class ProcessExitedException : public DllException
	{
	};
}
