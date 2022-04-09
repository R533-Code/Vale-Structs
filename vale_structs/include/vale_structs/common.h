#pragma once

#include <initializer_list>
#include <comppch.h>

namespace vale
{
	/// @brief Thread safety policy which signifies to a struct to use its thread safe implementation
	struct ThreadSafe {};
	/// @brief Thread safety policy which signifies to a struct to use its non-thread safe implementation
	struct NonThreadSafe{};

	/// @brief Buffer policy which signifies to a struct to use an optional buffer
	struct OptionalBuffer{};
	/// @brief Buffer policy which signifies to a struct to not use an optional buffer
	struct NonOptionalBuffer{};

	/// @brief Contains meta-programing utilities
	namespace details
	{
	template<typename T>
		/// @brief Helper struct to check if a type is [Non]ThreadSafe
		/// @tparam T The type to check for
	struct is_thread_safety_policy { static constexpr bool value = false; };

	template<>
		/// @brief Overload for ThreadSafe thread safety policy
	struct is_thread_safety_policy<ThreadSafe> { static constexpr bool value = true; };

	template<>
		/// @brief Overload for NonThreadSafe thread safety policy
	struct is_thread_safety_policy<NonThreadSafe> { static constexpr bool value = true; };

	template<typename T>
		/// @brief helper type to help determine if a type is [Non]ThreadSafe
		/// @tparam T The type to check for
	static constexpr bool is_thread_safety_policy_v = is_thread_safety_policy<T>::value;

	struct OptionalBuffer{};
	struct NonOptionalBuffer{};

	template <typename First, typename... Rest>
		/// @brief Checks if a parameter pack is formed by the same type
		/// @tparam First The first type
		/// @tparam ...Rest The rest of the parameter pack
		struct is_parameter_pack_of_same_type
		{
		static_assert(std::conjunction_v<std::is_same<First, Rest>...>);
		using type = First;
	};
	}

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