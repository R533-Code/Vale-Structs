#include <comppch.h>

//Cannot be put in precompiled header
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>
#include <chrono>

#include <vale_structs/array.h>
#include <vale_structs/variant.h>

using namespace vale;

#include <variant>
#define PRINT(val) std::cout << #val << ": " << val << '\n'

struct helper
{
	helper() { std::cout << "helper() called" << std::endl; throw std::exception{}; }
	//helper(const helper&) { std::cout << "helper(const helper&) called\n"; }
	helper(const helper&) = delete;
	helper(helper&&) noexcept { std::cout << "helper(helper&&) called\n"; }
	~helper() { std::cout << "~helper called!\n"; }
};

std::ostream& operator<<(std::ostream& os, const helper& var)
{
	os << "helper";
	return os;
}

int main(int argc, char** argv)
{
	vale::ts_variant<int, float, helper> arr1(10.0f);

	std::cout << std::boolalpha;
	PRINT(decltype(arr1)::alignment());
	PRINT(decltype(arr1)::buffer_byte_size());
	PRINT(decltype(arr1)::is_movable());
	PRINT(decltype(arr1)::is_noexcept_movable());
	PRINT(decltype(arr1)::is_copyable());
	PRINT(decltype(arr1)::is_noexcept_copyable());
	PRINT(arr1);
	//auto arr2 = arr1;
	//PRINT(arr2);
}