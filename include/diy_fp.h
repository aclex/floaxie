#ifndef DIY_FP_H
#define DIY_FP_H

#include <limits>
#include <cstdint>
#include <cassert>

#include <iostream>
#include <utility>

constexpr std::uint64_t double_mantissa_mask(0x000fffffffffffff);
constexpr std::uint64_t double_exponent_mask(0x7ff0000000000000);
constexpr std::uint64_t double_mantissa_size(std::numeric_limits<double>::digits - 1); // remember of hidden bit
constexpr std::uint64_t diy_fp_mantissa_size(std::numeric_limits<std::uint64_t>::digits);
constexpr int double_exponent_bias(0x3ff + double_mantissa_size);
constexpr std::uint64_t double_hidden_bit(0x1ul << double_mantissa_size);

class c_diy_fp
{
public:
	typedef std::uint64_t mantissa_storage_type;
	typedef int exponent_storage_type;

private:
	template<typename FloatType> static constexpr std::size_t bit_size()
	{
		return sizeof(FloatType) * std::numeric_limits<unsigned char>::digits;
	}

	template<typename FloatType> static constexpr mantissa_storage_type msb_value()
	{
		return 0x1ul << (bit_size<FloatType>() - 1);
	}

	template<typename FloatType> static constexpr mantissa_storage_type max_integer_value()
	{
		return msb_value<FloatType>() + (msb_value<FloatType>() - 1);
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

public:
	c_diy_fp() = default;
	c_diy_fp(const c_diy_fp&) = default;
	constexpr c_diy_fp(mantissa_storage_type mantissa, exponent_storage_type exponent) noexcept : m_f(mantissa), m_e(exponent) { }
	template<typename FloatType> explicit c_diy_fp(FloatType d) noexcept
	{
		static_assert(std::numeric_limits<FloatType>::is_iec559, "Only IEEE-754 floating point types are supported");

		union
		{
			FloatType value;
			mantissa_storage_type parts;
		};

		constexpr auto mantissa_bit_size(std::numeric_limits<FloatType>::digits - 1); // remember hidden bit
		constexpr mantissa_storage_type mantissa_mask(max_integer_value<FloatType>() >> (diy_fp_mantissa_size - mantissa_bit_size));
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
	}

	constexpr mantissa_storage_type mantissa() const noexcept
	{
		return m_f;
	}

	constexpr exponent_storage_type exponent() const noexcept
	{
		return m_e;
	}

	template<std::size_t original_matissa_bit_width> void normalize() noexcept
	{
		static_assert(original_matissa_bit_width >= 0, "Mantissa bit width should be >= 0");

		assert(!(m_f & msb_value<mantissa_storage_type>()));

		while (!(m_f & hidden_bit<original_matissa_bit_width>()))
		{
			m_f <<= 1;
			m_e--;
		}

		constexpr mantissa_storage_type e_diff = diy_fp_mantissa_size - original_matissa_bit_width - 1;
		m_f <<= e_diff;
		m_e -= e_diff;
	}

	c_diy_fp& operator-=(const c_diy_fp& rhs) noexcept
	{
		assert(m_e == rhs.m_e && m_f >= rhs.m_f);
		m_f -= rhs.m_f;
		m_e = rhs.m_e;

		return *this;
	}

	c_diy_fp operator-(const c_diy_fp& rhs) const noexcept
	{
		return c_diy_fp(*this) -= rhs;
	}

	c_diy_fp& operator*=(const c_diy_fp& rhs) noexcept
	{
		constexpr auto mask_32 = 0xffffffff;

		const std::uint64_t a = m_f >> 32;
		const std::uint64_t b = m_f & mask_32;
		const std::uint64_t c = rhs.m_f >> 32;
		const std::uint64_t d = rhs.m_f & mask_32;

		const std::uint64_t ac = a * c;
		const std::uint64_t bc = b * c;
		const std::uint64_t ad = a * d;
		const std::uint64_t bd = b * d;

		const std::uint64_t tmp = (bd >> 32) + (ad & mask_32) + (bc & mask_32) + (1ul << 31);

		m_f = ac + (ad >> 32) + (bc >> 32) + (tmp >> 32);
		m_e += rhs.m_e + 64;
	}

	c_diy_fp operator*(const c_diy_fp& rhs) const noexcept
	{
		return c_diy_fp(*this) *= rhs;
	}

	c_diy_fp& operator++() noexcept
	{
		++m_f;
		return *this;
	}

	c_diy_fp operator++(int) noexcept
	{
		auto temp = *this;
		++(*this);
		return temp;
	}

	c_diy_fp& operator--() noexcept
	{
		--m_f;
		return *this;
	}

	c_diy_fp operator--(int) noexcept
	{
		auto temp = *this;
		--(*this);
		return temp;
	}

	template<typename FloatType> static inline std::pair<c_diy_fp, c_diy_fp> normalized_boundaries(FloatType d) noexcept
	{
		static_assert(std::numeric_limits<FloatType>::is_iec559, "Only IEEE-754 floating point types are supported");

		std::pair<c_diy_fp, c_diy_fp> result;
		c_diy_fp &mi(result.first), &pl(result.second);
		pl = c_diy_fp(d);
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

	template<typename Ch, typename Alloc> friend std::basic_ostream<Ch, Alloc>& operator<<(std::basic_ostream<Ch, Alloc>& os, const c_diy_fp& v)
	{
		os << "(f = " << v.m_f << ", e = " << v.m_e << ')';
		return os;
	}

private:
	mantissa_storage_type m_f;
	exponent_storage_type m_e;
};

struct diy_fp
{
	std::uint64_t f;
	int e;
};

template<std::size_t original_matissa_bit_width> diy_fp normalize_diy_fp(diy_fp d)
{
	static_assert(original_matissa_bit_width >= 0, "Mantissa bit width should be >= 0");

	constexpr std::uint64_t hidden_bit(0x1ul << original_matissa_bit_width);

	diy_fp res = d;

	while (!(res.f & hidden_bit))
	{
		res.f <<= 1;
		res.e--;
	}

	constexpr std::uint64_t e_diff = diy_fp_mantissa_size - original_matissa_bit_width - 1;
	res.f <<= e_diff;
	res.e = res.e - e_diff;

	return res;
}

template<typename FloatType> inline diy_fp double2diy_fp(FloatType d)
{
	static_assert(std::numeric_limits<FloatType>::is_iec559, "Only IEEE-754 floating point types are supported");

	union
	{
		FloatType value;
		std::uint64_t parts;
	};

	constexpr auto type_bit_size(sizeof(FloatType) * std::numeric_limits<unsigned char>::digits);
	constexpr std::uint64_t type_msb_value(0x1ul << (type_bit_size - 1));
	constexpr std::uint64_t type_max_value(type_msb_value + (type_msb_value - 1));

	constexpr auto mantissa_size(std::numeric_limits<FloatType>::digits - 1); // remember hidden bit
	constexpr std::uint64_t hidden_bit(0x1ul << mantissa_size);
	constexpr std::uint64_t mantissa_mask(type_max_value >> (diy_fp_mantissa_size - mantissa_size));
	constexpr std::uint64_t exponent_mask((~(type_max_value & mantissa_mask)) ^ type_msb_value); // ignore sign bit
	constexpr int exponent_bias(std::numeric_limits<FloatType>::max_exponent - 1 + mantissa_size);

	value = d;

	std::uint64_t mantissa = parts & mantissa_mask;
	int exponent = (parts & exponent_mask) >> mantissa_size;

	if (exponent)
	{
		mantissa += hidden_bit;
		exponent -= exponent_bias;
	}
	else
	{
		exponent = 1 - exponent_bias;
	}

	diy_fp res { .f = mantissa, .e = exponent };

	return res;
}

inline diy_fp minus(diy_fp x, diy_fp y)
{
	assert(x.e == y.e && x.f >= y.f);
	return diy_fp {.f = x.f - y.f, .e = x.e };
}

inline diy_fp multiply(diy_fp x, diy_fp y)
{
	constexpr auto mask_32 = 0xffffffff;

	const std::uint64_t a = x.f >> 32;
	const std::uint64_t b = x.f & mask_32;
	const std::uint64_t c = y.f >> 32;
	const std::uint64_t d = y.f & mask_32;

	const std::uint64_t ac = a * c;
	const std::uint64_t bc = b * c;
	const std::uint64_t ad = a * d;
	const std::uint64_t bd = b * d;

	const std::uint64_t tmp = (bd >> 32) + (ad & mask_32) + (bc & mask_32) + (1ul << 31);

	return diy_fp { .f = ac + (ad >> 32) + (bc >> 32) + (tmp >> 32), .e = x.e + y.e + 64 };
}

inline void normalized_boundaries(double d, diy_fp* wm, diy_fp* wp)
{
	diy_fp v = double2diy_fp(d);
	diy_fp pl, mi;

	const bool significand_is_zero = v.f == double_hidden_bit;

	pl.f  = (v.f << 1) + 1;
	pl.e  = v.e - 1;
	pl = normalize_diy_fp<double_mantissa_size + 1>(pl); // as we increase precision of double by 1

	if (significand_is_zero)
	{
		mi.f = (v.f << 2) - 1;
		mi.e = v.e - 2;
	}
	else
	{
		mi.f = (v.f << 1) - 1;
		mi.e = v.e - 1;
	}

	mi.f <<= mi.e - pl.e;
	mi.e = pl.e;

	*wp = pl;
	*wm = mi;
}

#endif // DIY_FP_H
