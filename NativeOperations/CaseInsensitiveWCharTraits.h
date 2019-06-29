#pragma once
#include <string>

namespace ChatTranslator::NativeOperations
{
	class CaseInsensitiveWCharTraits : public std::char_traits<wchar_t>
	{
	public:
		static bool eq(const char_type& left, const char_type& right) noexcept
		{
			return towlower(left) == towlower(right);
		}

		static bool lt(const char_type& left, const char_type& right) noexcept
		{
			return towlower(left) < towlower(right);
		}

		static int compare(const char_type* const left, const char_type* const right, size_t count)
		{
			for (size_t i = 0; i < count; i++)
			{
				wint_t leftChar = towlower(left[i]);
				wint_t rightChar = towlower(right[i]);

				if (leftChar != rightChar)
				{
					return leftChar - rightChar;
				}
			}

			return 0;
		}

		static const char_type* find(const char_type* string, size_t count, const char_type& character)
		{
			for (size_t i = 0; i < count; i++)
			{
				if (towlower(string[i]) == towlower(character))
				{
					return &string[i];
				}
			}

			return nullptr;
		}
	};
}