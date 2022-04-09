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

#define PRINT(val) std::cout << #val << ": " << val << '\n'

int main(int argc, char** argv)
{
	vale::array<int, 10, ThreadSafe> arr1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	vale::array<int, 10, ThreadSafe> arr2 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	std::reverse(contiguous_iterator(arr1.data()), contiguous_iterator(arr1.data() + arr1.size()));
	PRINT(arr1);
	PRINT(arr2);
	arr1.swap(arr2);
	PRINT(arr1);
	PRINT(arr2);
}