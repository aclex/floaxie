/*
 * Copyright 2015, 2016 Alexey Chernov <4ernov@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FLOAXIE_DEFAULT_FALLBACK_H
#define FLOAXIE_DEFAULT_FALLBACK_H

#include <cstdlib>
#include <cwchar>

namespace floaxie
{
	template<typename FloatType, typename CharType> FloatType default_fallback(const CharType* str, CharType** str_end);

	template<> inline float default_fallback<float, char>(const char* str, char** str_end)
	{
		return std::strtof(str, str_end);
	}

	template<> inline double default_fallback<double, char>(const char* str, char** str_end)
	{
		return std::strtod(str, str_end);
	}

	template<> inline long double default_fallback<long double, char>(const char* str, char** str_end)
	{
		return std::strtold(str, str_end);
	}

	template<> inline float default_fallback<float, wchar_t>(const wchar_t* str, wchar_t** str_end)
	{
		return std::wcstof(str, str_end);
	}

	template<> inline double default_fallback<double, wchar_t>(const wchar_t* str, wchar_t** str_end)
	{
		return std::wcstod(str, str_end);
	}

	template<> inline long double default_fallback<long double, wchar_t>(const wchar_t* str, wchar_t** str_end)
	{
		return std::wcstold(str, str_end);
	}
}

#endif // FLOAXIE_DEFAULT_FALLBACK_H
