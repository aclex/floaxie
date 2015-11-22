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

#ifndef FLOAXIE_DIY_FP_H
#define FLOAXIE_DIY_FP_H

#include <limits>
#include <cstdint>
#include <cassert>
#include <ostream>
#include <utility>

#include <iostream>
#include <bitset>

namespace floaxie
{
	template<typename NumericType> inline bool round_up(NumericType last_bits, std::size_t lsb_pow)
	{
		const NumericType round_bit(0x1ul << (lsb_pow - 1));
		const NumericType check_mask((round_bit << 2) - 1);

		std::cout << "round_bit: " << bool(last_bits & round_bit) << std::endl;

		return (last_bits & round_bit) && ((last_bits & check_mask) > round_bit);
	}

	class diy_fp
	{
	public:
		typedef std::uint64_t mantissa_storage_type;
		typedef int exponent_storage_type;

		template<typename NumericType> static constexpr std::size_t bit_size()
		{
			return sizeof(NumericType) * std::numeric_limits<unsigned char>::digits;
		}

	private:

		template<typename NumericType> static constexpr mantissa_storage_type msb_value()
		{
			return 0x1ul << (bit_size<NumericType>() - 1);
		}

		template<typename NumericType> static constexpr mantissa_storage_type max_integer_value()
		{
			return msb_value<NumericType>() + (msb_value<NumericType>() - 1);
		}

		template<std::size_t bit_size> static constexpr mantissa_storage_type hidden_bit()
		{
			return 0x1ul << bit_size;
		}

		template<typename FloatType> static constexpr mantissa_storage_type hidden_bit()
		{
			static_assert(std::numeric_limits<FloatType>::is_iec559, "Only IEEE-754 floating point types are supported");
			return 0x1ul << (std::numeric_limits<FloatType>::digits - 1);
		}

// 		template<typename FloatType> static bool round_up(mantissa_storage_type f)
// 		{
// 			constexpr auto mantissa_bit_size(std::numeric_limits<FloatType>::digits - 1); // remember hidden bit
// 			constexpr mantissa_storage_type my_mantissa_size(std::numeric_limits<mantissa_storage_type>::digits);
//
// 			constexpr mantissa_storage_type lsb_mask(0x1ul << (my_mantissa_size - mantissa_bit_size + 1));
// 			constexpr mantissa_storage_type rb_mask(0x1ul << (my_mantissa_size - mantissa_bit_size));
// 			constexpr mantissa_storage_type tail_mask((0x1ul << (my_mantissa_size - mantissa_bit_size - 1)) - 1);
//
// // 			if (f & rb_mask)
// // 			{
// // 				if (f & tail_mask || f & lsb_mask)
// // 					return true;
// // 				else
// // 					return false;
// // 			}
// // 			else
// // 				return false;
//
// 			return (f & rb_mask) & ((f & tail_mask) | (f & lsb_mask));
// 		}

	public:
		diy_fp() = default;
		diy_fp(const diy_fp&) = default;
		constexpr diy_fp(mantissa_storage_type mantissa, exponent_storage_type exponent) : m_f(mantissa), m_e(exponent) { }
		template<typename FloatType> explicit diy_fp(FloatType d) noexcept
		{
			static_assert(std::numeric_limits<FloatType>::is_iec559, "Only IEEE-754 floating point types are supported");

			union
			{
				FloatType value;
				mantissa_storage_type parts;
			};

			constexpr auto mantissa_bit_size(std::numeric_limits<FloatType>::digits - 1); // remember hidden bit
			constexpr mantissa_storage_type my_mantissa_size(std::numeric_limits<mantissa_storage_type>::digits);
			constexpr mantissa_storage_type mantissa_mask(max_integer_value<FloatType>() >> (my_mantissa_size - mantissa_bit_size));
			constexpr mantissa_storage_type exponent_mask((~(max_integer_value<FloatType>() & mantissa_mask)) ^ msb_value<FloatType>()); // ignore sign bit
			constexpr exponent_storage_type exponent_bias(std::numeric_limits<FloatType>::max_exponent - 1 + mantissa_bit_size);

			value = d;

			m_f = parts & mantissa_mask;
			m_e = (parts & exponent_mask) >> mantissa_bit_size;

			if (m_e)
			{
				m_f += hidden_bit<FloatType>();
				m_e -= exponent_bias;
			}
			else
			{
				m_e = 1 - exponent_bias;
			}

			std::cout << "f from double: " << std::bitset<64>(m_f) << std::endl;
		}

		template<typename FloatType> explicit operator FloatType() const noexcept
		{
			static_assert(std::numeric_limits<FloatType>::is_iec559, "Only IEEE-754 floating point types are supported");

			std::cout << "is_normalized: " << is_normalized() << std::endl;
			assert(is_normalized());

			union
			{
				FloatType value;
				mantissa_storage_type parts;
			};

			constexpr auto mantissa_bit_size(std::numeric_limits<FloatType>::digits - 1); // remember hidden bit
			constexpr mantissa_storage_type my_mantissa_size(std::numeric_limits<mantissa_storage_type>::digits);
			constexpr mantissa_storage_type mantissa_mask(max_integer_value<FloatType>() >> (my_mantissa_size - mantissa_bit_size));
			constexpr exponent_storage_type exponent_bias(std::numeric_limits<FloatType>::max_exponent - 1 + mantissa_bit_size);
			constexpr std::size_t lsb_pow(my_mantissa_size - (mantissa_bit_size + 1));

// 			const auto f(m_f + 1); // provoke brewing round up by 1 ulp (i.e. lead to round-to-nearest on truncation)
			const auto f(m_f);

			std::cout << "my_mantissa_size: " << my_mantissa_size <<", theirs mantissa size: " << mantissa_bit_size << std::endl;
			std::cout << "f: " << std::bitset<64>(f) << std::endl;
// 			std::cout << "mask: " << std::bitset<64>(mantissa_mask) << std::endl;
			std::cout << "would write: " << std::bitset<64>((f >> lsb_pow) & mantissa_mask) << std::endl;
			std::cout << "lsb_pow: " << lsb_pow << std::endl;

// 			if (m_e >= std::numeric_limits<FloatType>::max_exponent)
// 			{
// 				return std::numeric_limits<FloatType>::infinity();
// 			}
//
// 			if (m_e < std::numeric_limits<FloatType>::min_exponent - exponent_bias - my_mantissa_size)
// 			{
// 				return FloatType(0);
// 			}
//
// 			const exponent_storage_type denorm_exp(std::numeric_limits<FloatType>::min_exponent - exponent_bias - m_e);
//
// 			assert(denorm_exp < my_mantissa_size);
//
// 			if (denorm_exp > 0)
// 			{
// 				m_e += denorm_exp;
//
// 			}

			parts = (m_e + lsb_pow + exponent_bias) << mantissa_bit_size;
			parts |= ((f >> lsb_pow) + round_up(f, lsb_pow)) & mantissa_mask;

			return value;
		}

		constexpr mantissa_storage_type mantissa() const
		{
			return m_f;
		}

		constexpr exponent_storage_type exponent() const
		{
			return m_e;
		}

		bool is_normalized() const noexcept
		{
// 			std::cout << "mant: " << std::bitset<64>(m_f) << std::endl;
// 			std::cout << "mask: " << std::bitset<64>(msb_value<mantissa_storage_type>()) << std::endl;
			return m_f & msb_value<mantissa_storage_type>();
		}

		template<std::size_t original_matissa_bit_width> std::size_t normalize() noexcept
		{
			static_assert(original_matissa_bit_width >= 0, "Mantissa bit width should be >= 0");

			assert(!is_normalized());

			const auto initial_e = m_e;

			while (!(m_f & hidden_bit<original_matissa_bit_width>()))
			{
				m_f <<= 1;
				m_e--;
			}

			constexpr mantissa_storage_type my_mantissa_size(std::numeric_limits<mantissa_storage_type>::digits);
			constexpr mantissa_storage_type e_diff = my_mantissa_size - original_matissa_bit_width - 1;

			m_f <<= e_diff;
			m_e -= e_diff;

			return initial_e - m_e;
		}

		std::size_t normalize() noexcept
		{
			return normalize<std::numeric_limits<double>::digits>();
		}

		diy_fp& operator-=(const diy_fp& rhs) noexcept
		{
			assert(m_e == rhs.m_e && m_f >= rhs.m_f);

			m_f -= rhs.m_f;
			m_e = rhs.m_e;

			return *this;
		}

		diy_fp operator-(const diy_fp& rhs) const noexcept
		{
			return diy_fp(*this) -= rhs;
		}

		diy_fp& operator*=(const diy_fp& rhs) noexcept
		{
			std::cout << "operator*" << std::endl;
			std::cout << "op1: " << (*this) << std::endl;
			std::cout << "op2: " << (*this) << std::endl;
			constexpr auto mask_32 = 0xffffffff;

			const mantissa_storage_type a = m_f >> 32;
			const mantissa_storage_type b = m_f & mask_32;
			const mantissa_storage_type c = rhs.m_f >> 32;
			const mantissa_storage_type d = rhs.m_f & mask_32;

			const mantissa_storage_type ac = a * c;
			const mantissa_storage_type bc = b * c;
			const mantissa_storage_type ad = a * d;
			const mantissa_storage_type bd = b * d;

			const mantissa_storage_type tmp = (bd >> 32) + (ad & mask_32) + (bc & mask_32) + (0x1ul << 31);

			m_f = ac + (ad >> 32) + (bc >> 32) + (tmp >> 32);
			m_e += rhs.m_e + 64;

			return *this;
		}

		diy_fp& precise_multiply(const diy_fp& rhs) noexcept
		{
			std::cout << "precise_multiply" << std::endl;
			std::cout << "op2: " << (*this) << std::endl;
			std::cout << "op2: " << rhs << std::endl;
			std::cout << "op1: mantissa hex: " << std::hex << m_f << ", e: " << m_e << std::endl;
			std::cout << "op2: mantissa hex: " << std::hex << rhs.m_f << ", e: " << rhs.m_e << std::endl;
			constexpr auto mask_32 = 0xffffffff;

			const mantissa_storage_type a = m_f >> 32;
			const mantissa_storage_type b = m_f & mask_32;
			const mantissa_storage_type c = rhs.m_f >> 32;
			const mantissa_storage_type d = rhs.m_f & mask_32;

			const mantissa_storage_type ac = a * c;
			const mantissa_storage_type bc = b * c;
			const mantissa_storage_type ad = a * d;
			const mantissa_storage_type bd = b * d;

			mantissa_storage_type rl = bd + (((ad + bc) & mask_32) << 32);
			mantissa_storage_type rh = ac + ((ad + bc) >> 32);

			std::cout << "result hex: higher: " << std::hex << rh << ", lower: " << std::hex << rl << std::endl;

			std::cout << "result higher binary: " << std::bitset<64>(rh) << std::endl;
			std::cout << "result lower binary:  " << std::bitset<64>(rl) << std::endl;

			std::size_t shift_count(0);

			while (!(rh & msb_value<mantissa_storage_type>()))
			{
				rh <<= 1;
				++shift_count;
			}

			m_f = rh;
			if (shift_count)
				m_f +=  rl >> (bit_size<mantissa_storage_type>() - shift_count);

			const mantissa_storage_type round_bit_mask(0x1ul << (bit_size<mantissa_storage_type>() - shift_count - 1));

			std::cout << "shift_count: " << shift_count << std::endl;
// 			std::cout << "residue: " << (rl >> (bit_size<mantissa_storage_type>() - shift_count)) << std::endl;
// 			std::cout << "residue: " << (rl >> 64) << std::endl;

			const bool should_round_up(rl & round_bit_mask);
			std::cout << "should_round_up: " << should_round_up << std::endl;

			if (should_round_up)
			{
				if (m_f < std::numeric_limits<diy_fp::mantissa_storage_type>::max())
				{
					++m_f;
				}
				else
				{
					m_f >>= 1;
					++m_f;
					++m_e;
				}

				const mantissa_storage_type epsilon_bit_mask(0x1ul << (bit_size<mantissa_storage_type>() - shift_count - 2));
				m_f |= bool(rl & epsilon_bit_mask); // copy '1' from next-to-round bit to ulp to move value from middle after rounding
			}

			m_e += rhs.m_e + 64 - static_cast<int>(shift_count);

			return (*this);
		}

		diy_fp& precise_multiply2(const diy_fp& rhs) noexcept
		{
			std::cout << "precise_multiply2" << std::endl;
			std::cout << "op2: " << (*this) << std::endl;
			std::cout << "op2: " << rhs << std::endl;
			std::cout << "op1: mantissa hex: " << std::hex << m_f << ", e: " << m_e << std::endl;
			std::cout << "op2: mantissa hex: " << std::hex << rhs.m_f << ", e: " << rhs.m_e << std::endl;
			constexpr auto mask_32 = 0xffffffff;

			const mantissa_storage_type a = m_f >> 32;
			const mantissa_storage_type b = m_f & mask_32;
			const mantissa_storage_type c = rhs.m_f >> 32;
			const mantissa_storage_type d = rhs.m_f & mask_32;

			const mantissa_storage_type ac = a * c;
			const mantissa_storage_type bc = b * c;
			const mantissa_storage_type ad = a * d;
			const mantissa_storage_type bd = b * d;

			const mantissa_storage_type rz = bd;
			const mantissa_storage_type ry = (rz >> 32) + (ad & mask_32) + (bc & mask_32);
			const mantissa_storage_type rx = (ry >> 32) + (ad >> 32) + (bc >> 32) + (ac & mask_32);
			const mantissa_storage_type rw = (rx >> 32) + (ac >> 32);

			std::cout << "rw hex: " << std::hex << rw << std::endl;

			mantissa_storage_type rl = ((ry & mask_32) << 32) | (rz & mask_32);
			mantissa_storage_type rh = ((rw & mask_32) << 32) | (rx & mask_32);

			std::cout << "result hex: higher: " << std::hex << rh << ", lower: " << std::hex << rl << std::endl;

			std::cout << "result higher binary: " << std::bitset<64>(rh) << std::endl;
			std::cout << "result lower binary:  " << std::bitset<64>(rl) << std::endl;
			std::cout << "result lower decimal: " << std::dec << rl << std::endl;
			std::cout << "type maximum:         " << std::numeric_limits<std::uint64_t>::max() << std::endl;

			std::size_t shift_count(0);

			while (!(rh & msb_value<mantissa_storage_type>()))
			{
				rh <<= 1;
				++shift_count;
			}

			m_f = rh;
			if (shift_count)
			{
				m_f +=  rl >> (bit_size<mantissa_storage_type>() - shift_count);
				std::cout << "rl copied prefix binary: " << std::bitset<64>(rl >> (bit_size<mantissa_storage_type>() - shift_count)) << std::endl;
				rl <<= shift_count;

				std::cout << "rl[shifted] binary:  " << std::bitset<64>(rl) << std::endl;
				std::cout << "rl[shifted] decimal: " << std::dec << (rl) << std::endl;
				std::cout << "type maximum:        " << std::numeric_limits<std::uint64_t>::max() << std::endl;
			}

			const mantissa_storage_type round_bit_mask(0x1ul << (bit_size<mantissa_storage_type>() - 1));
			std::cout << "rl[shifted] binary:    " << std::bitset<64>(rl) << std::endl;
			std::cout << "round_bit_mask binary: " << std::bitset<64>(round_bit_mask) << std::endl;

			std::cout << "shift_count: " << shift_count << std::endl;
// 			std::cout << "residue: " << (rl >> (bit_size<mantissa_storage_type>() - shift_count)) << std::endl;
// 			std::cout << "residue: " << (rl >> 64) << std::endl;

			const bool should_round_up(rl & round_bit_mask);
			std::cout << "should_round_up: " << should_round_up << std::endl;

			if (should_round_up)
			{
				if (m_f < std::numeric_limits<diy_fp::mantissa_storage_type>::max())
				{
					++m_f;
				}
				else
				{
					m_f >>= 1;
					++m_f;
					++m_e;
				}

				const mantissa_storage_type epsilon_bit_mask(0x1ul << (bit_size<mantissa_storage_type>() - 2));
				m_f |= bool(rl & epsilon_bit_mask); // copy '1' from next-to-round bit to ulp to move value from middle after rounding
			}

			std::cout << "final mantissa binary: " << std::bitset<64>(m_f) << std::endl;

			m_e += rhs.m_e + 64 - static_cast<int>(shift_count);

			std::cout << std::dec;

			return (*this);
		}

		diy_fp& precise_multiply3(const diy_fp& rhs) noexcept
		{
			std::cout << "precise_multiply2" << std::endl;
			std::cout << "op2: " << (*this) << std::endl;
			std::cout << "op2: " << rhs << std::endl;
			std::cout << "op1: mantissa hex: " << std::hex << m_f << ", e: " << m_e << std::endl;
			std::cout << "op2: mantissa hex: " << std::hex << rhs.m_f << ", e: " << rhs.m_e << std::endl;
			constexpr auto mask_32 = 0xffffffff;

			const mantissa_storage_type a = m_f >> 32;
			const mantissa_storage_type b = m_f & mask_32;
			const mantissa_storage_type c = rhs.m_f >> 32;
			const mantissa_storage_type d = rhs.m_f & mask_32;

			const mantissa_storage_type ac = a * c;
			const mantissa_storage_type bc = b * c;
			const mantissa_storage_type ad = a * d;
			const mantissa_storage_type bd = b * d;

			const mantissa_storage_type rz = bd;
			const mantissa_storage_type ry = (rz >> 32) + (ad & mask_32) + (bc & mask_32);
			const mantissa_storage_type rx = (ry >> 32) + (ad >> 32) + (bc >> 32) + (ac & mask_32);
			const mantissa_storage_type rw = (rx >> 32) + (ac >> 32);

			std::cout << "rw hex: " << std::hex << rw << std::endl;

			mantissa_storage_type rl = ((ry & mask_32) << 32) | (rz & mask_32);
			mantissa_storage_type rh = ((rw & mask_32) << 32) | (rx & mask_32);

			std::cout << "result hex: higher: " << std::hex << rh << ", lower: " << std::hex << rl << std::endl;

			std::cout << "result higher binary: " << std::bitset<64>(rh) << std::endl;
			std::cout << "result lower binary:  " << std::bitset<64>(rl) << std::endl;
			std::cout << "result lower decimal: " << std::dec << rl << std::endl;
			std::cout << "type maximum:         " << std::numeric_limits<std::uint64_t>::max() << std::endl;

			std::size_t shift_count(0);

			const mantissa_storage_type round_bit_mask(0x1ul << (bit_size<mantissa_storage_type>() - shift_count - 1));
			std::cout << "rl again binary:       " << std::bitset<64>(rl) << std::endl;
			std::cout << "round_bit_mask binary: " << std::bitset<64>(round_bit_mask) << std::endl;

			std::cout << "shift_count: " << shift_count << std::endl;
// 			std::cout << "residue: " << (rl >> (bit_size<mantissa_storage_type>() - shift_count)) << std::endl;
// 			std::cout << "residue: " << (rl >> 64) << std::endl;

			const bool should_round_up(rl & round_bit_mask);
			std::cout << "should_round_up: " << should_round_up << std::endl;

			m_f = rh;

			if (should_round_up)
			{
				if (m_f < std::numeric_limits<diy_fp::mantissa_storage_type>::max())
				{
					++m_f;
				}
				else
				{
					m_f >>= 1;
					++m_f;
					++m_e;
				}
			}

			while (!(m_f & msb_value<mantissa_storage_type>()))
			{
				m_f <<= 1;
				++shift_count;
			}
// 			if (shift_count)
// 			{
// 				m_f +=  rl >> (bit_size<mantissa_storage_type>() - shift_count);
// 				std::cout << "rl copied prefix binary: " << std::bitset<64>(rl << (bit_size<mantissa_storage_type>() - shift_count)) << std::endl;
//
// 				std::cout << "result lower[shifted] binary:  " << std::bitset<64>(rl << shift_count) << std::endl;
// 				std::cout << "result lower[shifted] decimal: " << std::dec << (rl << shift_count) << std::endl;
// 				std::cout << "type maximum:                  " << std::numeric_limits<std::uint64_t>::max() << std::endl;
// 			}

			if (should_round_up)
			{
				const mantissa_storage_type epsilon_bit_mask(0x1ul << (bit_size<mantissa_storage_type>() - 1));
				m_f |= bool(rl & epsilon_bit_mask); // copy '1' from next-to-round bit to ulp to move value from middle after rounding
			}

			std::cout << "final mantissa binary: " << std::bitset<64>(m_f) << std::endl;

			m_e += rhs.m_e + 64 - static_cast<int>(shift_count);

			std::cout << std::dec;

			return (*this);
		}

		diy_fp operator*(const diy_fp& rhs) const noexcept
		{
			return diy_fp(*this) *= rhs;
		}

		diy_fp& operator++() noexcept
		{
			++m_f;
			return *this;
		}

		diy_fp operator++(int) noexcept
		{
			auto temp = *this;
			++(*this);
			return temp;
		}

		diy_fp& operator--() noexcept
		{
			--m_f;
			return *this;
		}

		diy_fp operator--(int) noexcept
		{
			auto temp = *this;
			--(*this);
			return temp;
		}

		template<typename FloatType> static std::pair<diy_fp, diy_fp> boundaries(FloatType d) noexcept
		{
			static_assert(std::numeric_limits<FloatType>::is_iec559, "Only IEEE-754 floating point types are supported");

			std::pair<diy_fp, diy_fp> result;
			diy_fp &mi(result.first), &pl(result.second);
			pl = diy_fp(d);
			mi = pl;

			pl.m_f <<= 1;
			pl.m_f += 1;

			pl.m_e  -= 1;
			pl.normalize<std::numeric_limits<FloatType>::digits>(); // as we increase precision of IEEE-754 type by 1

			if (mi.m_f == hidden_bit<FloatType>())
			{
				mi.m_f <<= 2;
				mi.m_f -= 1;

				mi.m_e -= 2;
			}
			else
			{
				mi.m_f <<= 1;
				mi.m_f -= 1;

				mi.m_e -= 1;
			}

			mi.m_f <<= mi.m_e - pl.m_e;
			mi.m_e = pl.m_e;

			return result;
		}

		template<typename Ch, typename Alloc> friend std::basic_ostream<Ch, Alloc>& operator<<(std::basic_ostream<Ch, Alloc>& os, const diy_fp& v)
		{
			os << "(f = " << v.m_f << ", e = " << v.m_e << ')';
			return os;
		}

	private:
		mantissa_storage_type m_f;
		exponent_storage_type m_e;
	};
}

#endif // FLOAXIE_DIY_FP_H
