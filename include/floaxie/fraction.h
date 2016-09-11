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

#ifndef FLOAXIE_FRACTION_H
#define FLOAXIE_FRACTION_H

#include <cstddef>

#include <floaxie/static_pow.h>

namespace floaxie
{
	template
	<
		typename T,
		std::size_t decimal_digits,
		std::size_t binary_digits,
		std::size_t current_binary_digit,
		bool terminal = (binary_digits == current_binary_digit)
	>
	struct fraction_converter;

	template
	<
		typename T,
		std::size_t decimal_digits,
		std::size_t binary_digits,
		std::size_t current_binary_digit
	>
	struct fraction_converter<T, decimal_digits, binary_digits, current_binary_digit, false>
	{
		static T convert(T decimal_numerator)
		{
			constexpr T numerator(static_pow<10, decimal_digits>());
			constexpr T denominator(static_pow<2, current_binary_digit>());
			constexpr T decimal_fraction(numerator / denominator);

			constexpr std::size_t shift_amount(binary_digits - current_binary_digit);

			const T decision(decimal_numerator >= decimal_fraction);
			const T residue(decimal_numerator - decision * decimal_fraction);

			return (decision << shift_amount) |
				fraction_converter<T, decimal_digits, binary_digits, current_binary_digit + 1>::convert(residue);
		}
	};

	template
	<
		typename T,
		std::size_t decimal_digits,
		std::size_t binary_digits,
		std::size_t current_binary_digit
	>
	struct fraction_converter<T, decimal_digits, binary_digits, current_binary_digit, true>
	{
		static T convert(T decimal_numerator)
		{
			constexpr T numerator(static_pow<10, decimal_digits>());
			constexpr T denominator(static_pow<2, current_binary_digit>());
			constexpr T decimal_fraction(numerator / denominator);

			return decimal_numerator >= decimal_fraction;
		}
	};

	template<std::size_t decimal_digits, std::size_t binary_digits, typename T> inline T convert_numerator(T decimal_numerator)
	{
		return fraction_converter<T, decimal_digits, binary_digits, 1>::convert(decimal_numerator);
	}
}

#endif // FLOAXIE_FRACTION_H
