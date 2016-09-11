/*
 * Copyright 2015, 2016 Alexey Chernov <4ernov@gmail.com>
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

#include <cstddef>
#include <limits>
#include <cassert>

#include <floaxie/integer_of_size.h>

namespace floaxie
{
	template<typename NumericType> constexpr std::size_t bit_size() noexcept
	{
		return sizeof(NumericType) * std::numeric_limits<unsigned char>::digits;
	}

	template<typename NumericType> constexpr NumericType raised_bit(std::size_t power)
	{
		assert(power < bit_size<NumericType>());
		return NumericType(1) << power;
	}

	template<typename FloatType,
	typename NumericType =  typename integer_of_size<sizeof(FloatType)>::type>
	constexpr NumericType msb_value() noexcept
	{
		return raised_bit<NumericType>(bit_size<NumericType>() - 1);
	}

	template<typename FloatType,
	typename NumericType = typename integer_of_size<sizeof(FloatType)>::type>
	constexpr NumericType max_integer_value() noexcept
	{
		return std::numeric_limits<NumericType>::max();
	}

	template<typename NumericType> constexpr bool nth_bit(NumericType value, std::size_t power) noexcept
	{
		return value & raised_bit<NumericType>(power);
	}

	template<typename NumericType> constexpr bool highest_bit(NumericType value) noexcept
	{
		return nth_bit(value, bit_size<NumericType>() - 1);
	}

	template<typename NumericType> constexpr NumericType mask(std::size_t n) noexcept
	{
		return n < bit_size<NumericType>() ? raised_bit<NumericType>(n) - 1 : std::numeric_limits<NumericType>::max();
	}

	template<typename NumericType> constexpr NumericType mask(std::size_t begin_bit_no, std::size_t end_bit_no) noexcept
	{
		return mask<NumericType>(end_bit_no) ^ mask<NumericType>(begin_bit_no);
	}

	template<typename NumericType> constexpr NumericType peek(NumericType value, std::size_t begin_bit_no, std::size_t end_bit_no) noexcept
	{
		return (value & mask<NumericType>(begin_bit_no, end_bit_no)) >> begin_bit_no;
	}

	template<typename NumericType> constexpr NumericType prefix(NumericType value, std::size_t n) noexcept
	{
		return n != bit_size<NumericType>() ? value >> (bit_size<NumericType>() - n) : value;
	}

	template<typename NumericType> constexpr NumericType suffix(NumericType value, std::size_t n) noexcept
	{
		return n != bit_size<NumericType>() ? value & mask<NumericType>(n) : value;
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

	struct round_result
	{
		bool value;
		bool is_accurate;
	};

	template<typename NumericType> inline round_result round_up_fast(NumericType last_bits, std::size_t round_to_power) noexcept
	{
		round_result ret;

		const NumericType round_bit(raised_bit<NumericType>(round_to_power - 1));
		const NumericType check_mask(mask<NumericType>(round_to_power + 1) ^ round_bit);
		ret.is_accurate = suffix(last_bits, round_to_power) != round_bit;
		ret.value = (last_bits & round_bit) && (last_bits & check_mask);

		return ret;
	}

	template<typename NumericType> inline round_result round_up(NumericType last_bits, std::size_t round_to_power) noexcept
	{
		return round_up_fast(last_bits, round_to_power);
	}
}

#endif // FLOAXIE_BIT_OPS_H
