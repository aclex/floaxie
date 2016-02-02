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

#ifndef FLOAXIE_BIT_OPS_H
#define FLOAXIE_BIT_OPS_H

#include <type_traits>
#include <cstdint>
#include <limits>
#include <cassert>

// FIXME
#include <iostream>
#include <bitset>

namespace floaxie
{
	template<typename NumericType> constexpr std::size_t bit_size() noexcept
	{
		return sizeof(NumericType) * std::numeric_limits<unsigned char>::digits;
	}

	template<typename NumericType> constexpr std::uintmax_t msb_value() noexcept
	{
		return 0x1ul << (bit_size<NumericType>() - 1);
	}

	template<typename NumericType> constexpr std::uintmax_t max_integer_value() noexcept
	{
		return msb_value<NumericType>() + (msb_value<NumericType>() - 1);
	}

	template<std::size_t number> constexpr std::uintmax_t select_one_bit()
	{
		return 0x1ul << number;
	}

	template<typename NumericType> constexpr bool nth_bit(NumericType value, std::size_t n) noexcept
	{
		return value & (std::uintmax_t(1) << n);
	}

	template<typename NumericType> constexpr bool highest_bit(NumericType value) noexcept
	{
		return value & select_one_bit<bit_size<NumericType>() - 1>();
	}

	constexpr std::uintmax_t mask(std::size_t n) noexcept
	{
		return n < bit_size<std::uintmax_t>() ? (std::uintmax_t(1) << n) - 1 : max_integer_value<std::uintmax_t>();
	}

	constexpr std::uintmax_t mask(std::size_t begin_bit_no, std::size_t end_bit_no) noexcept
	{
		return mask(end_bit_no) ^ mask(begin_bit_no);
	}

	template<typename NumericType> constexpr NumericType peek(NumericType value, std::size_t begin_bit_no, std::size_t end_bit_no) noexcept
	{
		return (value & mask(begin_bit_no, end_bit_no)) >> begin_bit_no;
	}

	template<typename NumericType> constexpr std::uintmax_t prefix(NumericType value, std::size_t n) noexcept
	{
		return n != bit_size<NumericType>() ? value >> (bit_size<NumericType>() - n) : value;
	}

	template<typename NumericType> constexpr std::uintmax_t suffix(NumericType value, std::size_t n) noexcept
	{
		return n != bit_size<NumericType>() ? value & mask(n) : value;
	}

	template<typename NumericType> inline std::size_t suffix_length(NumericType value, bool sample) noexcept
	{
		std::size_t result(0);
		while ((value & 0x1ul) == sample)
		{
			++result;
			value >>= 1;
		}

		return result;
	}

	template<typename NumericType> constexpr typename std::make_unsigned<NumericType>::type positive_part(NumericType value) noexcept
	{
		return value > 0 ? value : 0;
	}

	template<typename NumericType> constexpr NumericType abs_diff(NumericType a, NumericType b) noexcept
	{
		return a > b ? a - b : b - a;
	}

	template<typename NumericType> inline bool round_up(NumericType last_bits, std::size_t lsb_pow) noexcept
	{
		const NumericType round_bit(0x1ul << (lsb_pow - 1));
		const NumericType check_mask((lsb_pow + 2 <= bit_size<NumericType>()) ? ((round_bit << 2) - 1) ^ round_bit : round_bit - 1);

		std::cout << "round_bit: " << bool(last_bits & round_bit) << std::endl;

// 		return (last_bits & round_bit || (last_bits + 1) & round_bit) && (last_bits & check_mask);
		return (last_bits & round_bit) && (last_bits & check_mask);
	}

	template<typename NumericType> inline bool round_up(NumericType last_bits, std::size_t lsb_pow, bool minor_vote) noexcept
	{
		const NumericType round_bit(0x1ul << (lsb_pow - 1));
		const NumericType check_mask((lsb_pow + 2 <= bit_size<NumericType>()) ? ((round_bit << 2) - 1) ^ round_bit : round_bit - 1);

		std::cout << "last_bits: " << std::bitset<64>(last_bits) << std::endl;
		std::cout << "round bit exponent: " << (lsb_pow - 1) << std::endl;
		std::cout << "round_bit: " << bool(last_bits & round_bit) << std::endl;

// 		return (last_bits & round_bit || (last_bits + 1) & round_bit) && (last_bits & check_mask);
		return (last_bits & round_bit) && ((last_bits & check_mask) || minor_vote);
	}

	template<typename NumericType> inline NumericType add_with_carry(NumericType lhs, NumericType rhs, bool& carry) noexcept
	{
		std::cout << "add_with_carry lhs: " << std::bitset<bit_size<NumericType>()>(lhs).to_string() << std::endl;
		std::cout << "add_with_carry rhs: " << std::bitset<bit_size<NumericType>()>(rhs).to_string() << std::endl;
		if (!nth_bit(lhs, bit_size<NumericType>() - 1) && !nth_bit(rhs, bit_size<NumericType>() - 1))
		{
			carry = false;
			return lhs + rhs;
		}
		else
		{
			const NumericType l_one(lhs & 0x1), r_one(rhs & 0x1);
			const NumericType one_sum(l_one + r_one);
			const NumericType s_lhs(lhs >> 1), s_rhs(rhs >> 1);
			NumericType sum(s_lhs + s_rhs);
			std::cout << "lhs': " << std::bitset<bit_size<NumericType>()>(s_lhs).to_string() << std::endl;
			std::cout << "rhs': " << std::bitset<bit_size<NumericType>()>(s_rhs).to_string() << std::endl;
			std::cout << "sum: " << std::bitset<bit_size<NumericType>()>(sum).to_string() << std::endl;
			carry = nth_bit(sum, bit_size<NumericType>() - 1);
			if (carry || (sum << 1) < std::numeric_limits<NumericType>::max() - 1 || one_sum < 2)
			{
				return (sum << 1) + (l_one + r_one);
			}
			else
			{
				carry = true;
				return 0;
			}
		}
	}
}

#endif // FLOAXIE_BIT_OPS_H
