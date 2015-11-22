#include <cstring>
#include <iostream>

#include "floaxie/crosh.h"

using namespace std;
using namespace floaxie;

bool check_short()
{
	diy_fp w(0, 0);
	const char str1[] = "12345";

	digit_decomp(w, str1, strlen(str1));

	cout << "w: " << w << endl;
	cout << "value of w: " << w.mantissa() * std::pow(2, w.exponent()) << endl;

	return w.mantissa() * std::pow(2, w.exponent()) == 12345;
}

bool check_long()
{
	diy_fp w(0, 0);
	const char str1[] = "500000000000000166533453693773481063544750213623046875";

	digit_decomp(w, str1, strlen(str1));

	cout << "w: " << w << endl;
	cout << "value of w: " << w.mantissa() * std::pow(2, w.exponent()) << endl;

	return w.mantissa() == 10000000000000003331ul && w.exponent() == -1;
}

int main(int, char**)
{
	if (!check_short()) return 2;
	if (!check_long()) return 2;
	return 0;
}
