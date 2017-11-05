#include <array>
#include <utility>
#include <iostream>
#include <cmath>
#include <cstdint>

#include "floaxie/atof.h"
#include "floaxie/print.h"
#include "floaxie/type_punning_cast.h"

using namespace std;
using namespace floaxie;


array<pair<const char*, std::uint64_t>, 33> test_chain = {{
	make_pair("0", 0x0),
	make_pair("0.0", 0x0),
	make_pair("0.", 0x0),
	make_pair("18014398509481993", 0x4350000000000002),
	make_pair("0.9199", 0x3fed6fd21ff2e48f),
	make_pair("1.89", 0x3ffe3d70a3d70a3d),
	make_pair("3.50582559e-71", 0x314ef8a6590bf591),
	make_pair("0.500000000000000166533453693773481063544750213623046875", 0x3fe0000000000002),
	make_pair("3.14159265358979323846264", 0x400921fb54442d18),
	make_pair("7.8459735791271921e+65", 0x4d9dcd0089c1314e),
	make_pair("3.571e266", 0x77462644c61d41aa),
	make_pair("3.08984926168550152811e-32", 0x39640de48676653b),
	make_pair("1.2345689012e37", 0x47a29362ed1b4eb0),
	make_pair("3.518437208883201171875e13", 0x42c0000000000002),
	make_pair("62.5364939768271845828", 0x404f44abd5aa7ca4),
	make_pair("8.10109172351e-10", 0x3e0bd5cbaef0fd0c),
	make_pair("1.50000000000000011102230246251565404236316680908203125", 0x3ff8000000000000),
	make_pair("9007199254740991.4999999999999999999999999999999995", 0x433fffffffffffff),
	make_pair("9214843084008499", 0x43405e6cec57761a),
	make_pair("30078505129381147446200", 0x44997a3c7271b021),
	make_pair("1777820000000000000001", 0x4458180d5bad2e3e),
	make_pair("0.500000000000000166547006220929549868969843373633921146392822265625", 0x3fe0000000000002),
	make_pair("0.50000000000000016656055874808561867439493653364479541778564453125", 0x3fe0000000000002),
	make_pair("0.3932922657273", 0x3fd92bb352c4623a),
	make_pair("7.65801751e-323", 0x000000000000000f),
	make_pair("675698e-312", 0x005e5e10a3ab09aa),
	make_pair("0.0000763761", 0x3f140583682300c0),
	make_pair("7.41098468e-324", 0x0000000000000001),
	make_pair("6.95255e118", 0x589b92112776983c),
	make_pair("3.68715e-227", 0x10ebf37577860131),
	make_pair("4.30583e92", 0x532a6c122de8c02f),
	make_pair("3.01442e49", 0x4a34a020ad69249c),
	make_pair("1.6564e-174", 0x1bda382a65a69de1)
}};

array<pair<const char*, double>, 6> test_pain = {{
	make_pair("0.500000000000000166533453693773481063544750213623046875", 0.500000000000000166533453693773481063544750213623046875),
	make_pair("3.08984926168550152811e-32", 3.08984926168550152811e-32),
	make_pair("7.8459735791271921e+65", 7.8459735791271921e+65),
	make_pair("8.10109172351e-10", 8.10109172351e-10),
	make_pair("9007199254740991.4999999999999999999999999999999995", 9007199254740991.4999999999999999999999999999999995),
	make_pair("62.5364939768271845828", 62.5364939768271845828),
}};

array<pair<const char*, double>, 1> test_pain2 = {{
	make_pair("3.08984926168550152811e-32", 3.08984926168550152811e-32),
}};

int main(int, char**)
{
	cout << "s: " << test_chain[0].first << ", d: " << test_chain[0].second << endl;
	char* str_end;
	size_t fallback_count(0);
	auto fallback_lambda = [&fallback_count](const char* str, char** str_end)
	{
		++fallback_count;
		return floaxie::default_fallback<double>(str, str_end);
	};

	for (const auto& p : test_chain)
	{
		cout << "\nChecking \"" << p.first << "\"..." << endl;
		auto ret= atof<double>(p.first, &str_end, fallback_lambda);
		cout << "\tcorrect: " << print_binary(type_punning_cast<double>(p.second)) << endl;
		cout << "\tresult:  " << print_binary(ret.value) << endl;
		if (ret != type_punning_cast<double>(p.second))
		{
			cout << "Incorrect conversion!" << endl;
			return 2;
		}

		if (ret.status != conversion_status::success)
		{
			cout << "Flow status!" << endl;
			return 3;
		}

		cout << fallback_count << " times out of " << test_chain.size() << " fallback conversion was called" << endl;
	}

	// also check parsing of non-numeric string
	const char non_num_str[] = "aazz";
	atof<double>(non_num_str, &str_end);
	if (str_end != non_num_str)
		return 4;

	// and, finally, check conversion with no `str_end` specified
	atof<double>("0.5", nullptr);

	return 0;
}
