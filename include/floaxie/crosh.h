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

#ifndef FLOAXIE_CROSH_H
#define FLOAXIE_CROSH_H

#include <cmath>
#include <cstring>
#include <cassert>

#include <vector>

#include <floaxie/diy_fp.h>
#include <floaxie/static_pow.h>
#include <floaxie/k_comp.h>
#include <floaxie/cached_power.h>
#include <floaxie/bit_ops.h>
#include <floaxie/pow10.h>
#include <floaxie/fraction.h>

#include <iostream>
#include <bitset>

namespace floaxie
{
	constexpr std::size_t decimal_q(bit_size<diy_fp::mantissa_storage_type>() * lg_2);

	constexpr std::size_t fraction_binary_digits(4);
	constexpr std::size_t fraction_decimal_digits(4);

	constexpr std::size_t offset = 2048;

	constexpr std::size_t frac_width(4);
	constexpr std::size_t exp_width(3);

	template<std::size_t kappa>
	inline diy_fp::mantissa_storage_type extract_fraction_digits(const char* str)
	{
		std::array<unsigned char, kappa> parsed_digits;
		parsed_digits.fill(0);

		for (std::size_t pos = 0; pos < kappa; ++pos)
		{
			const char c = str[pos] - '0';
			if (c >= 0 && c <= 9)
				parsed_digits[pos] = c;
			else
				break;
		}

		diy_fp::mantissa_storage_type result(0);
		std::size_t pow(0);
		for (auto rit = parsed_digits.rbegin(); rit != parsed_digits.rend(); ++rit)
			result += (*rit) * pow10<diy_fp::mantissa_storage_type, frac_width>(pow++);

		return result;
	}

	template<std::size_t kappa, bool no_dot, bool calc_frac>
	inline diy_fp::mantissa_storage_type parse_digits(const char* str, const char** str_end, bool* sign, int* K, unsigned char* frac = nullptr)
	{
		std::vector<unsigned char> parsed_digits;
		parsed_digits.reserve(kappa);

		*K = 0;
		*sign = true;

		if (calc_frac)
			*frac = 0;

		bool dot_set(false);
		bool frac_calculated(!calc_frac);
		std::size_t pow_gain(0);
		std::size_t zero_substring_length(0), fraction_digits_count(0);

		bool go_to_beach(false);
		std::size_t pos(0);

		while(!go_to_beach)
		{
			const char c = str[pos];
			switch (c)
			{
			case '0':
				if (!parsed_digits.empty() || dot_set)
				{
					++zero_substring_length;
					pow_gain += !dot_set;
				}
				break;

			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (parsed_digits.size() + zero_substring_length < kappa)
				{
					if (zero_substring_length)
					{
						parsed_digits.insert(parsed_digits.end(), zero_substring_length, 0);
						fraction_digits_count += zero_substring_length - pow_gain;
						zero_substring_length = 0;
						pow_gain = 0;
					}

					parsed_digits.push_back(c - '0');
					fraction_digits_count += dot_set;
				}
				else
				{
					if (!frac_calculated)
					{
						auto tail = extract_fraction_digits<frac_width>(str + pos - zero_substring_length);
						*frac = convert_numerator<fraction_decimal_digits, fraction_binary_digits>(tail);

						frac_calculated = true;

					}
					pow_gain += !dot_set;
				}
				break;

			case '.':
				go_to_beach = dot_set;
				dot_set = true;
				break;

			case '-':
			case '+':
				if (pos == 0)
				{
					*sign = static_cast<bool>('-' - c); // '+' => true, '-' => false
					break;
				}
				// fall down

			default:
				go_to_beach = true;
				break;
			}

			go_to_beach |= pos > offset;

			++pos;
		}

		diy_fp::mantissa_storage_type result(0);
		std::size_t pow(0);
		for (auto rit = parsed_digits.rbegin(); rit != parsed_digits.rend(); ++rit)
			result += (*rit) * pow10<diy_fp::mantissa_storage_type, decimal_q>(pow++);

		*str_end = str + (pos - 1);
		*K = pow_gain - fraction_digits_count;

		return result;
	}

	inline diy_fp parse_mantissa(const char* str, const char** str_end, bool* sign, int* K)
	{
		unsigned char frac(0);
		diy_fp w(parse_digits<decimal_q, false, true>(str, str_end, sign, K, &frac), 0);
		w.normalize();

		// extract additional binary digits and round up gently
		if (frac)
		{
			assert(w.exponent() >= -4);
			const std::size_t lsb_pow(4 + w.exponent());

			diy_fp::mantissa_storage_type f(w.mantissa());
			f |= frac >> lsb_pow;

			w = diy_fp(f, w.exponent());

			// round correctly avoiding integer overflow, undefined behaviour, pain and suffering
			bool accurate;
			const bool should_round_up(round_up(frac, lsb_pow, &accurate));
			if (should_round_up)
			{
				++w;
			}
		}

		return w;
	}

	inline int parse_exponent(const char* str, const char** str_end)
	{
		if (*str != 'e' && *str != 'E')
		{
			*str_end = str;
			return 0;
		}

		++str;

		bool sign;
		int K;
		int value(parse_digits<exp_width, false, false>(str, str_end, &sign, &K));
		value *= pow10<int, exp_width>(K);
		return sign ? value : -value;
	}


	template<typename FloatType> FloatType crosh(const char* str, const char** str_end, bool* sign, bool* accurate)
	{
		int K(0);
		diy_fp w(parse_mantissa(str, str_end, sign, &K));

		K += parse_exponent(*str_end, str_end);;

		if (K)
			w *= cached_power(K);

		w.normalize();

		return w.downsample<FloatType>(accurate);
	}
}

#endif // FLOAXIE_CROSH_H
