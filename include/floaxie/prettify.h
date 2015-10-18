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
#include <cstddef>
#include <utility>

namespace
{
	template<unsigned int base, unsigned int pow> struct static_pow_helper
	{
		static const unsigned int value = base * static_pow_helper<base, pow - 1>::value;
	};

	template<unsigned int base> struct static_pow_helper<base, 0>
	{
		static const unsigned int value = 1;
	};

	template<unsigned int base, unsigned int pow> constexpr unsigned long static_pow()
	{
		static_assert(base > 0, "Base should be positive");
		return static_pow_helper<base, pow>::value;
	}
}

namespace floaxie
{
	enum format_range {
		lower_scientific_100,
		lower_scientific_10,
		lower_normal,
		higher_normal,
		higher_normal_simplified,
		higher_scientific_10,
		higher_scientific_100
	};

	template<format_range range> inline void fill_exponent(int K, char* const buffer) noexcept;

	template<> inline void fill_exponent<higher_scientific_100>(int K, char* const buffer) noexcept
	{
		buffer[0] = '0' + K / 100; K %= 100;
		buffer[1] = '0' + K / 10; K %= 10;
		buffer[2] = '0' + K;
		buffer[3] = '\0';
	}

	template<> inline void fill_exponent<higher_scientific_10>(int K, char* const buffer) noexcept
	{
		buffer[0] = '0' + K / 10; K %= 10;
		buffer[1] = '0' + K;
		buffer[2] = '\0';
	}

	template<> inline void fill_exponent<lower_scientific_100>(int K, char* const buffer) noexcept
	{
		*buffer = '-';
		fill_exponent<higher_scientific_100>(-K, buffer + 1);
	}

	template<> inline void fill_exponent<lower_scientific_10>(int K, char* const buffer) noexcept
	{
		*buffer = '-';
		fill_exponent<higher_scientific_10>(-K, buffer + 1);
	}

	template<format_range range> inline void print_scientific(int dot_pos, unsigned int len, char* buffer) noexcept
	{
		if (len > 1)
		{
			/* leave the first digit. then add a '.' and at the end 'e...' */
			std::memmove(buffer + 2, buffer + 1, len - 1);
			buffer[1] = '.';
			buffer += len;
		}

		/* add 'e...' */
		buffer[1] = 'e';
		fill_exponent<range>(dot_pos - 1, buffer + 2);
	}

	template<unsigned int threshold> inline format_range detect_range(const int dot_pos, const unsigned int len) noexcept
	{
		static_assert(threshold > static_pow<10, 1>(), "Only 10 ⩽ |threshold| ⩽ 100 is supported");

		if (dot_pos > 0)
		{
			if (static_cast<unsigned int>(dot_pos) < len) return format_range::higher_normal;
			if (static_cast<unsigned int>(dot_pos) < threshold) return format_range::higher_normal_simplified;
			if (static_cast<unsigned int>(dot_pos) < static_pow<10, 2>()) return format_range::higher_scientific_10;
			return format_range::higher_scientific_100;
		}
		else
		{
			if (dot_pos > -static_cast<int>(threshold)) return format_range::lower_normal;
			if (dot_pos > -static_cast<int>(static_pow<10, 2>())) return format_range::lower_scientific_10;
			return format_range::lower_scientific_100;
		}
	}

	inline void prettify_string(char* buffer, int len, int k) noexcept
	{
		/* v = buffer * 10^k
			kk is such that 10^(kk-1) <= v < 10^kk
			this way kk gives the position of the comma.
		*/
		const int dot_pos = len + k;

		switch (detect_range<12>(dot_pos, len))
		{
		case format_range::higher_normal_simplified:
			/* the first digits are already in. Add some 0s and call it a day. */
			/* the 21 is a personal choice. Only 16 digits could possibly be relevant.
				* Basically we want to print 12340000000 rather than 1234.0e7 or 1.234e10 */
			std::memset(buffer + len, '0', dot_pos - len + 2);
			buffer[dot_pos] = '.';
			buffer[dot_pos + 2] = '\0';
			break;

		case format_range::higher_normal:
			/* comma number. Just insert a '.' at the correct location. */
			std::memmove(buffer + dot_pos + 1, buffer + dot_pos, len - dot_pos);
			buffer[dot_pos] = '.';
			buffer[len + 1] = '\0';
			break;

		case format_range::lower_normal:
		{
			/* something like 0.000abcde.
				* add '0.' and some '0's */
			const int offset = 2 - dot_pos;
			std::memmove(buffer + offset, buffer, len);
			std::memset(buffer, '0', offset);
			buffer[1] = '.';
			buffer[len + offset] = '\0';
			break;
		}

		case format_range::higher_scientific_10:
			print_scientific<format_range::higher_scientific_10>(dot_pos, len, buffer);
			break;
		case format_range::higher_scientific_100:
			print_scientific<format_range::higher_scientific_100>(dot_pos, len, buffer);
			break;
		case format_range::lower_scientific_10:
			print_scientific<format_range::lower_scientific_10>(dot_pos, len, buffer);
			break;
		case format_range::lower_scientific_100:
			print_scientific<format_range::lower_scientific_100>(dot_pos, len, buffer);
			break;
		}
	}
}

#endif // FLOAXIE_PRETTIFY_H
