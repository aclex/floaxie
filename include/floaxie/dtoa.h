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
 */

#ifndef FLOAXIE_DTOA_H
#define FLOAXIE_DTOA_H

#include <cmath>
#include <cassert>

#include <floaxie/grisu.h>
#include <floaxie/prettify.h>

namespace floaxie
{
	template<typename FloatType> inline void dtoa(FloatType v, char* buffer)
	{
		assert(!std::isnan(v));
		assert(!std::isinf(v));

		if (v == 0)
		{
			buffer[0] = '0';
			buffer[1] = '.';
			buffer[2] = '0';
			buffer[3] = '\0';
		}
		else
		{
			if (v < 0)
			{
				*buffer++ = '-';
				v = -v;
			}

			int len, K;
			grisu<-35, -32>(v, buffer, &len, &K);
			prettify_string(buffer, len, K);
		}
	}
}

#endif // FLOAXIE_DTOA_H
