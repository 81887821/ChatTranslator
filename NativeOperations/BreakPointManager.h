#pragma once
#include <string>
#include <tuple>
#include <vector>

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
		static const size_t TARGET_OFFSET;
		static const size_t TARGET_INSTRUCTION_SIZE;
		static void* targetInstructionAddress;
		static char* targetStringAddress;

		static const size_t VOICE_TARGET_OFFSET;
		static const size_t VOICE_TARGET_INSTRUCTION_SIZE;
		static void* voiceTargetInstructionAddress;
		static char* voiceTargetStringAddress;

		static vector<DWORD> threadsInDebug;
	};
}
