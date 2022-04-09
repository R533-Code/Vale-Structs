#pragma once

#include <initializer_list>
#include <comppch.h>

namespace vale
{
	struct ThreadSafe {};
	struct NonThreadSafe{};

	template<typename T>
	struct is_thread_safety_policy { static constexpr bool value = false; };

	template<>
	struct is_thread_safety_policy<ThreadSafe> { static constexpr bool value = true; };

	template<>
	struct is_thread_safety_policy<NonThreadSafe> { static constexpr bool value = true; };

	template<typename T>
	static constexpr bool is_thread_safety_policy_v = is_thread_safety_policy<T>::value;

	struct OptionalBuffer{};
	struct NonOptionalBuffer{};

	template <typename First, typename... Rest>
	struct is_parameter_pack_of_same_type {
		static_assert(std::conjunction_v<std::is_same<First, Rest>...>);
		using type = First;
	};

	template<typename T>
	struct contiguous_struct_view
	{
		T* ptr;
		size_t size;

		constexpr const T& operator[](size_t index)
		{
			if (index < size)
				return ptr[index];
			throw std::out_of_range("vale::contiguous_struct_view: Index was greater than size!");
		}
	};

	template<typename T>
	std::ostream& operator<<(std::ostream& os, const contiguous_struct_view<T>& var)
	{
		os << "{";
		for (size_t i = 0; i < var.size - 1; i++)
			os << *(var.ptr + i) << ", ";
		os << *(var.ptr + var.size - 1) << '}';
		return os;
	}
}