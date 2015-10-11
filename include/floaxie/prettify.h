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
 * prettify_string() and fill_exponent() functions use code taken from
 * Florian Loitsch's original Grisu algorithms implementation
 * (http://florian.loitsch.com/publications/bench.tar.gz)
 * and "Printing Floating-Point Numbers Quickly and Accurately with
 * Integers" paper
 * (http://florian.loitsch.com/publications/dtoa-pldi2010.pdf)
 */

#ifndef FLOAXIE_PRETTIFY_H
#define FLOAXIE_PRETTIFY_H

#include <cstring>
#include <cassert>
#include <cstddef>

namespace floaxie
{
	inline void fill_exponent(int K, char* buffer)
	{
		int i = 0;

		if (K < 0)
		{
			buffer[i++] = '-';
			K = -K;
		}

		if (K >= 100)
		{
			buffer[i++] = '0' + K / 100; K %= 100;
			buffer[i++] = '0' + K / 10; K %= 10;
			buffer[i++] = '0' + K;
		}
		else if (K >= 10)
		{
			buffer[i++] = '0' + K / 10; K %= 10;
			buffer[i++] = '0' + K;
		}
		else
		{
			buffer[i++] = '0' + K;
		}

		buffer[i] = '\0';
	}

	inline void prettify_string(char* buffer, int len, int k)
	{
		/* v = buffer * 10^k
			kk is such that 10^(kk-1) <= v < 10^kk
			this way kk gives the position of the comma.
		*/
		const int kk = len + k;

		if (len <= kk && kk <= 21)
		{
			/* the first digits are already in. Add some 0s and call it a day. */
			/* the 21 is a personal choice. Only 16 digits could possibly be relevant.
				* Basically we want to print 12340000000 rather than 1234.0e7 or 1.234e10 */
			std::memset(buffer + len, '0', kk - len + 2);
			buffer[kk] = '.';
			buffer[kk + 2] = '\0';
		}
		else if (0 < kk && kk <= 21)
		{
			/* comma number. Just insert a '.' at the correct location. */
			std::memmove(buffer + kk + 1, buffer + kk, len - kk);
			buffer[kk] = '.';
			buffer[len + 1] = '\0';
		}
		else if (-6 < kk && kk <= 0)
		{
			/* something like 0.000abcde.
				* add '0.' and some '0's */
			const int offset = 2 - kk;
			std::memmove(buffer + offset, buffer, len);
			std::memset(buffer, '0', offset);
			buffer[1] = '.';
			buffer[len + offset] = '\0';
		}
		else if (len == 1)
		{
			/* just add 'e...' */
			buffer[1] = 'e';
			fill_exponent(kk - 1, buffer + 2);
		}
		else
		{
			/* leave the first digit. then add a '.' and at the end 'e...' */
			std::memmove(buffer + 2, buffer + 1, len - 1);
			buffer[1] = '.';
			buffer[len + 1] = 'e';
			fill_exponent(kk - 1, buffer + len + 2);
		}
	}
}

#endif // FLOAXIE_PRETTIFY_H
