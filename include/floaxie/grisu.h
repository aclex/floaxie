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
 * digin_gen(), grisu() functions and kappa precalculation approach
 * were greatly influenced by
 * Florian Loitsch's original Grisu algorithms implementation
 * (http://florian.loitsch.com/publications/bench.tar.gz),
 * "Printing Floating-Point Numbers Quickly and Accurately with
 * Integers" paper
 * (http://florian.loitsch.com/publications/dtoa-pldi2010.pdf)
 * and Milo Yip's Grisu implementation
 * (https://github.com/miloyip/dtoa-benchmark)
 */

#ifndef FLOAXIE_GRISU_H
#define FLOAXIE_GRISU_H

#include <utility>
#include <cstdint>
#include <cstring>
#include <cassert>

#include <floaxie/diy_fp.h>
#include <floaxie/cached_power.h>
#include <floaxie/k_comp.h>
#include <floaxie/static_pow.h>

namespace floaxie
{
	template<unsigned int pow> constexpr std::pair<int, std::uint32_t> make_kappa_div()
	{
		return std::pair<int, std::uint32_t>(pow, static_pow<10, pow - 1>());
	}

	template<> constexpr std::pair<int, std::uint32_t> make_kappa_div<0>()
	{
		return std::pair<int, std::uint32_t>(0, 1);
	}

	inline std::pair<int, std::uint32_t> calculate_kappa_div(std::uint32_t n) noexcept
	{
		if (n < static_pow<10, 1>()) return make_kappa_div<1>();
		if (n < static_pow<10, 2>()) return make_kappa_div<2>();
		if (n < static_pow<10, 3>()) return make_kappa_div<3>();
		if (n < static_pow<10, 4>()) return make_kappa_div<4>();
		if (n < static_pow<10, 5>()) return make_kappa_div<5>();
		if (n < static_pow<10, 6>()) return make_kappa_div<6>();
		if (n < static_pow<10, 7>()) return make_kappa_div<7>();
		if (n < static_pow<10, 8>()) return make_kappa_div<8>();
		if (n < static_pow<10, 9>()) return make_kappa_div<9>();
		return make_kappa_div<10>();
	}

	template<bool positive_exponent> inline void digit_gen(const diy_fp& Mp, const diy_fp& Mm, char* buffer, int* len, int* K) noexcept;

	template<> inline void digit_gen<false>(const diy_fp& Mp, const diy_fp& Mm, char* buffer, int* len, int* K) noexcept
	{
		assert(Mp.exponent() <= 0);

		const diy_fp& delta(Mp - Mm);

		const diy_fp one(0x1ul << -Mp.exponent(), Mp.exponent());

		std::uint32_t p1 = Mp.mantissa() >> -one.exponent();
		std::uint64_t p2 = Mp.mantissa() & (one.mantissa() - 1);

		assert(p1 || p2);

		*len = 0;

		auto delta_f = delta.mantissa();

		const bool close_to_delta = p2 <= delta_f;

		if (p1)
		{
			auto&& kappa_div(calculate_kappa_div(p1));

			int& kappa(kappa_div.first);
			std::uint32_t& div(kappa_div.second);

			while (kappa > 0 && p1)
			{
				buffer[(*len)++] = '0' + p1 / div;

				p1 %= div;

				kappa--;
				div /= 10;
			}

			if (close_to_delta)
			{
				*K += kappa;
				return;
			}
			else
			{
				std::memset(buffer + (*len), '0', kappa);
				(*len) += kappa;
				kappa = 0;
			}
		}

		const bool some_already_written = (*len) > 0;
		int kappa = 0;

		while (p2 > delta_f)
		{
			p2 *= 10;

			const int d = p2 >> -one.exponent();

			if (some_already_written || d)
				buffer[(*len)++] = '0' + d;

			p2 &= one.mantissa() - 1;

			kappa--;
			delta_f *= 10;
		}

		*K += kappa;
	}

	template<int alpha, int gamma> inline void digit_gen(const diy_fp& Mp, const diy_fp& Mm, char* buffer, int* len, int* K) noexcept
	{
		constexpr bool exponent_is_positive = alpha > 0 && gamma > 0;
		digit_gen<exponent_is_positive>(Mp, Mm, buffer, len, K);
	}

	template<int alpha, int gamma, typename FloatType> inline void grisu2(FloatType v, char* buffer, int* length, int* K) noexcept
	{
		static_assert(alpha <= gamma - 3,
			"It's imposed that γ ⩾ α + 3, since otherwise it's not always possible to find a proper decimal cached power");

		std::pair<diy_fp, diy_fp>&& w(diy_fp::boundaries(v));
		diy_fp &w_m(w.first), &w_p(w.second);

		const int mk = k_comp_exp<alpha, gamma>(w_p.exponent());
		const diy_fp& c_mk(cached_power(mk));

		w_m *= c_mk;
		w_p *= c_mk;

		++w_m;
		--w_p;

		*K = -mk;

		digit_gen<alpha, gamma>(w_p, w_m, buffer, length, K);
	}
}

#endif // FLOAXIE_GRISU_H
