#include "stdafx.h"
#include "ModuleUtils.h"
#include "WindowsError.h"
#include "CaseInsensitiveWCharTraits.h"
#include <filesystem>
using namespace std;

tuple<unique_ptr<HMODULE[]>, size_t> ChatTranslator::NativeOperations::ModuleUtils::GetAllModules(const HANDLE process)
{
	DWORD modulesSize = sizeof(HMODULE) * 64 / 2;
	unique_ptr<HMODULE[]> modules;
	DWORD neededSize;

	do
	{
		modulesSize *= 2;
		modules = make_unique<HMODULE[]>(modulesSize);

		if (!EnumProcessModulesEx(process, modules.get(), modulesSize, &neededSize, LIST_MODULES_64BIT))
		{
			throw WindowsError(GetLastError(), "EnumProcessModules failed");
		}
	} while (neededSize > modulesSize);

	return make_tuple(move(modules), neededSize / sizeof(HMODULE));
}

HMODULE ChatTranslator::NativeOperations::ModuleUtils::GetModuleByName(const HANDLE process, const wstring& moduleName)
{
	unique_ptr<HMODULE[]> modules;
	size_t numberOfModules;
	auto caseInsensitiveModuleName = basic_string<wchar_t, CaseInsensitiveWCharTraits>(moduleName.begin(), moduleName.end());

	tie(modules, numberOfModules) = GetAllModules(process);

	for (size_t i = 0; i < numberOfModules; i++)
	{
		DWORD bufferLength = MAX_PATH;
		unique_ptr<wchar_t[]> buffer = make_unique<wchar_t[]>(sizeof(wchar_t) * bufferLength);
		DWORD copiedLength;

		while (true)
		{
			copiedLength = GetModuleFileNameEx(process, modules[i], buffer.get(), bufferLength);

			if (copiedLength >= bufferLength)
			{
				bufferLength *= 2;
				buffer = make_unique<wchar_t[]>(sizeof(wchar_t) * bufferLength);
			}
			else if (copiedLength > 0)
			{
				auto modulePath = filesystem::path(buffer.get(), buffer.get() + copiedLength);
				if (caseInsensitiveModuleName.compare(modulePath.filename().c_str()) == 0)
				{
					return modules[i];
				}
				else
				{
					break;
				}
			}
			else
			{
				throw WindowsError(GetLastError(), "GetModuleFileNameEx failed");
			}
		}
	}

	return nullptr;
}
