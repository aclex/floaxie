/*
 * Copyright 2015 Alexey Chernov <4ernov@gmail.com>
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

#ifndef FLOAXIE_FTOA_H
#define FLOAXIE_FTOA_H

#include <cmath>
#include <cstddef>
#include <cassert>

#include <floaxie/grisu.h>
#include <floaxie/prettify.h>

namespace floaxie
{
	/** \brief Returns maximum size of buffer can ever be required by `ftoa()`.
	 *
	 * Maximum size of buffer passed to `ftoa()` guaranteed not to lead to
	 * undefined behaviour.
	 *
	 * \tparam FloatType floating point type, which value is planned to be
	 * printed to the buffer.
	 *
	 * \return maximum size of buffer, which can ever be used in the very worst
	 * case.
	 */
	template<typename FloatType> constexpr std::size_t max_buffer_size() noexcept
	{
		// digits, '.' (or 'e' plus three-digit power with optional sign) and '\0'
		return max_digits<FloatType>() + 1 + 1 + 3 + 1;
	}

	/** \brief Prints floating point value to optimal string representation.
	 *
	 * The function prints the string representation of the specified floating
	 * point value using
	 * [**Grisu2**](http://florian.loitsch.com/publications/dtoa-pldi2010.pdf)
	 * algorithm and tries to get it as shorter, as possible. Usually it
	 * succeeds, but sometimes fails, and the output representation is not
	 * the shortest for this value. For the sake of speed improvement this is
	 * ignored, while there's **Grisu3** algorithm which rules this out
	 * informing the caller of the failure, so that it can call slower, but
	 * more accurate algorithm in this case.
	 *
	 * The format of the string representation is one of the following:
	 * 1. Decimal notation, which contains:
	 *  - minus sign ('-') in case of negative value
	 *  - sequence of one or more decimal digits optionally containing
	 *    decimal point character ('.')
	 * 2. Decimal exponent notation, which contains:
	 *  - minus ('-') sign in case of negative value
	 *  - sequence of one or more decimal digits optionally containing
	 *    decimal point character ('.')
	 *  - 'e' character followed by minus sign ('-') in case of negative
	 *    power of the value (i.e. the specified value is < 1) and
	 *    sequence of one, two of three decimal digits.
	 *
	 * \tparam FloatType type of floating point value, calculated using passed
	 * input parameter \p **v**
	 * \tparam CharType character type (typically `char` or `wchar_t`) of the
	 * output buffer \p **buffer**
	 *
	 * \param v floating point value to print
	 * \param buffer character buffer of enough size (see `max_buffer_size()`)
	 * to print the representation to
	 *
	 * \see `max_buffer_size()`
	 */
	template<typename FloatType, typename CharType> inline void ftoa(FloatType v, CharType* buffer) noexcept
	{
		assert(!std::isnan(v));
		assert(!std::isinf(v));

		if (v == 0)
		{
			buffer[0] = '0';
			buffer[1] = '.';
			buffer[2] = '0';
			buffer[3] = '\0';
		}
		else
		{
			*buffer = '-';
			buffer += v < 0;

			constexpr int alpha(-35), gamma(-32);
			constexpr unsigned int decimal_scientific_threshold(16);

			int len, K;

			grisu2<alpha, gamma>(v, buffer, &len, &K);
			prettify<decimal_scientific_threshold>(buffer, len, K);
		}
	}
}

#endif // FLOAXIE_FTOA_H
