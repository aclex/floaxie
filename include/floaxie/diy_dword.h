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

#ifndef FLOAXIE_DIY_DWORD_H
#define FLOAXIE_DIY_DWORD_H

#include <limits>
#include <ostream>

// FIXME
#include <bitset>

#include <floaxie/bit_ops.h>
#include <floaxie/utility.h>

namespace floaxie
{
	template<typename WordType> class diy_dword
	{
	public:
		typedef WordType word_type;

		diy_dword() = default;
		diy_dword(const diy_dword&) = default;
		explicit diy_dword(word_type v) noexcept : m_higher(0), m_lower(v) { }
		diy_dword(word_type h, word_type l) noexcept : m_higher(h), m_lower(l) { }

		constexpr word_type higher() const noexcept { return m_higher; }
		constexpr word_type lower() const noexcept { return m_lower; }

		diy_dword& operator+=(word_type rhs) noexcept
		{
			assert(m_higher < std::numeric_limits<word_type>::max() || m_lower < std::numeric_limits<word_type>::max() - rhs);

			bool carry(false);
			m_lower = add_with_carry(m_lower, rhs, carry);

			m_higher += carry;

			return *this;
		}

		diy_dword operator+(word_type rhs) const noexcept
		{
			return diy_dword(*this).operator+=(rhs);
		}

		diy_dword& operator>>=(std::size_t n) noexcept
		{
			if (n < bit_size<word_type>())
			{
				std::cout << "n: " << n << std::endl;
				const word_type carrier(suffix(m_higher, n));
				std::cout << "carrier: " << carrier << std::endl;
				m_lower = (m_lower >> n) | (carrier << (bit_size<word_type>() - n));
				m_higher >>= n;
			}
			else
			{
				const std::size_t additional_shift_amount = n - bit_size<word_type>();
				m_lower = m_higher >> additional_shift_amount;
				m_higher = 0;
			}

			return *this;
		}

		diy_dword operator>>(std::size_t n) const noexcept
		{
			return diy_dword(*this).operator>>=(n);
		}

		diy_dword& operator<<=(std::size_t n) noexcept
		{
			if (n < bit_size<word_type>())
			{
				const word_type carrier(prefix(m_lower, n));
				m_lower <<= n;
				m_higher = (m_higher << n) | carrier;
			}
			else
			{
				const std::size_t additional_shift_amount = n - bit_size<word_type>();
				m_higher = m_lower << additional_shift_amount;
				m_lower = 0;
			}

			return *this;
		}

		diy_dword operator<<(std::size_t n) const noexcept
		{
			return diy_dword(*this).operator<<=(n);
		}

		constexpr explicit operator bool() const noexcept
		{
			return m_higher || m_lower;
		}

	private:
		word_type m_higher, m_lower;
	};


	template<typename WordType> inline diy_dword<WordType> dword_add(WordType a, WordType b) noexcept
	{
		return diy_dword<WordType>(a).operator+=(b);
	}

	template<typename WordType> diy_dword<WordType> dword_multiply(WordType lhs, WordType rhs) noexcept
	{
		std::cout << "dword_multiply" << std::endl;
		constexpr auto mask_32 = 0xffffffff;

		const WordType a = lhs >> 32;
		const WordType b = lhs & mask_32;
		const WordType c = rhs >> 32;
		const WordType d = rhs & mask_32;

		const WordType ac = a * c;
		const WordType bc = b * c;
		const WordType ad = a * d;
		const WordType bd = b * d;

		const WordType rz = bd;
		const WordType ry = (rz >> 32) + (ad & mask_32) + (bc & mask_32);
		const WordType rx = (ry >> 32) + (ad >> 32) + (bc >> 32) + (ac & mask_32);
		const WordType rw = (rx >> 32) + (ac >> 32);

// 		std::cout << "rw hex: " << std::hex << rw << std::endl;

		WordType rl = ((ry & mask_32) << 32) | (rz & mask_32);
		WordType rh = ((rw & mask_32) << 32) | (rx & mask_32);

// 		std::cout << "result hex: higher: " << std::hex << rh << ", lower: " << std::hex << rl << std::endl;

		std::cout << "result higher binary: " << std::bitset<64>(rh) << std::endl;
		std::cout << "result lower binary:  " << std::bitset<64>(rl) << std::endl;

		return diy_dword<WordType>(rh, rl);
	}

	template<typename CharType, typename WordType> std::basic_ostream<CharType>& operator<<(std::basic_ostream<CharType>& os, diy_dword<WordType> v)
	{
		os << print_binary(v.higher()) << ' ' << print_binary(v.lower());
		return os;
	}

	template<typename WordType> inline diy_dword<WordType> peek(diy_dword<WordType> value, std::size_t begin_bit_no, std::size_t end_bit_no) noexcept
	{
		typedef WordType word_type;
		assert(begin_bit_no < end_bit_no);

		if (begin_bit_no < bit_size<word_type>() && end_bit_no < bit_size<word_type>())
		{
			return diy_dword<WordType>(peek(value.lower(), begin_bit_no, end_bit_no));
		}

		if (begin_bit_no >= bit_size<word_type>() && end_bit_no >= bit_size<word_type>())
		{
			return diy_dword<WordType>(peek(value.higher(), begin_bit_no - bit_size<word_type>(), end_bit_no - bit_size<word_type>()));
		}

		// assuming begin_bit_no < bit_size<word_type>() && end_bit_no >= bit_size<word_type>()

		word_type lower = peek(value.lower(), begin_bit_no, bit_size<word_type>()), higher;
		const std::size_t lower_size(bit_size<word_type>() - begin_bit_no), lower_remaining_size(begin_bit_no);
		const std::size_t higher_size(end_bit_no - bit_size<word_type>());

		if (higher_size < lower_remaining_size)
		{
			lower |= peek(value.higher(), 0, higher_size) << lower_size;
			higher = 0;
		}
		else
		{
			lower |= peek(value.higher(), 0, lower_remaining_size) << lower_size;
			higher = peek(value.higher(), lower_remaining_size, higher_size);
		}

		return diy_dword<WordType>(higher, lower);
	}

	template<typename WordType> inline bool round_up(diy_dword<WordType> value, std::size_t lsb_pow) noexcept
	{
		typedef WordType word_type;
		if (lsb_pow < bit_size<word_type>())
		{
			return round_up(value.lower(), lsb_pow);
		}
		else if (lsb_pow > bit_size<word_type>())
		{
			const word_type round_bit_pow(lsb_pow - 1 - bit_size<word_type>());
			return nth_bit(value.higher(), round_bit_pow) && (nth_bit(value.higher(), lsb_pow) || value.lower());
		}
		else
		{
			const word_type round_bit_pow(bit_size<word_type>() - 1);
			return nth_bit(value.lower(), round_bit_pow) && (nth_bit(value.higher(), lsb_pow) || value.lower() << 1);
		}
	}
}

#endif // FLOAXIE_DIY_DWORD_H
