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
 * diy_fp class and helper functions use code and influenced by
 * Florian Loitsch's original Grisu algorithms implementation
 * (http://florian.loitsch.com/publications/bench.tar.gz)
 * and "Printing Floating-Point Numbers Quickly and Accurately with
 * Integers" paper
 * (http://florian.loitsch.com/publications/dtoa-pldi2010.pdf)
 */

#ifndef FLOAXIE_BASIC_DIY_FP_H
#define FLOAXIE_BASIC_DIY_FP_H

namespace floaxie
{
	template<typename MantissaType, typename ExponentType> class basic_diy_fp
	{
	public:
		typedef MantissaType mantissa_storage_type;
		typedef ExponentType exponent_storage_type;

		basic_diy_fp() = default;
		basic_diy_fp(const basic_diy_fp&) = default;
		constexpr basic_diy_fp(MantissaType mantissa, ExponentType exponent) : m_f(mantissa), m_e(exponent) { }
		constexpr mantissa_storage_type mantissa() const { return m_f; }
		constexpr exponent_storage_type exponent() const { return m_e; }

		constexpr explicit operator bool() const noexcept
		{
			return m_f && m_e;
		}

	protected:
		mantissa_storage_type m_f;
		exponent_storage_type m_e;
	};
}

#endif // FLOAXIE_BASIC_DIY_FP_H
