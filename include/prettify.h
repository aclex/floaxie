#ifndef PRETTIFY_H
#define PRETTIFY_H

#include <cstring>

inline void fill_exponent(int K, char* buffer)
{
	int i = 0;
	if (K < 0)
	{
		buffer[i++] = '-';
		K = -K;
	}
	else
	{
		buffer[i++] = '+';
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

inline void prettify_string(char* buffer, int from_pos, int end_pos, int k)
{
	int nb_digits = end_pos - from_pos;
	int i, offset;
	/* v = buffer * 10^k
		kk is such that 10^(kk-1) <= v < 10^kk
		this way kk gives the position of the comma.
	*/
	int kk = nb_digits + k;

	buffer[end_pos] = '\0';
	if (nb_digits <= kk && kk <= 21)
	{
		/* the first digits are already in. Add some 0s and call it a day. */
		/* the 21 is a personal choice. Only 16 digits could possibly be relevant.
			* Basically we want to print 12340000000 rather than 1234.0e7 or 1.234e10 */
		for (i = nb_digits; i < kk; i++)
			buffer[from_pos + i] = '0';
		buffer[kk] = '.';
		buffer[kk+1] = '0';
		buffer[kk+2] = '\0';
	}
	else if (0 < kk && kk <= 21)
	{
		/* comma number. Just insert a '.' at the correct location. */
		std::memmove(&buffer[from_pos + kk + 1], &buffer[from_pos + kk], nb_digits - kk);
		buffer[from_pos + kk] = '.';
		buffer[from_pos + nb_digits + 1] = '\0';
	}
	else if (-6 < kk && kk <= 0)
	{
		/* something like 0.000abcde.
			* add '0.' and some '0's */
		offset = 2-kk;
		std::memmove(&buffer[from_pos + offset], &buffer[from_pos], nb_digits);
		buffer[from_pos] = '0';
		buffer[from_pos + 1] = '.';
		for (i = from_pos + 2; i < from_pos + offset; i++)
			buffer[i] = '0';
		buffer[from_pos + nb_digits + offset] = '\0';
	}
	else if (nb_digits == 1)
	{
		/* just add 'e...' */
		buffer[from_pos + 1] = 'e';
		/* fill_positive_fixnum will terminate the string */
		fill_exponent(kk - 1, &buffer[from_pos + 2]);
	}
	else
	{
		/* leave the first digit. then add a '.' and at the end 'e...' */
		std::memmove(&buffer[from_pos + 2], &buffer[from_pos + 1], nb_digits-1);
		buffer[from_pos + 1] = '.';
		buffer[from_pos + nb_digits + 1] = 'e';
		/* fill_fixnum will terminate the string */
		fill_exponent(kk - 1, &buffer[from_pos + nb_digits + 2]);
	}
}

#endif // PRETTIFY_H
