#pragma once
#include <functional>
#include "stdafx.h"

namespace ChatTranslator::NativeOperations
{
	using namespace std;

	class UniqueHandle
	{
	public:
		UniqueHandle() : handle(INVALID_HANDLE_VALUE), closer(nullptr)
		{
		}
		UniqueHandle(HANDLE handle, const function<void(HANDLE)> &closer = CloseHandle) : handle(handle), closer(closer)
		{
		}
		UniqueHandle(const UniqueHandle &other) = delete;
		UniqueHandle(UniqueHandle &&other) noexcept : handle(other.handle), closer(move(other.closer))
		{
			other.handle = INVALID_HANDLE_VALUE;
			other.closer = nullptr;
		}

		~UniqueHandle()
		{
			if (closer)
			{
				closer(handle);
			}
		}

		HANDLE Get() const
		{
			return handle;
		}

		operator HANDLE() const
		{
			return handle;
		}

		UniqueHandle &operator=(const UniqueHandle &other) = delete;
		UniqueHandle &operator=(UniqueHandle &&other) noexcept
		{
			handle = other.handle;
			closer = move(other.closer);

			other.handle = INVALID_HANDLE_VALUE;
			other.closer = nullptr;
		}

	protected:
		HANDLE handle;
		std::function<void(HANDLE)> closer;
	};
}
