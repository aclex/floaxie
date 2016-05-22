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

#ifndef FLOAXIE_INTEGER_OF_SIZE_H
#define FLOAXIE_INTEGER_OF_SIZE_H

#include <cstdint>
#include <cstddef>

namespace floaxie
{
	template<typename T> struct identity
	{
		typedef T type;
	};

	template<std::size_t size> struct integer_of_size : identity<std::uintmax_t()> {};
	template<> struct integer_of_size<sizeof(std::uint64_t)> : identity<std::uint64_t> {};
	template<> struct integer_of_size<sizeof(std::uint32_t)> : identity<std::uint32_t> {};
}

#endif // FLOAXIE_INTEGER_OF_SIZE_H
