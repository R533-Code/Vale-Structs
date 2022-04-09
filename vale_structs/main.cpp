#include <comppch.h>

//Cannot be put in precompiled header
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>
#include <chrono>

#include <vale_structs/array.h>
#include <array>

template<typename T>
void testprint(vale::array_view<T> arr)
{
	for (auto& i : arr)
	{
		std::cout << i << '|';
	}
}

using namespace vale;

int main(int argc, char** argv)
{
	vale::array<int, 10> arr = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	std::cout << arr.to_view(0, 10);
	testprint(arr.to_view());
}