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
		constexpr const T& operator[](size_t index)
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