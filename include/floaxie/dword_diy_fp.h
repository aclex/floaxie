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

#ifndef FLOAXIE_DWORD_DIY_FP_H
#define FLOAXIE_DWORD_DIY_FP_H

#include <cstdint>

// FIXME
#include <iostream>

#include <floaxie/diy_dword.h>
#include <floaxie/basic_diy_fp.h>
#include <floaxie/bit_ops.h>

namespace floaxie
{
	class dword_diy_fp : public basic_diy_fp<diy_dword<std::uint64_t>, int>
	{
	public:
		using basic_diy_fp<diy_dword<std::uint64_t>, int>::basic_diy_fp;

		constexpr bool is_normalized() const noexcept
		{
			return highest_bit(m_f.higher());
		}

		void normalize() noexcept
		{
			if (!m_f)
				return;

			while (!is_normalized())
			{
				m_f <<= 1;
				--m_e;
			}
		}

		friend dword_diy_fp operator+(dword_diy_fp lhs, dword_diy_fp rhs) noexcept
		{
			std::cout << "lhs: " << "(f = " << lhs.mantissa() << ", e = " << lhs.exponent() << ')' << std::endl;
			std::cout << "rhs: " << "(f = " << rhs.mantissa() << ", e = " << rhs.exponent() << ')' << std::endl;
			std::cout << "lhs: " << static_cast<bool>(lhs) << std::endl;
			std::cout << "!lhs: " << (!lhs) << std::endl;
			if (!lhs)
			{
				std::cout << "return rhs: " << "(f = " << rhs.mantissa() << ", e = " << rhs.exponent() << ')' << std::endl;
				return rhs;
			}

			if (!rhs)
			{
				std::cout << "return lhs: " << "(f = " << lhs.mantissa() << ", e = " << lhs.exponent() << ')' << std::endl;
				return lhs;
			}

			if (lhs.m_e < rhs.m_e)
			{
				using std::swap;
				swap(lhs, rhs);
			}

			constexpr std::size_t mantissa_bit_size(bit_size<mantissa_storage_type>());

			const std::size_t margin_e(lhs.m_e - rhs.m_e);

			std::cout << std::dec;
			std::cout << "lhs.m_e: " << lhs.m_e << ", rhs.m_e: " << rhs.m_e << std::endl;
			std::cout << "mantissa_bit_size: " << mantissa_bit_size << std::endl;
			const std::size_t shift_amount = (mantissa_bit_size - margin_e);
			std::cout << "margin_e: " << margin_e << ", shift amount: " << shift_amount << std::endl;
// 			assert(margin_e < mantissa_bit_size);
			if (margin_e > mantissa_bit_size)
				return lhs;

			mantissa_storage_type shifted_part = margin_e > 0 ? (rhs.m_f >> shift_amount) : rhs.m_f;
			std::cout << "lhs: " << "(f = " << lhs.mantissa() << ", e = " << lhs.exponent() << ')' << std::endl;
			std::cout << "rpl: " << "(f = " << shifted_part << ", e = " << lhs.exponent() << ')' << std::endl;

			bool carry(false);
			std::cout << "check shifted_part: " << print_binary(shifted_part.lower()) << std::endl;
			auto lower = add_with_carry(lhs.m_f.lower(), shifted_part.lower(), carry);
			const bool lower_carry(carry);
			std::cout << "lower: " << print_binary(lower) << ", carry: " << lower_carry << std::endl;
			auto higher = add_with_carry(lhs.m_f.higher(), shifted_part.higher(), carry);
			bool higher_carry(carry);
			std::cout << "1 higher: " << print_binary(higher) << ", carry: " << higher_carry << std::endl;
			higher = add_with_carry(higher, static_cast<dword_diy_fp::mantissa_storage_type::word_type>(lower_carry), carry);
			higher_carry |= carry;
			std::cout << "2 higher: " << print_binary(higher) << ", carry: " << higher_carry << ", additional carry: " << carry << std::endl;

			lhs.m_f = mantissa_storage_type(higher, lower);

			if (higher_carry)
			{
				lhs.m_f >>= 1;
				higher = lhs.m_f.higher() | select_one_bit<bit_size<typename mantissa_storage_type::word_type>() - 1>();
				lower = lhs.m_f.lower();
				lhs.m_f = mantissa_storage_type(higher, lower);
				++lhs.m_e;
			}

// 			std::cout << "result hex: higher: " << std::hex << lhs.m_f.higher() << ", lower: " << std::hex << lhs.m_f.lower() << std::endl;

			std::cout << "result higher binary: " << std::bitset<64>(lhs.m_f.higher()) << std::endl;
			std::cout << "result lower binary:  " << std::bitset<64>(lhs.m_f.lower()) << std::endl;
			std::cout << "result lower decimal: " << std::dec << lhs.m_f.lower() << std::endl;
			std::cout << "type maximum:         " << std::numeric_limits<std::uint64_t>::max() << std::endl;
			std::cout << "lhs exponent:     " << lhs.m_e << std::endl;
			std::cout << std::dec;

			return lhs;
		}

		dword_diy_fp& operator+=(const dword_diy_fp& rhs) noexcept
		{
			dword_diy_fp result(*this + rhs);
			m_f = result.m_f;
			m_e = result.m_e;

			return *this;
		}

		template<typename FloatType> explicit operator FloatType() const noexcept
		{
			static_assert(std::numeric_limits<FloatType>::is_iec559, "Only IEEE-754 floating point types are supported");
			static_assert(bit_size<FloatType>() <= 2 * bit_size<mantissa_storage_type>(), "Too wide floating point type");

			if (m_e >= std::numeric_limits<FloatType>::max_exponent)
			{
				return std::numeric_limits<FloatType>::infinity();
			}

			constexpr std::size_t her_mantissa_size(std::numeric_limits<FloatType>::digits - 1); // remember hidden bit
			constexpr std::size_t my_mantissa_size(bit_size<mantissa_storage_type>());

			std::cout << std::dec;
			std::cout << "my_mantissa_size: " << my_mantissa_size <<", theirs mantissa size: " << her_mantissa_size << std::endl;

			std::cout << "exp: " << m_e << ", min_exponent: " << std::numeric_limits<FloatType>::min_exponent << std::endl;

			if (m_e + int(my_mantissa_size) < std::numeric_limits<FloatType>::min_exponent - int(her_mantissa_size))
			{
				return FloatType(0);
			}

			assert(is_normalized());

			union
			{
				FloatType value;
				mantissa_storage_type::word_type parts;
			};

// 			constexpr mantissa_storage_type mantissa_mask(max_integer_value<FloatType>() >> (my_mantissa_size - her_mantissa_size));
			constexpr exponent_storage_type exponent_bias(std::numeric_limits<FloatType>::max_exponent - 1 + her_mantissa_size);
			constexpr std::size_t lsb_pow(my_mantissa_size - (her_mantissa_size + 1));

// 			const auto f(m_f + 1); // provoke brewing round up by 1 ulp (i.e. lead to round-to-nearest on truncation)
// 			const auto f(m_f.higher());
// 			std::cout << "f: " << print_binary(f) << std::endl;
			std::cout << "m_f: " << m_f << std::endl;
// 			std::cout << "mask: " << print_binary(mantissa_mask) << std::endl;

			const std::size_t denorm_shift(positive_part(std::numeric_limits<FloatType>::min_exponent - int(her_mantissa_size + 1) - m_e));

			std::cout << "denorm_shift: " << denorm_shift << ", lsb_pow: " << lsb_pow << std::endl;
			assert(denorm_shift < my_mantissa_size);

			const std::size_t begin_bit_no(std::max(denorm_shift, lsb_pow)), end_bit_no(begin_bit_no + her_mantissa_size);
			std::cout << "mantissa bits to peek: [" << begin_bit_no << ',' << end_bit_no << ')' << std::endl;
			const exponent_storage_type her_exponent(m_e + begin_bit_no + exponent_bias - (denorm_shift > lsb_pow));
			std::cout << "her exponent: " << print_binary(her_exponent) << std::endl;
			parts = mantissa_storage_type::word_type(her_exponent) << her_mantissa_size;
			const auto p = peek(m_f, begin_bit_no, end_bit_no);
			std::cout << "peeked mantissa: " << p << std::endl;
			const auto r = round_up(m_f, begin_bit_no);
			std::cout << "round up: " << r << std::endl;
			const mantissa_storage_type::word_type her_mantissa(p.lower() + r);
			std::cout << "her mantissa: " << print_binary(her_mantissa) << std::endl;
			parts |= her_mantissa;

			return value;
		}
	};

	template<typename CharType> std::basic_ostream<CharType>& operator<<(std::basic_ostream<CharType>& os, dword_diy_fp v)
	{
		os << "(f = " << v.mantissa() << ", e = " << v.exponent() << ')';
		return os;
	}
}

#endif // FLOAXIE_DWORD_DIY_FP_H
