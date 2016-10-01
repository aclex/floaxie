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

#include <floaxie/krosh.h>

#include <floaxie/default_fallback.h>

/** \brief Floaxie functions templates.
 *
 * This namespace contains two main public floaxie functions (`atof()` and
 * `ftoa()`), as well as several helper functions (e.g. `max_buffer_size()`)
 * and internal type and function templates.
 */
namespace floaxie
{
	/** \brief Parses floating point string representation.
	 *
	 * Interprets string representation of floating point value using Krosh
	 * algorithm and, if successful, value of the specified type is returned.
	 *
	 * The accepted representation format is ordinary or exponential decimal
	 * floating point expression, containing:
	 *   - optional sign ('+' or '-')
	 *   - sequence of one or more decimal digits optionally containing decimal
	 *     point character ('.')
	 *   - optional 'e' of 'E' character followed by optional sign ('+' or '-')
	 *     and sequence of one or more decimal digits.
	 *
	 * Function doesn't expects any preceding spacing characters and treats the
	 * representation as incorrect, if there's any.
	 *
	 * \tparam FloatType target floating point type to store results.
	 * \tparam FallbackCallable fallback conversion function type, in case of
	 * Krosh is unsure if the result is correctly rounded (default is strtof()
	 * for floats, strtod() for doubles, strtold() for long doubles).
	 *
	 * \param str buffer containing the string representation of the value.
	 * \param str_end out parameter, which will contain a pointer to first
	 * character after the parsed value in the specified buffer. If str_end is
	 * NULL, it is ignored.
	 * \param fallback_func pointer to fallback function.
	 *
	 * \return parsed value, if the input is correct, default constructed value
	 * otherwise.
	 */
	template
	<
		typename FloatType,
		typename FallbackCallable = FloatType (const char*, char**)
	>
	inline FloatType atof(const char* str, char** str_end, FallbackCallable fallback_func = default_fallback<FloatType, char>)
	{
		const auto& cr(krosh<FloatType>(str));

		if (cr.str_end == str)
			return 0;

		if (!cr.is_accurate)
			return fallback_func(str, str_end);

		if (str_end)
			*str_end = const_cast<char*>(cr.str_end);

		return cr.value;
	}
}

#endif // FLOAXIE_ATOF_H
