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

#ifndef FLOAXIE_ATOF_H
#define FLOAXIE_ATOF_H

#include <floaxie/take_apart.h>
#include <floaxie/crosh.h>

#include <floaxie/default_fallback.h>

#include <iostream>

namespace floaxie
{
	template
	<
		typename FloatType,
		typename FallbackCallable = FloatType (const char*, char**)
	>
	inline FloatType atof(const char* str, char** str_end, FallbackCallable fallback_func = default_fallback<FloatType, char>)
	{
		char buffer[max_buffer_length];
		bool sign;
		int len, K;

		take_apart(str, const_cast<const char**>(str_end), buffer, &sign, &len, &K);

		if (*str_end == str)
			return 0;

		bool accurate(false);

		auto ret = crosh<FloatType>(buffer, len, K, &accurate);

		if (!accurate)
			return fallback_func(str, str_end);

		if (!sign)
			ret = -ret;

		return ret;
	}
}

#endif // FLOAXIE_ATOF_H
