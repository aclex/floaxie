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
 *
 * prettify_string() and fill_exponent() functions use code taken from
 * Florian Loitsch's original Grisu algorithms implementation
 * (http://florian.loitsch.com/publications/bench.tar.gz)
 * and "Printing Floating-Point Numbers Quickly and Accurately with
 * Integers" paper
 * (http://florian.loitsch.com/publications/dtoa-pldi2010.pdf)
 */

#ifndef FLOAXIE_PRETTIFY_H
#define FLOAXIE_PRETTIFY_H

#include <cstring>
#include <cassert>
#include <cstdlib>
#include <algorithm>

#include <floaxie/static_pow.h>
#include <floaxie/print.h>

namespace floaxie
{
	enum format {
		decimal,
		scientific
	};

	constexpr const char digits_lut[200] = {
		'0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
		'1', '0', '1', '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9',
		'2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
		'3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9',
		'4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4', '7', '4', '8', '4', '9',
		'5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
		'6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
		'7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7', '7', '8', '7', '9',
		'8', '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
		'9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'
	};

	template<std::size_t threshold> inline format choose_format(const std::size_t field_width) noexcept
	{
		static_assert(threshold > static_pow<10, 1>(), "Only 10 ⩽ |threshold| ⩽ 100 is supported");

		return field_width > threshold ? format::scientific : format::decimal;
	}

	inline void fill_exponent(int K, char* buffer) noexcept
	{
		const unsigned int hundreds = K / 100;
		K %= 100;
		buffer[0] = '0' + hundreds;
		buffer += (hundreds > 0);

		const char* d = digits_lut + K * 2;
		buffer[0] = d[0];
		buffer[1] = d[1];

		buffer[2] = '\0';
	}

	inline void print_scientific(char* buffer, const unsigned int len, const int dot_pos) noexcept
	{
		const int K = dot_pos - 1;
		if (len > 1)
		{
			/* leave the first digit. then add a '.' and at the end 'e...' */
			std::memmove(buffer + 2, buffer + 1, len - 1);
			buffer[1] = '.';
			buffer += len;
		}

		/* add 'e...' */
		buffer[1] = 'e';
		buffer[2] = '-';
		buffer += K < 0;

		fill_exponent(std::abs(K), buffer + 2);
	}

	inline void print_decimal(char* buffer, const unsigned int len, const int k) noexcept
	{
		const int dot_pos = len + k;

		const unsigned int actual_dot_pos = dot_pos > 0 ? dot_pos : 1;

		// sorry for obfuscating, but that branchless one works:
		const unsigned int left_offset = (2 + std::abs(dot_pos)) * (dot_pos <= 0); // ≡ dot_pos > 0 ? 0 : 2 - dot_pos
		const unsigned int right_offset = positive_part(k);

		const unsigned int left_shift_src = positive_part(dot_pos);
		const unsigned int left_shift_dest = dot_pos > 0 ? left_shift_src + (k < 0) : left_offset;
		const unsigned int left_shift_len = positive_part(static_cast<int>(len) - static_cast<int>(left_shift_src));

		const unsigned int term_pos = len + right_offset + (left_shift_dest - left_shift_src);

		std::memmove(buffer + left_shift_dest, buffer + left_shift_src, left_shift_len);
		std::memset(buffer, '0', left_offset);
		std::memset(buffer + len, '0', right_offset);
		buffer[actual_dot_pos] = '.';
		buffer[term_pos] = '\0';
	}

	template<std::size_t decimal_scientific_threshold>
	inline void prettify(char* buffer, const unsigned int len, const int k) noexcept
	{
		/* v = buffer * 10 ^ k
			dot_pos is such that 10 ^ (dot_pos - 1) <= v < 10 ^ dot_pos
			this way dot_pos gives the position of the comma.
		*/
		const int dot_pos = len + k;

		// is always positive, since dot_pos is negative only when k is negative
		const std::size_t field_width = std::max(dot_pos, -k);

		switch (choose_format<decimal_scientific_threshold>(field_width))
		{
		case format::decimal:
			print_decimal(buffer, len, k);
			break;

		case format::scientific:
			print_scientific(buffer, len, dot_pos);
			break;
		}
	}
}

#endif // FLOAXIE_PRETTIFY_H
