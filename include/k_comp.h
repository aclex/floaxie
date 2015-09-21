#ifndef K_COMP_H
#define K_COMP_H

#include <cmath>

// #define D_1_LOG2_10 0.30102999566398114 // 1 / lg(10)

inline int k_comp(int e, int alpha, int gamma)
{
	constexpr static auto q(64);
	constexpr static double one_div_log2_10(0.30102999566398114); // 1 / lg(10)

	return std::ceil((alpha - e + q - 1) * one_div_log2_10);
}

#endif // K_COMP_H
