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
}

#endif // FLOAXIE_STATIC_POW_H
