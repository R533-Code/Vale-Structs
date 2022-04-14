#pragma once
#include <vale_structs/common.h>

namespace vale
{
	/// @brief Unspecialized array which defaults to a NonThreadSafe.
	/// Reports an error if ThreadSafety is not a thread safety policy: [Non]ThreadSafe.
	/// @tparam T The type of the array
	/// @tparam nb_elem The size of the array
	/// @tparam ThreadSafety The thread safety policy of the array
	template<typename T, size_t nb_elem, typename ThreadSafety = NonThreadSafe>
	class array { static_assert(helpers::is_thread_safety_policy_v<ThreadSafety>, "ThreadSafety can only be ThreadSafe/NonThreadSafe"); };

	template<typename T>
	/// @brief Alias over a contiguous_struct_view for more easy typing
	/// @tparam T The type of the view
	using array_view = contiguous_struct_view<T>;

	template<typename T>
	/// @brief Alias over a contiguous_iterator for more easy typing
	/// @tparam T The type of the view
	using array_iterator = contiguous_iterator<T>;

	template<typename T, size_t nb_elem>
	/// @brief A thread safe array which provides helpful methods when working concurrently.
	/// Thread safe array which provides facilities to access its content concurrently.
	/// Does not possess iterators or .to_view() facilities to avoid false usages.
	/// The mutex is public to permit CTAD. It is recommended NEVER to lock it,
	/// as all methods but '.data()' and '.size()' lock it.
	/// @tparam T The type of the objects to store.
	class array<T, nb_elem, ThreadSafe>
	{
		// Check that the array has a size > 0
		static_assert(nb_elem > 0, "Array size should be greater than 0!");

	public:
		/// @brief Helper alias for iterators
		using iterator = array_iterator<T>;
		/// @brief Helper alias for const iterators
		using const_iterator = array_iterator<const T>;

		/// @brief Fills the array by assigning 'obj' to each of its item.
		constexpr void fill(const T& obj) noexcept(std::is_nothrow_copy_assignable_v<T>)
		{
			std::scoped_lock lock(mutex);
			for (size_t i = 0; i < nb_elem; i++)
				buffer[i] = obj;
		}

		constexpr void swap(array& other) noexcept(std::is_nothrow_move_assignable_v<T>)
		{
			std::scoped_lock lock(mutex, other.mutex);
			for (size_t i = 0; i < nb_elem; i++)
			{
				T temp = std::move(other.buffer[i]);
				other.buffer[i] = std::move(buffer[i]);
				buffer[i] = std::move(temp);
			}
		}
		
		/// @brief Returns the object at 'index', and throws if the index is out of range.
		/// If the index is greater than nb_elem - 1, throws std::out_of_range
		/// @param index The index of the object
		/// @return reference to the object
		[[nodiscard]] constexpr T& operator[](size_t index)
		{
			std::scoped_lock lock(mutex);
			if (index < nb_elem)
				return buffer[index];
			throw std::out_of_range("vale::array: index was greater than size!");
		}

		/// @brief Returns the object at 'index', and throws if the index is out of range.
		/// If the index is greater than nb_elem - 1, throws std::out_of_range
		/// @param index The index of the object
		/// @return const reference to the object
		[[nodiscard]] constexpr const T& operator[](size_t index) const
		{
			std::scoped_lock lock(mutex);
			if (index < nb_elem)
				return buffer[index];
			throw std::out_of_range("vale::array: index was greater than size!");
		}

		/// @brief Returns the last object in the array
		/// @return const reference to the last object
		[[nodiscard]] constexpr const T& back() const
		{
			std::scoped_lock lock(mutex);
			return buffer[nb_elem - 1];
		}

		/// @brief Returns the last object in the array
		/// @return reference to the last object
		[[nodiscard]] constexpr T& back()
		{
			std::scoped_lock lock(mutex);
			return buffer[nb_elem - 1];
		}

		/// @brief Returns the first object in the array
		/// @return const reference to the first object
		[[nodiscard]] constexpr const T& front() const
		{
			std::scoped_lock lock(mutex);
			return buffer[0];
		}

		/// @brief Returns the first object in the array
		/// @return reference to the first object
		[[nodiscard]] constexpr T& front()
		{
			std::scoped_lock lock{ mutex };
			return buffer[0];
		}

		/// @brief Access the object at 'index' through a functor.
		/// Accesses the object at 'index' and passes it to a functor that accepts
		/// a 'const T&'. 
		/// Returns true if the object is passed to the function, which means the index was in the range of the array.
		/// @param index The index of the object
		/// @param func function to which the object is passed
		/// @return false if the index is out of range, true if the object was passed to the function
		constexpr bool access_index(size_t index, void(*func)(const T&)) const noexcept(noexcept(func(std::declval<T>())))
		{
			std::scoped_lock lock{ mutex };
			if (index < nb_elem)
			{
				func(buffer[index]);
				return true;
			}
			return false;
		}

		/// @brief Access the object at 'index' through a functor.
		/// Accesses the object at 'index' and passes it to a functor that accepts
		/// a 'T&'. 
		/// Returns true if the object is passed to the function, which means the index was in the range of the array.
		/// @param index The index of the object
		/// @param func function to which the object is passed
		/// @return false if the index is out of range, true if the object was passed to the function
		constexpr bool access_index(size_t index, void(*func)(T&)) noexcept(noexcept(func(std::declval<T>())))
		{
			std::scoped_lock lock{ mutex };
			if (index < nb_elem)
			{
				func(buffer[index]);
				return true;
			}
			return false;
		}

		/// @brief Call a functor with each of the object in the array
		/// @param func The functor to which a reference of each object is passed
		constexpr void for_each(void(*func)(T&)) noexcept(noexcept(func(std::declval<T>())))
		{
			std::scoped_lock lock{ mutex };
			for (size_t i = 0; i < nb_elem; i++)
				func(buffer[i]);
		}

		/// @brief Call a functor with each of the object in the array
		/// @param func The functor to which a const reference of each object is passed
		constexpr void for_each(void(*func)(const T&)) const noexcept(noexcept(func(std::declval<T>())))
		{
			std::scoped_lock lock{ mutex };
			for (size_t i = 0; i < nb_elem; i++)
				func(buffer[i]);
		}		

		template<typename Func, typename... Args>
		/// @brief Passes begin and end iterators followed by an argument pack to a function, and returns the result of the function.
		/// This method can be used to thread-safely access iterators of the array.
		/// This method is not responsible of the thread-safety of the result of the function:
		/// to access the result of the function thread-safely, use pass_iterators_and(...) instead.
		/// \code{.cpp}		
		/// vale::array<int, 10, ThreadSafe> arr1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		/// arr1.pass_iterators(std::sort<array_iterator<int>>); //Sorts the array thread-safely
		/// \endcode
		/// @tparam Func Type of the function to which to pass the iterators followed by the argument pack
		/// @tparam ...Args Parameter pack
		/// @param function The function to which to pass the iterators followed by the argument pack
		/// @param ...args Argument pack forwarded to 'function' after iterators
		/// @return What is returned by the 'function'
		constexpr auto pass_iterators(Func function, Args&&... args) -> helpers::return_type_of_callable_t<Func>
		{
			std::scoped_lock lock{ mutex };
			return function(contiguous_iterator(buffer), contiguous_iterator(buffer + nb_elem), std::forward<Args>(args)...);
		}

		template<typename Func, typename... Args>
		/// @brief Passes begin and end iterators, followed by an argument pack to a function, and passes the result of that function to 'and_result'.
		/// This method can be used to thread-safely access the result of a function to which we passed
		/// iterators of the array, and some other parameters in the form of a pack.
		/// \code{.cpp}
		/// vale::array<int, 10, ThreadSafe> arr1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		/// arr1.pass_iterators_and(
		///		[](array_iterator<int> i) { /*DO SOMETHING USEFUL WITH THE RESULT*/; }, //Access the result of max_element
		///		std::max_element<array_iterator<int>, bool(const int&, const int&)>,
		///		[](const int& a, const int& b) {return a < b; } //Passed to max_element
		/// );
		/// \endcode
		/// @tparam Func The functor to which iterators and an argument pack are passed
		/// @tparam ...Args The parameter pack
		/// @param and_result The functor to which the result of 'function' is passed
		/// @param function The functor to which begin/end iterators and the parameter pack are passed
		/// @param ...args The argument pack which is forwarded to 'function' after passing begin/end iterators
		constexpr void pass_iterators_and(void(*and_result)(helpers::return_type_of_callable_t<Func>), Func function, Args&&... args)
		{
			std::scoped_lock lock{ mutex };
			and_result(function(contiguous_iterator(buffer), contiguous_iterator(buffer + nb_elem), std::forward<Args>(args)...));
		}

		/// @brief Returns the size of the array.
		/// The size of the array is the template parameter 'nb_elem'.
		/// Does not lock the mutex protecting the data.
		/// @return The size of the array
		[[nodiscard]] constexpr size_t size() const noexcept { return nb_elem; }

		/// @brief Returns a pointer to the beginning of the data
		/// Does not lock the mutex protecting the data.
		/// @return const pointer to the beginning of the data
		[[nodiscard]] constexpr const T* data() const noexcept { return buffer; }

		/// @brief Returns a pointer to the beginning of the data.
		/// Does not lock the mutex protecting the data.
		/// @return pointer to the beginning of the data
		[[nodiscard]] constexpr T* data() noexcept { return buffer; }

		inline void print(std::ostream& os) const
		{
			std::scoped_lock lock{ mutex };
			os << "{";
			for (size_t i = 0; i < size - 1; i++)
				os << *(var.data() + i) << ", ";
			os << *(var.data() + size - 1) << '}';
		}

	public: //MEMBERS

		/// @brief C-style array of objects
		T buffer[nb_elem];
		/// @brief The mutex which protects the data
		mutable std::mutex mutex{};
	};

	template<typename T, size_t nb_elem>
	/// @brief A non-thread safe array of objects
	/// @tparam T The type of the array
	class array<T, nb_elem, NonThreadSafe>
	{
		// Check that the array has a size > 0
		static_assert(nb_elem > 0, "Array size should be greater than 0!");

	public:
		/// @brief Helper alias for iterators
		using iterator = array_iterator<T>;
		/// @brief Helper alias for const iterators
		using const_iterator = array_iterator<const T>;

		/// @brief Fills the array by assigning 'obj' to each of its item.
		constexpr void fill(const T& obj) noexcept(std::is_nothrow_copy_assignable_v<T>)
		{
			for (size_t i = 0; i < nb_elem; i++)
				buffer[i] = obj;
		}

		/// @brief Returns the object at 'index', and throws if the index is out of range.
		/// If the index is greater than nb_elem - 1, throws std::out_of_range
		/// @param index The index of the object
		/// @return reference to the object
		[[nodiscard]] constexpr T& operator[](size_t index)
		{
			if (index < nb_elem)
				return buffer[index];
			throw std::out_of_range("vale::array: index was greater than size!");
		}

		/// @brief Returns the object at 'index', and throws if the index is out of range.
		/// If the index is greater than nb_elem - 1, throws std::out_of_range
		/// @param index The index of the object
		/// @return const reference to the object
		[[nodiscard]] constexpr const T& operator[](size_t index) const
		{
			if (index < nb_elem)
				return buffer[index];
			throw std::out_of_range("vale::array: index was greater than size!");
		}

		/// @brief Returns the last object in the array
		/// @return const reference to the last object
		[[nodiscard]] constexpr const T& back() const	noexcept { return buffer[nb_elem - 1]; }

		/// @brief Returns the last object in the array
		/// @return reference to the last object
		[[nodiscard]] constexpr T& back()				noexcept { return buffer[nb_elem - 1]; }

		/// @brief Returns the first object in the array
		/// @return const reference to the first object
		[[nodiscard]] constexpr const T& front() const	noexcept { return buffer[0]; }

		/// @brief Returns the first object in the array
		/// @return reference to the first object
		[[nodiscard]] constexpr T& front()				noexcept { return buffer[0]; }

		/// @brief Returns the size of the array.
		/// The size of the array is the template parameter 'nb_elem'.
		/// @return The size of the array
		[[nodiscard]] constexpr size_t size()	const	noexcept { return nb_elem; }

		/// @brief Returns a pointer to the beginning of the data
		/// @return const pointer to the beginning of the data
		[[nodiscard]] constexpr const T* data() const	noexcept { return buffer; }

		/// @brief Returns a pointer to the beginning of the data
		/// @return pointer to the beginning of the data
		[[nodiscard]] constexpr T* data()				noexcept { return buffer; }

		/// @brief Returns an iterator to the beginning of the array
		/// @return iterator to the beginning of the array
		[[nodiscard]] constexpr array_iterator<T> begin()		noexcept { return array_iterator<T>(buffer); }
		/// @brief Returns an iterator to the end of the array
		/// @return iterator to the end of the array
		[[nodiscard]] constexpr array_iterator<T> end()		noexcept { return array_iterator<T>(buffer + nb_elem); }

		/// @brief Returns an const iterator to the beginning of the array
		/// @return const iterator to the beginning of the array
		[[nodiscard]] constexpr array_iterator<const T> cbegin()		const noexcept { return buffer + nb_elem; }
		/// @brief Returns an const iterator to the end of the array
		/// @return const iterator to the end of the array
		[[nodiscard]] constexpr array_iterator<const T> cend()		const noexcept { return buffer + nb_elem; }

		/// @brief Returns a view of all the items in the struct
		/// @return view of all the items in the struct
		[[nodiscard]] constexpr array_view<T> to_view() noexcept { return array_view<T>(buffer, nb_elem); }
		
		/// @brief Returns a view of all the items in the struct starting from offset.
		/// Throws if offset >= nb_elem.
		/// @return view of all the items in the struct beginning from offset, or throws.
		[[nodiscard]] array_view<T> to_view(size_t offset)
		{ 
			if (offset < nb_elem)
				return array_view<T>(buffer + offset, nb_elem - offset);
			throw std::out_of_range("vale::array: offset was greater than size!");
		}
		
		/// @brief Returns a view of 'size' items in the struct starting from offset.
		/// Throws if offset >= nb_elem.
		/// @return view of 'size' items in the struct beginning from offset, or throws.
		[[nodiscard]] array_view<T> to_view(size_t offset, size_t size)
		{
			if (offset + size - 1 < nb_elem)
			{
				return array_view<T>(buffer + offset, size);
			}
			throw std::out_of_range("vale::array: offset + size was greater than size!");
		}

		inline void print(std::ostream& os) const
		{
			os << "{";
			for (size_t i = 0; i < size - 1; i++)
				os << *(var.data() + i) << ", ";
			os << *(var.data() + size - 1) << '}';
		}

	public: //MEMBERS

		/// @brief C-style array of objects
		T buffer[nb_elem];
	};

	//CTAD for vale::array: this works because of aggregate initialization
	template <class First, class... Rest>
	array(First, Rest...)->array<typename helpers::is_parameter_pack_of_same_type<First, Rest...>::type, 1 + sizeof...(Rest)>;

	template<typename T, size_t size, typename ThreadSafety>
	/// @brief writes the content of the array between '{}', separating the objects by ','.
	/// Will lock the mutex of the array if its thread safety policy is ThreadSafe.
	static std::ostream& operator<<(std::ostream& os, const array<T, size, ThreadSafety>& var)
	{
		var.print(os);
		return os;
	}
}