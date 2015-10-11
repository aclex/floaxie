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

#include <cstdint>

#include <floaxie/diy_fp.h>
#include <floaxie/cached_power.h>
#include <floaxie/k_comp.h>

namespace floaxie
{
	template<typename NumericType> constexpr NumericType static_pow(NumericType base, NumericType pow) noexcept
	{
		return pow == 0 ? 1 : base * static_pow(base, pow - 1);
	}

	template<typename NumericType> constexpr NumericType static_pow10(NumericType pow)
	{
		return static_pow(static_cast<NumericType>(10), pow);
	}

	constexpr std::pair<int, std::uint32_t> make_kappa_div(std::uint32_t pow)
	{
		return std::pair<int, std::uint32_t>(pow, static_pow10(pow > 0 ? pow - 1 : 0));
	}

	inline std::pair<int, std::uint32_t> calculate_kappa_div(std::uint32_t n)
	{
		if (n < static_pow10(4))
		{
			if (n < static_pow10(2))
			{
				if (n < static_pow10(1))
					return make_kappa_div(1);
				else
					return make_kappa_div(2);
			}
			else
			{
				if (n < static_pow10(3))
					return make_kappa_div(3);
				else
					return make_kappa_div(4);
			}
		}
		else
		{
			if (n < static_pow10(6))
			{
				if (n < static_pow10(5))
					return make_kappa_div(5);
				else
					return make_kappa_div(6);
			}
			else
			{
				if (n < static_pow10(8))
				{
					if (n < static_pow10(7))
						return make_kappa_div(7);
					else
						return make_kappa_div(8);
				}
				else
				{
					if (n < static_pow10(9))
						return make_kappa_div(9);
					else
						return make_kappa_div(10);
				}
			}
		}
	}

	template<int alpha, int gamma> void digit_gen(const diy_fp& Mp, const diy_fp& delta, char* buffer, int* len, int* K);

	template<> void digit_gen<-35, -32>(const diy_fp& Mp, const diy_fp& delta, char* buffer, int* len, int* K)
	{
		assert(Mp.exponent() <= 0);

		const diy_fp one(0x1ul << -Mp.exponent(), Mp.exponent());

		std::uint32_t p1 = Mp.mantissa() >> -one.exponent();
		std::uint64_t p2 = Mp.mantissa() & (one.mantissa() - 1);

		assert(p1 || p2);

		*len = 0;

		auto delta_f = delta.mantissa();

		if (p1)
		{
			const bool close_to_delta = p2 <= delta_f;

			auto&& kappa_div(calculate_kappa_div(p1));

			int& kappa(kappa_div.first);
			std::uint32_t& div(kappa_div.second);

			while (kappa > 0)
			{
				buffer[(*len)++] = '0' + p1 / div;

				p1 %= div;

				kappa--;
				div /= 10;

				if (close_to_delta && !p1)
				{
					*K += kappa;
					return;
				}
			}
		}

		if (p2)
		{
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
	}

	template<int alpha, int gamma, typename FloatType> void grisu(FloatType v, char* buffer, int* length, int* K)
	{
		std::pair<diy_fp, diy_fp>&& w(diy_fp::boundaries(v));
		diy_fp &w_m(w.first), &w_p(w.second);

		const int mk = k_comp_exp<alpha, gamma>(w_p.exponent());
		const diy_fp& c_mk(cached_power(mk));

		w_m *= c_mk;
		w_p *= c_mk;

		++w_m;
		--w_p;

		const diy_fp& delta(w_p - w_m);

		*K = -mk;

		digit_gen<alpha, gamma>(w_p, delta, buffer, length, K);
	}
}

#endif // FLOAXIE_GRISU_H
