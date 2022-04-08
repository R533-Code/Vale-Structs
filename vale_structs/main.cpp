#include <comppch.h>

//Cannot be put in precompiled header
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>
#include <chrono>

#include <vale_structs/array.h>

int main(int argc, char** argv)
{
	using namespace std::literals::chrono_literals;

	std::cout << "--------THREAD_SAFE---------\n";
	for (size_t count = 0; count < 10; count++)
	{
		vale::array<int, 10, vale::ThreadSafe> ts_array = {};
		std::thread thr1([&]() {
			ts_array.for_each([](int& i) { i = 0; });
			});
		std::this_thread::sleep_for(0.2us);
		ts_array.for_each([](int& i) { i = 1; });
		
		thr1.join();
		std::cout << ts_array << '\n';
	}
	std::cout << "------NON_THREAD_SAFE-------\n";
	for (size_t count = 0; count < 10; count++)
	{
		vale::array<int, 10, vale::NonThreadSafe> nts_array = {};
		std::thread thr1([&]() {
			for (size_t i = 0; i < nts_array.size(); i++)
			{
				nts_array[i] = 0;
			}
			});
		std::this_thread::sleep_for(0.2us);
		for (size_t i = 0; i < nts_array.size(); i++)
		{
			nts_array[i] = 1;
		}

		thr1.join();
		std::cout << nts_array << '\n';
	}
}