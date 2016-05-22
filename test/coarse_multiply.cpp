#include "floaxie/atof.h"
#include "floaxie/diy_fp.h"

#include "short_numbers.h"

using namespace std;
using namespace floaxie;

int main(int, char**)
{
	double results[10];
	bool accurate;
	for (size_t i = 0; i < 10; ++i)
	{
		diy_fp&& diy_result(diy_fp(short_numbers[i]) * diy_fp(short_numbers[i]));
		diy_result.normalize();
		results[i] = diy_result.downsample<double>(&accurate);
	}
}
