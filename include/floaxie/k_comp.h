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
 * k_comp() function uses code taken from
 * Florian Loitsch's "Printing Floating-Point Numbers Quickly
 * and Accurately with Integers" paper
 * (http://florian.loitsch.com/publications/dtoa-pldi2010.pdf)
 */

#ifndef FLOAXIE_K_COMP_H
#define FLOAXIE_K_COMP_H

#include <cmath>

namespace floaxie
{
	/* We ignore mantissa component (q) in exponent to eliminate
	 * excessive add and subtract of it during K computation.
	 *
	 * Function name was changed to not confuse it with the original
	 * k_comp() function from reference paper where this component
	 * is considered.
	 */
	template<int alpha, int gamma> inline int k_comp_exp(int e)
	{
		constexpr static double one_div_log2_10(0.30102999566398114); // 1 / lg(10)

		return std::ceil((alpha - e - 1) * one_div_log2_10);
	}
}

#endif // FLOAXIE_K_COMP_H
