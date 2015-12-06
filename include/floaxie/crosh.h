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

#include <iostream>
#include <bitset>

namespace floaxie
{
	constexpr std::size_t decimal_q(diy_fp::bit_size<diy_fp::mantissa_storage_type>() * lg_2);

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

	inline diy_fp::mantissa_storage_type pow10(std::size_t pow)
	{
		diy_fp::mantissa_storage_type ret(1);
		for (std::size_t i = 0; i < pow; ++i)
			ret *= 10;

		return ret;
	}

	inline std::size_t digit_decomp(diy_fp& w, const char* buffer, std::size_t len)
	{
		diy_fp::mantissa_storage_type f(0);
		diy_fp::exponent_storage_type e(0);

		// read up to kappa decimal digits to mantissa
		const std::size_t kappa(std::min(decimal_q, len));

		std::cout << "kappa: " << kappa << std::endl;
		std::cout << "1000 == " << pow10(3) << std::endl;

		for (std::size_t curr_char = 0; curr_char < kappa; ++curr_char)
		{
			const char digit = buffer[curr_char] - '0';
			f += digit * pow10(kappa - curr_char - 1);
// 			std::cout << "digit: " << char(digit + '0') << ", curr_char: " << curr_char << ", pow: " << kappa - curr_char - 1 << std::endl;
// 			std::cout << "mantissa binary: " << std::bitset<64>(f) << std::endl;
// 			std::cout << "f: " << f << std::endl;
		}

		std::cout << "before normalization, mantissa binary: " << std::bitset<64>(f) << std::endl;

		// normalize interim value
		constexpr diy_fp::mantissa_storage_type msb(0x1ul << (diy_fp::bit_size<diy_fp::mantissa_storage_type>() - 1));
		while (!(f & msb))
		{
			f <<= 1;
			--e;
		}

		std::cout << "after normalization, mantissa binary: " << std::bitset<64>(f) << std::endl;
		std::cout << "after normalization, f: " << f <<", e: "<< e << std::endl;

		if (kappa < len)
		{
			assert(e >= -4);
			const std::size_t lsb_pow(5 + e);

			// save some more bits restoring prefix of fraction
			const unsigned char frac(extract_simple_fraction_4(buffer + kappa, len - kappa));

			std::cout << "acc: " << std::bitset<8>(frac >> lsb_pow).to_string().substr(8 + e) << std::endl;
			f |= frac >> lsb_pow;
			std::cout << "after restore, mantissa binary: " << std::bitset<64>(f) << std::endl;

			// round correctly avoiding integer overflow, undefined behaviour, pain and suffering
			const bool should_round_up(round_up(frac, lsb_pow));
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
			std::cout << "after roundup, mantissa binary: " << std::bitset<64>(f) << std::endl;
		}

		w = diy_fp(f, e);

		return kappa;
	}

	template<typename FloatType> FloatType crosh(const char* buffer, int len, int K)
	{
		std::cout << "buffer: " << buffer << std::endl;
		std::cout << "len: " << len << std::endl;
		std::cout << "initial K: " << K << std::endl;
		diy_fp w;
		const int kappa(digit_decomp(w, buffer, len));
		K += len - kappa;
// 		K = -18;
		std::cout << "K: " << K << std::endl;

		if (K)
		{
			const diy_fp& c_mk(cached_power(K));
			std::cout << "c_mk: " << c_mk << std::endl;
			std::cout << "wp: " << w << std::endl;
// 			w *= c_mk;
			w.precise_multiply2(c_mk);
			std::cout << "wr: " << w << std::endl;
// 			w.normalize();
		}

		return static_cast<FloatType>(w);
	}
}

#endif // FLOAXIE_CROSH_H
