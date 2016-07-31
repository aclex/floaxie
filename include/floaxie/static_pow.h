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
 */

#ifndef FLOAXIE_STATIC_POW_H
#define FLOAXIE_STATIC_POW_H

#include <array>
#include <utility>

namespace floaxie
{
	template<unsigned int base, unsigned int pow> struct static_pow_helper
	{
		static const unsigned int value = base * static_pow_helper<base, pow - 1>::value;
	};

	template<unsigned int base> struct static_pow_helper<base, 0>
	{
		static const unsigned int value = 1;
	};

	template<unsigned int base, unsigned int pow> constexpr unsigned long static_pow()
	{
		static_assert(base > 0, "Base should be positive");
		return static_pow_helper<base, pow>::value;
	}

	// Sequential power calculation

	template<typename T, T Add, typename Seq> struct concat_sequence;

	template<typename T, T Add, T... Seq> struct concat_sequence<T, Add, std::integer_sequence<T, Seq...>>
	{
		using type = std::integer_sequence<T, Seq..., Add>;
	};

	template<typename Seq> struct make_integer_array;

	template<typename T, T... Ints> struct make_integer_array<std::integer_sequence<T, Ints...>>
	{
		using type = std::array<T, sizeof...(Ints)>;
		static constexpr type value = type{Ints...};
	};

	template<typename T, T base, std::size_t current_pow> struct pow_sequencer
	{
		static const T value = pow_sequencer<T, base, current_pow - 1>::value * base;
		typedef typename concat_sequence<T, value, typename pow_sequencer<T, base, current_pow - 1>::sequence_type>::type sequence_type;
	};

	template<typename T, T base> struct pow_sequencer<T, base, 0>
	{
		static constexpr T value = 1;
		typedef std::integer_sequence<T, 1> sequence_type;
	};

	template<typename T, T base, std::size_t max_pow> constexpr T seq_pow(std::size_t pow)
	{
		typedef make_integer_array<typename pow_sequencer<T, base, max_pow>::sequence_type> maker;
		constexpr typename maker::type arr(maker::value);

		return arr[pow];
	}
}

#endif // FLOAXIE_STATIC_POW_H
