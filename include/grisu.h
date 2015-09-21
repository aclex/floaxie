#ifndef GRISU_H
#define GRISU_H

#include <cstdint>

#include <iostream>

#include "diy_fp.h"
#include "cached_power.h"
#include "k_comp.h"

void digit_gen(c_diy_fp Mp, c_diy_fp delta, char* buffer, int* len, int* K)
{
	constexpr auto ten9(1000000000);
	uint32_t div; int d,kappa;
	const diy_fp one = { .f = static_cast<std::uint64_t>(1) << -Mp.exponent(), .e = Mp.exponent() };
	uint32_t p1 = Mp.mantissa() >> -one.e; /// Mp_cut
	uint64_t p2 = Mp.mantissa() & (one.f - 1);

	*len = 0; kappa = 10; div = ten9;

	auto delta_f = delta.mantissa();

	while (kappa > 0)
	{ /// Mp_inv1
		d = p1 / div;
		if (d || *len)
			buffer[(*len)++] = '0' + d;

		p1 %= div;
		kappa--;
		div /= 10;

		if ((static_cast<std::uint64_t>(p1) << -one.e) + p2 <= delta_f)
		{ /// Mp_delta
			*K += kappa;
			return;
		}
	}

	do
	{
		p2 *= 10;
		d = p2 >> -one.e;
		if (d || *len)
			buffer[(*len)++] = '0' + d; /// Mp_inv2

		p2 &= one.f - 1;
		kappa--;
		delta_f *= 10;

	}
	while (p2 > delta_f);

	*K += kappa;
}

void grisu(double v, char* buffer, int* length, int* K)
{
	int q = 64, alpha = -35, gamma = -32;
	std::pair<c_diy_fp, c_diy_fp>&& w(c_diy_fp::normalized_boundaries(v));
	c_diy_fp &w_m(w.first), &w_p(w.second);
	int mk = k_comp(w_p.exponent() + q, alpha, gamma);
	c_diy_fp c_mk = cached_power(mk);
	w_m *= c_mk;
	w_p *= c_mk;
	++w_m;
	--w_p;
	c_diy_fp delta = w_p - w_m;
	*K = -mk;
	digit_gen(w_p, delta, buffer, length, K);
}

#endif // GRISU_H
