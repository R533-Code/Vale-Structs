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
	struct is_parameter_pack_same_type {
		static_assert(std::conjunction_v<std::is_same<First, Rest>...>);
		using type = First;
	};
}