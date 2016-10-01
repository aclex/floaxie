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

#include <vector>
#include <cstddef>
#include <cassert>

#include <floaxie/diy_fp.h>
#include <floaxie/static_pow.h>
#include <floaxie/k_comp.h>
#include <floaxie/cached_power.h>
#include <floaxie/bit_ops.h>
#include <floaxie/fraction.h>

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
			result += (*rit) * seq_pow<diy_fp::mantissa_storage_type, 10, frac_width>(pow++);

		return result;
	}

	struct digit_parse_result
	{
		digit_parse_result() : value(0), K(0), sign(true), frac(0) { }
		diy_fp::mantissa_storage_type value;
		int K;
		const char* str_end;
		bool sign;
		unsigned char frac;
	};

	template<std::size_t kappa, bool no_dot, bool calc_frac> inline digit_parse_result parse_digits(const char* str)
	{
		digit_parse_result ret;

		std::vector<unsigned char> parsed_digits;
		parsed_digits.reserve(kappa);

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
						ret.frac = convert_numerator<fraction_decimal_digits, fraction_binary_digits>(tail);

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
					ret.sign = static_cast<bool>('-' - c); // '+' => true, '-' => false
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

		std::size_t pow(0);
		for (auto rit = parsed_digits.rbegin(); rit != parsed_digits.rend(); ++rit)
			ret.value += (*rit) * seq_pow<diy_fp::mantissa_storage_type, 10, decimal_q>(pow++);

		ret.str_end = str + (pos - 1);
		ret.K = pow_gain - fraction_digits_count;

		return ret;
	}

	struct mantissa_parse_result
	{
		diy_fp value;
		int K;
		const char* str_end;
		bool sign;
	};

	inline mantissa_parse_result parse_mantissa(const char* str)
	{
		mantissa_parse_result ret;

		const auto& digits_parts(parse_digits<decimal_q, false, true>(str));

		ret.value = diy_fp(digits_parts.value, 0);

		auto& w(ret.value);
		w.normalize();

		ret.K = digits_parts.K;
		ret.str_end = digits_parts.str_end;
		ret.sign = digits_parts.sign;

		// extract additional binary digits and round up gently
		if (digits_parts.frac)
		{
			assert(w.exponent() >= -4);
			const std::size_t lsb_pow(4 + w.exponent());

			diy_fp::mantissa_storage_type f(w.mantissa());
			f |= digits_parts.frac >> lsb_pow;

			w = diy_fp(f, w.exponent());

			// round correctly avoiding integer overflow, undefined behaviour, pain and suffering
			if (round_up(digits_parts.frac, lsb_pow).value)
			{
				++w;
			}
		}

		return ret;
	}

	struct exponent_parse_result
	{
		int value;
		const char* str_end;
	};

	inline exponent_parse_result parse_exponent(const char* str)
	{
		exponent_parse_result ret;
		if (*str != 'e' && *str != 'E')
		{
			ret.value = 0;
			ret.str_end = str;
		}
		else
		{
			++str;

			const auto& digit_parts(parse_digits<exp_width, false, false>(str));

			ret.value = digit_parts.value * seq_pow<int, 10, exp_width>(digit_parts.K);

			if (!digit_parts.sign)
				ret.value = -ret.value;

			ret.str_end = digit_parts.str_end;
		}

		return ret;
	}

	template<typename FloatType> struct krosh_result
	{
		FloatType value;
		const char* str_end;
		bool is_accurate;
	};

	template<typename FloatType> krosh_result<FloatType> krosh(const char* str)
	{
		krosh_result<FloatType> ret;

		auto mp(parse_mantissa(str));
		diy_fp& w(mp.value);

		const auto& ep(parse_exponent(mp.str_end));

		mp.K += ep.value;

		if (mp.K)
			w *= cached_power(mp.K);

		w.normalize();
		const auto& v(w.downsample<FloatType>());
		ret.value = v.value;
		ret.str_end = ep.str_end;
		ret.is_accurate = v.is_accurate;

		if (!mp.sign)
			ret.value = -ret.value;

		return ret;
	}
}

#endif // FLOAXIE_CROSH_H
