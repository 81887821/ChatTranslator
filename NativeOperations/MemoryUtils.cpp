#include <memory>
#include "stdafx.h"
#include "MemoryUtils.h"
#include "WindowsError.h"
#include "MemoryUtilsDeclaration.h"
using namespace std;

ChatTranslator::NativeOperations::MemoryUtils::MemoryPattern::MemoryPattern(const string &patternString)
{
	size = patternString.length() / 2;
	pattern = make_unique<uint8_t[]>(size);
	wildcard = make_unique<bool[]>(size);

	for (size_t i = 0; i < size; i++)
	{
		if (patternString[i * 2] == '?')
		{
			if (patternString[i * 2 + 1] == '?')
			{
				pattern[i] = 0;
				wildcard[i] = true;
			}
			else
			{
				throw invalid_argument("Wildcard of half byte is not allowed.");
			}
		}
		else if (patternString[i * 2 + 1] == '?')
		{
			throw invalid_argument("Wildcard of half byte is not allowed.");
		}
		else
		{
			pattern[i]= StringToPatternByte(patternString, i * 2);
			wildcard[i] = false;
		}
	}
}

size_t ChatTranslator::NativeOperations::MemoryUtils::MemoryPattern::GetSize() const noexcept
{
	return size;
}

bool ChatTranslator::NativeOperations::MemoryUtils::MemoryPattern::Match(const uint8_t *memory) const noexcept
{
	bool is_match = true;

	for (size_t i = 0; i < size; i++)
	{
		if (!wildcard[i] && pattern[i] != memory[i])
		{
			is_match = false;
			break;
		}
	}

	return is_match;
}

uint8_t ChatTranslator::NativeOperations::MemoryUtils::MemoryPattern::CharToByte(char c)
{
	if (c >= 'A' && c <= 'F')
	{
		return c - 'A' + 10;
	}
	else if (c >= 'a' && c <= 'f')
	{
		return c - 'a' + 10;
	}
	else if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	else
	{
		throw invalid_argument(std::string("Cannot convert ") + to_string(c) + "to byte.");
	}
}

uint8_t ChatTranslator::NativeOperations::MemoryUtils::MemoryPattern::StringToPatternByte(const string patternString, size_t index)
{
	return (CharToByte(patternString[index]) << 4) + CharToByte(patternString[index + 1]);
}

vector<void*> ChatTranslator::NativeOperations::MemoryUtils::SearchMemory(HANDLE targetProcess, const string &patternString)
{
	SYSTEM_INFO systemInfo;
	MEMORY_BASIC_INFORMATION memoryInformation;
	GetSystemInfo(&systemInfo);
	size_t bufferSize = 4 * (1 << 20);
	auto buffer = make_unique<uint8_t[]>(bufferSize);
	auto pattern = MemoryPattern(patternString);
	auto foundPoints = vector<void *>();

	uint8_t *current = reinterpret_cast<uint8_t *>(systemInfo.lpMinimumApplicationAddress);
	uint8_t *maximumAddress = reinterpret_cast<uint8_t *>(systemInfo.lpMaximumApplicationAddress);

	while (current < maximumAddress)
	{
		if (VirtualQueryEx(targetProcess, current, &memoryInformation, sizeof(memoryInformation)))
		{
			if (memoryInformation.Protect == PAGE_READWRITE && memoryInformation.State == MEM_COMMIT)
			{
				if (memoryInformation.RegionSize > bufferSize)
				{
					bufferSize = memoryInformation.RegionSize;
					buffer = make_unique<uint8_t[]>(bufferSize);
				}

				size_t readSize;

				if (ReadProcessMemory(targetProcess, current, buffer.get(), memoryInformation.RegionSize, &readSize))
				{
					for (size_t index : FindPattern(buffer, readSize, pattern))
					{
						foundPoints.emplace_back(current + index);
					}
				}
				else
				{
					throw WindowsError(GetLastError());
				}
			}
		}
		else
		{
			throw WindowsError(GetLastError());
		}

		current += memoryInformation.RegionSize;
	}

	return foundPoints;
}

void ChatTranslator::NativeOperations::MemoryUtils::Write(HANDLE targetProcess, void *position, const void *content, size_t contentSize)
{
	if (!WriteProcessMemory(targetProcess, position, content, contentSize, nullptr))
	{
		throw WindowsError(GetLastError());
	}
}

vector<size_t> ChatTranslator::NativeOperations::MemoryUtils::FindPattern(const unique_ptr<uint8_t[]>& buffer, size_t bufferSize, const MemoryPattern &pattern)
{
	auto foundPointIndices = vector<size_t>();
	size_t patternSize = pattern.GetSize();

	for (size_t i = 0; i <= bufferSize - patternSize; i++)
	{
		if (pattern.Match(&buffer[i]))
		{
			foundPointIndices.emplace_back(i);
		}
	}

	return foundPointIndices;
}
