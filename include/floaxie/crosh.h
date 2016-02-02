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
		std::cout << "before normalization, f: " << f <<", e: "<< e << std::endl;

		// normalize interim value
		while (!highest_bit(f))
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
			std::cout << "after restore, f: " << f <<", e: "<< e << std::endl;

			// round correctly avoiding integer overflow, undefined behaviour, pain and suffering
			const bool should_round_up(round_up(frac, lsb_pow));
			if (should_round_up)
			{
				const auto n_f = f;
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

				// don't round, when it leads to consecutive rounding (which is apparently wrong)
				auto h_f = f ^ n_f;
				auto l = suffix_length(h_f, 1);
				std::cout << "l: " << l << std::endl;
				auto c_f = f | (0x1ul << (l - 1));
				std::cout << "original f:    " << print_binary(n_f) << std::endl;
				std::cout << "rounded f:     " << print_binary(f) << std::endl;
				std::cout << "character f:   " << print_binary(h_f) << std::endl;
				std::cout << "constructed f: " << print_binary(c_f) << std::endl;
				if (l && round_up(c_f, l))
				{
					std::cout << "cancel rounding..." << std::endl;
					f = n_f;
				}
			}
			std::cout << "after roundup, mantissa binary: " << std::bitset<64>(f) << std::endl;
			std::cout << "after roundup, f: " << f <<", e: "<< e << std::endl;
		}

		w = diy_fp(f, e);

		return kappa;
	}

	inline diy_fp::mantissa_storage_type probe_w(const diy_fp& w, const diy_fp& c_rmk, const diy_fp& D) noexcept
	{
		diy_fp rh, rl;
		precise_multiply4(w, c_rmk, rh, rl);

		if (highest_bit(rl.mantissa()))
		{
			++rh;
		}

		rh.normalize();
		assert(rh.exponent() == D.exponent());
		std::cout << "r: " << rh << std::endl;
		std::cout << "D: " << D << std::endl;
		return abs_diff(rh.mantissa(), D.mantissa());
	}

	inline diy_fp precise_round(const diy_fp& rh, const diy_fp& rl, const diy_fp& D, const diy_fp& c_rmk) noexcept
	{
		diy_fp w(rh);
		if (highest_bit(rl.mantissa()))
			++w;

		diy_fp wp(w), wm(w);
		++wp;
		--wm;

		diy_fp W[3];
		W[0] = wm;
		W[1] = w;
		W[2] = wp;

		diy_fp::mantissa_storage_type diff[3];
		std::cout << "Dm..." << std::endl;
		diff[0] = probe_w(wm, c_rmk, D);
		std::cout << "D..." << std::endl;
		diff[1] = probe_w(w, c_rmk, D);
		std::cout << "Dp..." << std::endl;
		diff[2] = probe_w(wp, c_rmk, D);

		std::cout << "*** Dm = " << diff[0] << ", D = " << diff[1] <<", Dp = " << diff[2] << std::endl;
		diy_fp::mantissa_storage_type min_d = diff[0];
		size_t min_idx = 0;

		if (diff[1] <= min_d)
		{
			min_d = diff[1];
			min_idx = 1;
		}

		if (diff[2] <= min_d)
			min_idx = 2;

		std::cout << "vote for index: " << min_idx << std::endl;
		return W[min_idx];

// 		if (rl.mantissa() == msb_value<diy_fp::mantissa_storage_type>())
// 		{
// 			std::cout << "probing: w, wp, wm" << std::endl;
// 			if (probe_w(w, c_rmk, D))
// 			{
// 				std::cout << "vote for w" << std::endl;
// 				return w;
// 			}
// 			else if (probe_w(wp, c_rmk, D))
// 			{
// 				std::cout << "vote for wp" << std::endl;
// 				return wp;
// 			}
// 			else
// 			{
// 				probe_w(wm, c_rmk, D); // FIXME
// 				std::cout << "vote for wm" << std::endl;
// 				return wm;
// 			}
// 		}
// 		else if (rl.mantissa() < msb_value<diy_fp::mantissa_storage_type>())
// 		{
// 			std::cout << "probing: wm, w, wp" << std::endl;
// 			if (probe_w(wm, c_rmk, D))
// 			{
// 				std::cout << "vote for wm" << std::endl;
// 				return wm;
// 			}
// 			else if (probe_w(w, c_rmk, D))
// 			{
// 				std::cout << "vote for w" << std::endl;
// 				return w;
// 			}
// 			else
// 			{
// 				probe_w(wp, c_rmk, D); // FIXME
// 				std::cout << "vote for wp" << std::endl;
// 				return wp;
// 			}
// 		}
// 		else
// 		{
// 			std::cout << "probing: wp, w, wm" << std::endl;
// 			if (probe_w(wp, c_rmk, D))
// 			{
// 				std::cout << "vote for wp" << std::endl;
// 				return wp;
// 			}
// 			else if (probe_w(w, c_rmk, D))
// 			{
// 				std::cout << "vote for w" << std::endl;
// 				return w;
// 			}
// 			else
// 			{
// 				probe_w(wm, c_rmk, D); // FIXME
// 				std::cout << "vote for wm" << std::endl;
// 				return wm;
// 			}
// 		}
	}

	template<typename FloatType> FloatType crosh(const char* buffer, int len, int K)
	{
		std::cout << "buffer: " << buffer << std::endl;
		std::cout << "len: " << len << std::endl;
		std::cout << "initial K: " << K << std::endl;
		diy_fp D, w;
		const int kappa(digit_decomp(D, buffer, len));
		K += len - kappa;
// 		K = -18;
		std::cout << "K: " << K << std::endl;

		if (K)
		{
			const diy_fp& c_mk(cached_power(K)), c_rmk(cached_power(-K));
			std::cout << "c_mk: " << c_mk << std::endl;
			std::cout << "D: " << D << std::endl;
// 			w *= c_mk;
// 			D.precise_multiply2(c_mk);
			diy_fp rh, rl;
			precise_multiply4(D, c_mk, rh, rl);
			std::cout << "~w: " << rh << std::endl;
			w = precise_round(rh, rl, D, c_rmk);
			std::cout << "w:  " << w << std::endl;
			w.normalize();
			std::cout << "wn: " << w << std::endl;
// 			w.normalize();
		}
		else
		{
			w = D;
		}

		return static_cast<FloatType>(w);
	}
}

#endif // FLOAXIE_CROSH_H
