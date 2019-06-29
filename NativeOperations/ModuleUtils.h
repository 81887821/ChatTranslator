#pragma once
#include "stdafx.h"
#include <tuple>
#include <memory>
#include <string>

namespace ChatTranslator::NativeOperations
{
	using namespace std;

	class ModuleUtils
	{
	public:
		static tuple<unique_ptr<HMODULE[]>, size_t> GetAllModules(const HANDLE process);
		static HMODULE GetModuleByName(const HANDLE process, const wstring& moduleName);
	};
}
