#include <comppch.h>

//Cannot be put in precompiled header
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>
#include <chrono>

#include <vale_structs/array.h>
#include <vale_structs/variant.h>

#include <string>

using namespace vale;
using namespace std::string_literals;

#define PRINT(val) std::cout << #val << ": " << val << '\n'

int main(int argc, char** argv)
{
	vale::array array_variants = { vale::variant<int, float, std::string>(10.0f), vale::variant<int, float, std::string>("Hello Vale"s) };
	PRINT(array_variants);
}