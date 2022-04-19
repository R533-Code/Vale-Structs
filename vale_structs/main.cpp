#include <comppch.h>

//Cannot be put in precompiled header
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>
#include <chrono>

#include <vale_structs/array.h>
#include <vale_structs/variant.h>
#include <variant>

using namespace vale;

#define PRINT(val) std::cout << #val << ": " << val << '\n'

struct helper
{
	helper() { std::cout << "helper() called" << std::endl; throw std::exception{}; }
	helper(const helper&) { std::cout << "helper(const helper&) called\n"; }
	helper(helper&&) noexcept { std::cout << "helper(helper&&) called\n"; }
	~helper() { std::cout << "~helper called!\n"; }
};

auto func()
{
	return vale::variant<int, float, std::string>(std::to_string(10));
}

int main(int argc, char** argv)
{	
	vale::variant<int, float, std::string> arr1(func());
	std::cout << std::boolalpha;
	PRINT(decltype(arr1)::alignment());
	PRINT(decltype(arr1)::buffer_byte_size());
	PRINT(decltype(arr1)::is_movable());
	PRINT(decltype(arr1)::is_noexcept_movable());
	PRINT(decltype(arr1)::is_copyable());
	PRINT(decltype(arr1)::is_noexcept_copyable());
	PRINT(arr1);
	auto arr2 = arr1;
	PRINT(arr2);
}