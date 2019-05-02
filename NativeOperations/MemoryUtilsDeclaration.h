#pragma once
#include <string>
#include <vector>
#include "stdafx.h"

namespace ChatTranslator::NativeOperations
{
	using namespace std;

	class MemoryUtils
	{
	public:
		class MemoryPattern
		{
		public:
			explicit MemoryPattern(const string &patternString);

			size_t GetSize() const noexcept;
			bool Match(const uint8_t *memory) const noexcept;

		protected:
			size_t size;
			unique_ptr<uint8_t[]> pattern;
			unique_ptr<bool[]> wildcard;

			static uint8_t CharToByte(char c);
			static uint8_t StringToPatternByte(const string patternString, size_t index);
		};

		static vector<void *> SearchMemory(HANDLE targetProcess, const string &memoryPattern);
		template<typename T>
		static inline T Read(HANDLE targetProcess, void *position);
		template<typename T>
		static inline void Write(HANDLE targetProcess, void *position, T value);
		static void Write(HANDLE targetProcess, void *position, const void *content, size_t contentSize);

	protected:
		static vector<size_t> FindPattern(const unique_ptr<uint8_t[]> &buffer, size_t bufferSize, const MemoryPattern &pattern);

	private:
		MemoryUtils() = delete;
	};
}
