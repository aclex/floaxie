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

#ifndef FLOAXIE_CROSH_H
#define FLOAXIE_CROSH_H

#include <cmath>
#include <cstring>
#include <cassert>

#include <floaxie/diy_fp.h>
#include <floaxie/static_pow.h>
#include <floaxie/k_comp.h>
#include <floaxie/cached_power.h>
#include <floaxie/bit_ops.h>
#include <floaxie/pow10.h>

#include <iostream>
#include <bitset>

namespace floaxie
{
	constexpr std::size_t decimal_q(bit_size<diy_fp::mantissa_storage_type>() * lg_2);

	inline char calculate_fraction_4(int numerator)
	{
		if (numerator >= 5000)
		{
			if (numerator >= 7500)
			{
				if (numerator >= 8750)
				{
					if (numerator >= 9375)
						return 0x0f;
					else
						return 0x0e;
				}
				else
				{
					if (numerator >= 8125)
						return 0x0d;
					else
						return 0x0c;
				}
			}
			else
			{
				if (numerator >= 6250)
				{
					if (numerator >= 6875)
						return 0x0b;
					else
						return 0x0a;
				}
				else
				{
					if (numerator >= 5625)
						return 0x09;
					else
						return 0x08;
				}
			}
		}
		else
		{
			if (numerator >= 2500)
			{
				if (numerator >= 3750)
				{
					if (numerator >= 4325)
						return 0x07;
					else
						return 0x06;
				}
				else
				{
					if (numerator >= 3125)
						return 0x05;
					else
						return 0x04;
				}
			}
			else
			{
				if (numerator >= 1250)
				{
					if (numerator >= 1875)
						return 0x03;
					else
						return 0x02;
				}
				else
				{
					if (numerator >= 625)
						return 0x01;
					else
						return 0x00;
				}
			}
		}
	}

	inline bool detect_round_up_bit(int numerator)
	{
		switch (numerator)
		{
			case 9375:
			case 8750:
			case 8125:
			case 7500:
			case 6875:
			case 6250:
			case 5625:
			case 5000:
			case 4325:
			case 3750:
			case 3125:
			case 2500:
			case 1875:
			case 1250:
			case 625:
			case 0:
				return false;
			default:
				return true;
		}
	}

	inline unsigned char extract_simple_fraction_4(const char* buffer, int len)
	{
		const unsigned int num = (len > 0 ? buffer[0] - '0' : 0) * static_pow<10, 3>() +
				(len > 1 ? buffer[1] - '0' : 0) * static_pow<10, 2>() +
				(len > 2 ? buffer[2] - '0' : 0) * static_pow<10, 1>() +
				(len > 3 ? buffer[3] - '0' : 0);

		std::cout << "num: " << num << std::endl;

		unsigned char frac = calculate_fraction_4(num);
// 		const bool round_up = (frac & 0x1ul) && (len > 4 || detect_round_up_bit(num));
		const bool round_up = (len > 4 || detect_round_up_bit(num));
		std::cout << "round_up: " << round_up << std::endl;
		frac <<= 1;
		frac |= round_up;
		auto frac_str(std::bitset<8>(frac).to_string().substr(3));
		std::cout << "frac binary: " << frac_str << std::endl;

		return frac;
	}

	inline std::size_t digit_decomp(diy_fp& w, const char* buffer, std::size_t len)
	{
		diy_fp::mantissa_storage_type f(0);
		diy_fp::exponent_storage_type e(0);

		// read up to kappa decimal digits to mantissa
		const std::size_t kappa(std::min(decimal_q, len));

		for (std::size_t curr_char = 0; curr_char < kappa; ++curr_char)
		{
			const char digit = buffer[curr_char] - '0';
			f += digit * pow10<diy_fp::mantissa_storage_type, decimal_q>(kappa - curr_char - 1);
		}

		// normalize interim value
		while (!highest_bit(f))
		{
			f <<= 1;
			--e;
		}

		if (kappa < len)
		{
			assert(e >= -4);
			const std::size_t lsb_pow(5 + e);

			// save some more bits restoring prefix of fraction
			const unsigned char frac(extract_simple_fraction_4(buffer + kappa, len - kappa));

			f |= frac >> lsb_pow;

			// round correctly avoiding integer overflow, undefined behaviour, pain and suffering
			bool accurate;
			const bool should_round_up(round_up(frac, lsb_pow, &accurate));
			if (should_round_up)
			{
				if (f < std::numeric_limits<diy_fp::mantissa_storage_type>::max())
				{
					++f;
				}
				else
				{
					f >>= 1;
					++f;
					++e;
				}
			}
		}

		w = diy_fp(f, e);

		return kappa;
	}

	template<typename FloatType> FloatType crosh(const char* buffer, int len, int K, bool* accurate)
	{
		diy_fp D;
		const int kappa(digit_decomp(D, buffer, len));
		K += len - kappa;

		diy_fp w = D;
		if (K)
			w *= cached_power(K);

		w.normalize();

		return w.downsample<FloatType>(accurate);
	}
}

#endif // FLOAXIE_CROSH_H
