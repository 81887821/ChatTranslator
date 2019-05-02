#pragma once
#include <string>
#include <memory>
#include "stdafx.h"
#include "MemoryUtilsDeclaration.h"
#include "WindowsError.h"

template<typename T>
inline T ChatTranslator::NativeOperations::MemoryUtils::Read(HANDLE targetProcess, void *position)
{
	auto buffer = std::make_unique<T>();
	size_t readSize;

	if (ReadProcessMemory(targetProcess, position, buffer.get(), sizeof(T), &readSize))
	{
		if (readSize >= sizeof(T))
		{
			return *buffer;
		}
	}

	throw WindowsError(GetLastError());
}

template<>
inline std::string ChatTranslator::NativeOperations::MemoryUtils::Read(HANDLE targetProcess, void *position)
{
	auto string = std::string();
	size_t bufferSize = 64;
	auto buffer = std::make_unique<uint8_t[]>(64);
	auto currentPosition = reinterpret_cast<uint8_t *>(position);
	size_t readSize;

	while (true)
	{
		if (ReadProcessMemory(targetProcess, currentPosition, buffer.get(), bufferSize, &readSize))
		{
			size_t stringEndIndex = 0;

			while (stringEndIndex < readSize && buffer[stringEndIndex] != 0)
			{
				stringEndIndex++;
			}

			string.append(buffer.get(), buffer.get() + stringEndIndex);

			if (buffer[stringEndIndex] == 0)
			{
				return string;
			}
			else
			{
				currentPosition += readSize;
			}
		}
		else
		{
			throw WindowsError(GetLastError());
		}
	}
}

template<typename T>
inline void ChatTranslator::NativeOperations::MemoryUtils::Write(HANDLE targetProcess, void *position, T value)
{
	if (!WriteProcessMemory(targetProcess, position, &value, sizeof(T), nullptr))
	{
		throw WindowsError(GetLastError());
	}
}
