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

#include <algorithm>
#include <limits>
#include <cstdint>
#include <cassert>
#include <ostream>
#include <utility>

#include <floaxie/bit_ops.h>
#include <floaxie/print.h>
#include <floaxie/type_punning_cast.h>

namespace floaxie
{
	class diy_fp
	{
	public:
		typedef std::uint64_t mantissa_storage_type;
		typedef int exponent_storage_type;

	private:
		template<typename FloatType> static constexpr mantissa_storage_type hidden_bit()
		{
			static_assert(std::numeric_limits<FloatType>::is_iec559, "Only IEEE-754 floating point types are supported");
			return raised_bit<mantissa_storage_type>(std::numeric_limits<FloatType>::digits - 1);
		}

	public:
		diy_fp() = default;
		diy_fp(const diy_fp&) = default;
		constexpr diy_fp(mantissa_storage_type mantissa, exponent_storage_type exponent) : m_f(mantissa), m_e(exponent) { }
		template<typename FloatType> explicit diy_fp(FloatType d) noexcept
		{
			static_assert(std::numeric_limits<FloatType>::is_iec559, "Only IEEE-754 floating point types are supported");

			constexpr auto full_mantissa_bit_size(std::numeric_limits<FloatType>::digits);
			constexpr auto mantissa_bit_size(full_mantissa_bit_size - 1); // remember hidden bit
			constexpr mantissa_storage_type mantissa_mask(mask<mantissa_storage_type>(mantissa_bit_size));
			constexpr mantissa_storage_type exponent_mask((~mantissa_mask) ^ msb_value<FloatType>()); // ignore sign bit
			constexpr exponent_storage_type exponent_bias(std::numeric_limits<FloatType>::max_exponent - 1 + mantissa_bit_size);

			mantissa_storage_type parts = type_punning_cast<mantissa_storage_type>(d);

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
		}

		template<typename FloatType> struct downsample_result
		{
			FloatType value;
			bool is_accurate;
		};

		template<typename FloatType> inline downsample_result<FloatType> downsample()
		{
			static_assert(std::numeric_limits<FloatType>::is_iec559, "Only IEEE-754 floating point types are supported.");
			static_assert(sizeof(FloatType) == sizeof(mantissa_storage_type), "Float type is not compatible.");

			assert(is_normalized());

			downsample_result<FloatType> ret;
			ret.is_accurate = true;

			constexpr auto full_mantissa_bit_size(std::numeric_limits<FloatType>::digits);
			constexpr auto mantissa_bit_size(full_mantissa_bit_size - 1); // remember hidden bit
			constexpr mantissa_storage_type my_mantissa_size(bit_size<mantissa_storage_type>());
			constexpr mantissa_storage_type mantissa_mask(mask<mantissa_storage_type>(mantissa_bit_size));
			constexpr exponent_storage_type exponent_bias(std::numeric_limits<FloatType>::max_exponent - 1 + mantissa_bit_size);
			constexpr std::size_t lsb_pow(my_mantissa_size - full_mantissa_bit_size);

			const auto f(m_f);

			if (m_e >= std::numeric_limits<FloatType>::max_exponent)
			{
				ret.value = std::numeric_limits<FloatType>::infinity();
				return ret;
			}

			if (m_e + int(my_mantissa_size) < std::numeric_limits<FloatType>::min_exponent - int(mantissa_bit_size))
			{
				ret.value = FloatType(0);
				return ret;
			}

			const std::size_t denorm_exp(positive_part(std::numeric_limits<FloatType>::min_exponent - int(mantissa_bit_size) - m_e - 1));

			assert(denorm_exp < my_mantissa_size);

			mantissa_storage_type parts;
			const std::size_t shift_amount(std::max(denorm_exp, lsb_pow));
			parts = (m_e + shift_amount + exponent_bias - (denorm_exp > lsb_pow)) << mantissa_bit_size;
			const auto& round(round_up(f, shift_amount));
			parts |= ((f >> shift_amount) + round.value) & mantissa_mask;

			ret.value = type_punning_cast<FloatType>(parts);
			ret.is_accurate = round.is_accurate;

			return ret;
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
			return m_f & msb_value<mantissa_storage_type>();
		}

		template<std::size_t original_matissa_bit_width> void normalize() noexcept
		{
			static_assert(original_matissa_bit_width >= 0, "Mantissa bit width should be >= 0");

			while (!nth_bit(m_f, original_matissa_bit_width))
			{
				m_f <<= 1;
				m_e--;
			}

			constexpr mantissa_storage_type my_mantissa_size(bit_size<mantissa_storage_type>());
			constexpr mantissa_storage_type e_diff = my_mantissa_size - original_matissa_bit_width - 1;

			m_f <<= e_diff;
			m_e -= e_diff;
		}

		void normalize() noexcept
		{
			while (!highest_bit(m_f))
			{
				m_f <<= 1;
				m_e--;
			}
		}

		diy_fp& operator-=(const diy_fp& rhs) noexcept
		{
			assert(m_e == rhs.m_e && m_f >= rhs.m_f);

			m_f -= rhs.m_f;

			return *this;
		}

		diy_fp operator-(const diy_fp& rhs) const noexcept
		{
			return diy_fp(*this) -= rhs;
		}

		diy_fp& operator*=(const diy_fp& rhs) noexcept
		{
			constexpr auto mask_32 = 0xffffffff;

			// TODO: don't rely on 64 bit size of mantissa_storage_type
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

		diy_fp operator*(const diy_fp& rhs) const noexcept
		{
			return diy_fp(*this) *= rhs;
		}

		diy_fp& operator++() noexcept
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
			if (m_f > 1)
			{
				--m_f;
			}
			else
			{
				m_f <<= 1;
				--m_f;
				--m_e;
			}
			return *this;
		}

		diy_fp operator--(int) noexcept
		{
			auto temp = *this;
			--(*this);
			return temp;
		}

		bool operator==(const diy_fp& d) const noexcept
		{
			return m_f == d.m_f && m_e == d.m_e;
		}

		bool operator!=(const diy_fp& d) const noexcept
		{
			return !operator==(d);
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

			constexpr auto d_digits(std::numeric_limits<FloatType>::digits);

			pl.normalize<d_digits>(); // as we increase precision of IEEE-754 type by 1

			const unsigned char shift_amount(1 + nth_bit(mi.m_f, d_digits));

			mi.m_f <<= shift_amount;
			mi.m_f -= 1;
			mi.m_e -= shift_amount;

			mi.m_f <<= mi.m_e - pl.m_e;
			mi.m_e = pl.m_e;

			return result;
		}

		template<typename Ch, typename Alloc>
		friend std::basic_ostream<Ch, Alloc>& operator<<(std::basic_ostream<Ch, Alloc>& os, const diy_fp& v)
		{
			os << "(f = " << print_binary(v.m_f) << ", e = " << v.m_e << ')';
			return os;
		}

	private:
		mantissa_storage_type m_f;
		exponent_storage_type m_e;
	};
}

#endif // FLOAXIE_DIY_FP_H
