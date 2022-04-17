#pragma once

#include <initializer_list>
#include <iterator> // For iterator tags
#include <cstddef>  // For std::ptrdiff_t
#include <comppch.h>

namespace vale
{
	/// @brief Thread safety policy which signifies to a struct to use its thread safe implementation
	struct ThreadSafe {};
	/// @brief Thread safety policy which signifies to a struct to use its non-thread safe implementation
	struct NonThreadSafe {};

	/// @brief Buffer policy which signifies to a struct to use an optional buffer
	struct OptionalBuffer {};
	/// @brief Buffer policy which signifies to a struct to not use an optional buffer
	struct NonOptionalBuffer {};

	/// @brief Contains meta-programing utilities
	namespace details
	{
		template<size_t index, typename First, typename... Pack>
		/// @brief Helper type that stores the type at index 'index' of the type passed as template arguments.
		/// @tparam First The first type
		/// @tparam ...Pack Type pack
		struct recurse_index_pack
		{
			//we recurse till the index is 0
			using type =
				typename recurse_index_pack<index - 1, Pack...>::type;
		};

		template<typename First, typename... Pack>
		/// @brief Helper type that stores the type at index 'index' of the type passed as template arguments.
		/// This is the overload for when the type was found: in that case 'First' is the type to return.
		/// @tparam First The type at the passed index
		/// @tparam ...Pack Type pack
		struct recurse_index_pack<0, First, Pack...>
		{
			using type = First;
		};

		template<size_t index>
		/// @brief Helper type overload for when the index was greater than the size of a pack
		struct recurse_index_pack<index, void>
		{
			//We do not implement a ::type field, which will cause a compilation error.
		};

		template<bool condition, bool... conditions>
		/// @brief Unspecialized helper
		struct recurse_type_pack
		{};

		template<bool... conditions>
		/// @brief Helper for getting the index of a type in a parameter type
		struct recurse_type_pack<false, conditions...>
		{
			static constexpr uint64_t value = 1 + recurse_type_pack<conditions...>::value;
		};

		template<>
		/// @brief Helper for getting the index of a type in a parameter type
		struct recurse_type_pack<false>
		{
			//No field to signify that the type was not found
		};

		template<bool... conditions>
		/// @brief Helper for getting the index of a type in a parameter type
		struct recurse_type_pack<true, conditions...>
		{
			static constexpr uint64_t value = 0;
		};		
	}

	namespace helpers
	{
		template<typename First, typename... Rest>
		/// @brief Helper type to access the maximum sizeof all types passed as template arguments
		/// @tparam First The first type
		/// @tparam ...Rest Parameter pack
		struct get_max_size_of_type_pack
		{
			static constexpr uint64_t size = std::max({ sizeof(First), sizeof(Rest)... });
		};

		template<>
		/// @brief Helper type overload for errors, as void doesn't have a size
		struct get_max_size_of_type_pack<void>
		{
			//No 'size' field so this results in a compilation error
		};

		template<typename First, typename... Rest>
		/// @brief Helper to get the maximum sizeof all types passed as template arguments
		/// @tparam First The first type
		/// @tparam ...Rest Parameter pack
		static constexpr uint64_t get_max_size_of_type_pack_v = get_max_size_of_type_pack<First, Rest...>::size;

		template<typename First, typename... Rest>
		/// @brief Helper to get the minimum sizeof all types passed as template arguments
		/// @tparam First The first type
		/// @tparam ...Rest Parameter pack
		struct get_min_size_of_type_pack
		{
			static constexpr uint64_t size = std::min({ sizeof(First), sizeof(Rest)... });
		};

		template<>
		/// @brief Helper type overload for errors, as void doesn't have a size
		struct get_min_size_of_type_pack<void>
		{
			//No 'size' field to cause compilation error.
		};

		template<typename First, typename... Rest>
		/// @brief Helper to get the minimum sizeof all types passed as template arguments
		/// @tparam First The first type
		/// @tparam ...Rest Parameter pack
		static constexpr uint64_t get_min_size_of_type_pack_v = get_min_size_of_type_pack<First, Rest...>::size;

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

		template<typename First, typename... Rest>
		/// @brief helper type to count the number of fundamental types in a pack
		/// @tparam First The first type
		/// @tparam ...Rest The rest of the parameter pack
		struct count_fundamental
		{
			static constexpr uint64_t value = std::is_fundamental_v<First>
				+ (std::is_fundamental_v<Rest> + ...);
		}

		template<typename First, typename... Rest>
		/// @brief helper type to count the number of fundamental types in a pack
		/// @tparam First The first type
		/// @tparam ...Rest The rest of the parameter pack
		static constexpr uint64_t count_fundamental_v = count_fundamental<First, Rest...>::value;

		template<typename First, typename... Rest>
		/// @brief helper type to count the number of fundamental types in a pack
		/// @tparam First The first type
		/// @tparam ...Rest The rest of the parameter pack
		struct count_non_fundamental
		{
			static constexpr uint64_t value = !std::is_fundamental_v<First>
				+ (!std::is_fundamental_v<Rest> + ...);
		}

		template<typename First, typename... Rest>
		/// @brief helper type to count the number of fundamental types in a pack
		/// @tparam First The first type
		/// @tparam ...Rest The rest of the parameter pack
		static constexpr uint64_t count_non_fundamental_v = count_non_fundamental<First, Rest...>::value;

		template <typename First, typename... Rest>
		/// @brief Checks if a parameter pack is formed by the same type
		/// @tparam First The first type
		/// @tparam ...Rest The rest of the parameter pack
		struct is_parameter_pack_of_same_type
		{
			static_assert(std::conjunction_v<std::is_same<First, Rest>...>);
			using type = First;
		};

		template<typename Callable>
		/// @brief Helper to get the return type of a callable
		/// @tparam Callable The functor
		using return_type_of_callable_t = //Takes advantages of std::function's CTAD
			typename decltype(std::function{ std::declval<Callable>() })::result_type;

		template<size_t index, typename First, typename... Rest>
		/// @brief Returns the type at index 'index' of a pack
		/// @tparam First The first type
		/// @tparam ...Rest The pack from which to extract the type
		struct get_type_at_index_from_pack
		{
			static_assert(index < sizeof...(Rest), "Index was greater than sizeof...(Pack)!");
			using type =
				typename details::recurse_index_pack<index, First, Rest...>::type;
		};

		template<size_t index, typename First, typename... Rest>
		/// @brief Returns the type at index 'index' of a pack
		/// @tparam First The first type
		/// @tparam ...Rest The pack from which to extract the type
		using get_type_at_index_from_pack_t =
			typename get_type_at_index_from_pack<index, First, Rest...>::type;

		template<typename Type, typename First, typename... Rest>
		/// @brief Returns the index of 'Type' in 'First' + 'Rest'
		/// @tparam Type The type to search for		
		struct get_index_of_type_from_pack
		{
			static constexpr uint64_t value =
				details::recurse_type_pack<std::is_same_v<Type, First>, std::is_same_v<Type, Rest>...>::value;
		};

		template<typename Type, typename First, typename... Rest>
		/// @brief Returns the index of 'Type' in 'First' + 'Rest'
		/// @tparam Type The type to search for	
		static constexpr uint64_t get_index_of_type_from_pack_v = get_index_of_type_from_pack<Type, First, Rest...>::value;

		template<typename Type, typename First, typename... Rest>
		/// @brief Checks if a 'Type' is not found in 'First' + 'Rest'
		/// @tparam Type The type to check for
		struct is_type_not_in_pack
		{
			//We take advantage of bool being 1 when converted to an int, and also use fold expressions
			static constexpr bool value = static_cast<uint64_t>(std::is_same_v<Type, First> +(std::is_same_v<Type, Rest> +...)) < 1;
		};

		template<typename Type, typename First>
		/// @brief Checks if a type is not found in 'First'
		/// @tparam Type The type to check for
		struct is_type_not_in_pack<Type, First>
		{
			static constexpr bool value = static_cast<uint64_t>(std::is_same_v<Type, First>) < 1;
		};

		template<typename Type, typename First, typename... Rest>
		static constexpr bool is_type_not_in_pack_v = is_type_not_in_pack<Type, First, Rest...>::value;

		template<typename First, typename... Rest>
		/// @brief Check if a parameter pack does not contain any duplicate types
		struct is_pack_with_no_duplicates
		{
			static constexpr bool value =
				is_type_not_in_pack<First, Rest...>::value && is_pack_with_no_duplicates<Rest...>::value;
		};

		template<typename First, typename Second>
		/// @brief Check if a parameter pack does not contain any duplicate types
		struct is_pack_with_no_duplicates<First, Second>
		{
			static constexpr bool value = is_type_not_in_pack<First, Second>::value;
		};

		template<typename First, typename... Rest>
		/// @brief Check if a parameter pack does not contain any duplicate types
		static constexpr bool is_pack_with_no_duplicates_v = is_pack_with_no_duplicates<First, Rest...>::value;
	}

	template<typename T>
	/// @brief An iterator for object that are contiguous in memory
	/// @tparam T The type of to which the iterator points
	struct contiguous_iterator
	{
		//HELPER TAGS
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;

		//CONSTRUCTOR
		constexpr contiguous_iterator(pointer ptr)
			: ptr(ptr) {}

		//OPERATOR
		constexpr value_type& operator*() const { return *ptr; }
		constexpr value_type& operator->() const { return *ptr; }
		constexpr contiguous_iterator& operator++() { ptr++; return *this; }
		constexpr contiguous_iterator operator++(int) { contiguous_iterator tmp = *this; ++(*this); return tmp; }
		constexpr contiguous_iterator& operator--() { ptr--; return *this; }
		constexpr contiguous_iterator operator--(int) { contiguous_iterator tmp = *this; --(*this); return tmp; }
		constexpr contiguous_iterator operator+(difference_type val) { return ptr + val; }
		constexpr contiguous_iterator operator-(difference_type val) { return ptr - val; }
		constexpr difference_type operator-(const contiguous_iterator val) const { return ptr - val.ptr; }
		constexpr contiguous_iterator& operator+=(difference_type val) { ptr += val; }
		constexpr contiguous_iterator& operator-=(difference_type val) { ptr -= val; }
		//COMPARISONS
		friend constexpr bool operator== (const contiguous_iterator& a, const contiguous_iterator& b) { return a.ptr == b.ptr; };
		friend constexpr bool operator!= (const contiguous_iterator& a, const contiguous_iterator& b) { return a.ptr != b.ptr; };
		friend constexpr bool operator<	(const contiguous_iterator& a, const contiguous_iterator& b) { return a.ptr < b.ptr; }
		friend constexpr bool operator>	(const contiguous_iterator& a, const contiguous_iterator& b) { return a.ptr > b.ptr; }
		friend constexpr bool operator<=(const contiguous_iterator& a, const contiguous_iterator& b) { return a.ptr <= b.ptr; }
		friend constexpr bool operator>=(const contiguous_iterator& a, const contiguous_iterator& b) { return a.ptr >= b.ptr; }

	private:
		pointer ptr;
	};

	template<typename T>
	/// @brief A non-owning view of contiguous structs.
	/// Cheap object that should not be passed by reference.
	/// Should be preferred over passing structs by const-reference.
	/// @tparam T The type pointed to
	class contiguous_struct_view
	{
		/// @brief Pointer to the data
		const T* ptr;
		/// @brief The number of item pointed to
		size_t nb_elem;

	public:
		/// @brief Size of the view, or the number of objects it points to
		/// @return size_t representing the size of the view
		constexpr size_t size() const noexcept { return nb_elem; }

		/// @brief Returns a pointer to the beginning of the view
		/// @return const pointer to the data
		constexpr const T* data() const noexcept { return ptr; }

		/// @brief Returns the object at 'index', and throws if the index is out of range.
		/// @param index The index of the object
		/// @return const reference to the object
		constexpr const T& operator[](size_t index) const
		{
			if (index < nb_elem)
				return ptr[index];
			throw std::out_of_range("vale::contiguous_struct_view: Index was greater than size!");
		}

		/// @brief Constructs a struct view with a pointer and a size
		/// @param ptr Pointer to the beginning of the view
		/// @param size Number of element pointed to by the view
		constexpr contiguous_struct_view(const T* ptr, size_t size) noexcept
			: ptr(ptr), nb_elem(size)
		{}

		/// @brief Check if the view is empty (points to 0 objects)
		/// @return true if the view is empty
		constexpr bool is_empty() const noexcept { return nb_elem == 0; }

		/// @brief Returns the first object in the array
		/// @return const reference to the first object
		constexpr const T& front() const
		{
			if (!is_empty())
				return ptr[0];
			throw std::out_of_range("vale::contiguous_struct_view: View was empty!");
		}

		/// @brief Returns the last object in the array
		/// @return const reference to the last object
		constexpr const T& back() const
		{
			if (!is_empty())
				return ptr[nb_elem - 1];
			throw std::out_of_range("vale::contiguous_struct_view: View was empty!");
		}

		/// @brief Returns an iterator to the beginning of the array
		/// @return iterator to the beginning of the array
		constexpr contiguous_iterator<const T> begin()	const noexcept { return ptr; }
		/// @brief Returns an iterator to the end of the array
		/// @return iterator to the end of the array
		constexpr contiguous_iterator<const T> end()	const noexcept { return ptr + nb_elem; }

		/// @brief Returns an const iterator to the beginning of the array
		/// @return const iterator to the beginning of the array
		constexpr contiguous_iterator<const T> cbegin()	const noexcept { return ptr; }
		/// @brief Returns an const iterator to the end of the array
		/// @return const iterator to the end of the array
		constexpr contiguous_iterator<const T> cend()	const noexcept { return ptr + nb_elem; }

		/// @brief Check if the view starts with an object == 'with'
		/// @param with The object to compare with
		/// @return false if the view is empty or the first object != with.
		constexpr bool starts_with(const T& with) const
		{
			if (is_empty())
				return false;
			return with == ptr[0];
		}

		/// @brief Check if the view ends with an object == 'with'
		/// @param with The object to compare with
		/// @return false if the view is empty or the last object != with.
		constexpr bool ends_with(const T& with) const
		{
			if (is_empty())
				return false;
			return with == ptr[nb_elem - 1];
		}

		/// @brief Check if the view ends with an object == 'with'
		/// @param with The object to compare with
		/// @return false if the view is empty or the last object != with.
		constexpr bool contains(const T& with) const
		{
			for (size_t i = 0; i < nb_elem; i++)
			{
				if (with == ptr[i])
					return true;
			}
			return false;
		}

		/// @brief Check if 2 views contains the same objects.
		/// This does not check if the pointer and size are the same, but rather if
		/// all the objects pointed by the views are the same.
		/// @param a The view to compare with 'b'
		/// @param b The view to compare with 'a'
		/// @return Returns true if both views have the same size and contains the same objects
		friend constexpr bool operator==(const contiguous_struct_view<T>& a, const contiguous_struct_view<T>& b)
		{
			if (a.size() == b.size())
			{
				for (size_t i = 0; i < a.size(); i++)
				{
					if (a[i] != b[i])
						return false;
				}
				return true;
			}
			return false;
		}

		/// @brief Check if 2 views doesn't contain the same objects.
		/// This does not check if the pointer and size are the different, but rather if
		/// at least one of the objects pointed by the views is different.
		/// @param a The view to compare with 'b'
		/// @param b The view to compare with 'a'
		/// @return Returns false if both views have the same size and contains the same objects
		friend constexpr bool operator!=(const contiguous_struct_view<T>& a, const contiguous_struct_view<T>& b)
		{
			if (a.size() == b.size())
			{
				for (size_t i = 0; i < a.size(); i++)
				{
					if (a[i] != b[i])
						return true;
				}
				return false;
			}
			return true;
		}
	};

	template<typename T>
	/// @brief writes the content of a view between '{}', separating objects by ','. 
	std::ostream& operator<<(std::ostream& os, const contiguous_struct_view<T>& var)
	{
		os << "{";
		for (size_t i = 0; i < var.size() - 1; i++)
			os << *(var.data() + i) << ", ";
		os << *(var.data() + var.size() - 1) << '}';
		return os;
	}
}