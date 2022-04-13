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
	helper() { std::cout << "helper() called\n"; }
	helper(const helper&) { std::cout << "helper(const helper&) called\n"; }
	helper(helper&&) noexcept { std::cout << "helper(helper&&) called\n"; }
	~helper() { std::cout << "~helper called!\n"; }
};

int main(int argc, char** argv)
{
	vale::variant<int, float> arr(1.01f);
	std::cout << arr << '\n';
	arr = 10;
	std::cout << arr << '\n';

	vale::array<int, 10, ThreadSafe> arr1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	PRINT(arr1);
	std::random_device rd;
	std::mt19937 g(rd());
	arr1.pass_iterators(std::shuffle<array_iterator<int>, std::mt19937&>, g);
	std::shuffle(array_iterator<int>(arr1.buffer), array_iterator<int>(arr1.buffer + arr1.size()), g);
	PRINT(arr1);
	arr1.pass_iterators(
		std::sort<array_iterator<int>, bool(const int&, const int&)>,
		[](const int& a, const int& b) {return a < b; }
	);
	PRINT(arr1);
}