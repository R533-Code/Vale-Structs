#pragma once
#include <vale_structs/common.h>

namespace vale
{
	//Unspecialized array, which defaults to a non-thread safe array
	template<typename T, size_t nb_elem, typename ThreadSafety = NonThreadSafe>
	class array { static_assert(is_thread_safety_policy_v<ThreadSafety>, "ThreadSafety can only be ThreadSafe/NonThreadSafe"); };

	template<typename T, size_t nb_elem>
	//A Thread Safe fixed-size array
	class array<T, nb_elem, ThreadSafe>
	{
		static_assert(nb_elem != 0, "Array size should be greater than 0!");

	public:
		template<typename... Args>
		//Clears the object stored and replaces them by calling the constructor
		//and forwarding the arguments to it.
		constexpr void fill(Args&&... args)
		{
			std::scoped_lock lock(mutex);
			for (size_t i = 0; i < nb_elem; i++)
				buffer[i] = obj;			
		}

		//Returns the object at 'index'. 
		//If the index is greater than nb_elem - 1, throws std::out_of_range
		constexpr T& operator[](size_t index)
		{
			std::scoped_lock lock(mutex);
			if (index < nb_elem)
				return buffer[index];
			throw std::out_of_range("vale::array: index was greater than size!");
		}

		//Returns the object at 'index'. 
		//If the index is greater than nb_elem - 1, throws std::out_of_range
		constexpr const T& operator[](size_t index) const
		{
			std::scoped_lock lock(mutex);
			if (index < nb_elem)
				return buffer[index];
			throw std::out_of_range("vale::array: index was greater than size!");
		}

		//Returns the last object in the array
		constexpr const T& back() const
		{
			std::scoped_lock lock(mutex);			
			return buffer[nb_elem - 1];
		}

		//Returns the last object
		constexpr T& back()
		{
			std::scoped_lock lock(mutex);
			return buffer[nb_elem - 1];
		}

		//Returns the first object in the array
		constexpr const T& front() const
		{
			std::scoped_lock lock(mutex);
			return buffer[0];
		}

		//Returns the first object
		constexpr T& front()
		{
			std::scoped_lock lock{ mutex };
			return buffer[0];
		}

		//Access the object at 'index' and passes it to a functor that accepts
		//a 'const T&'. Returns true if the object is passed to the function.
		constexpr bool access_index(size_t index, void(*func)(const T&)) const
		{
			std::scoped_lock lock{mutex};
			if (index < nb_elem)
			{
				func(buffer[index]);
				return true;
			}
			return false;
		}

		//Access the object at 'index' and passes it to a functor that accepts
		//a 'const T&'. Returns true if the object is passed to the function.
		constexpr bool access_index(size_t index, void(*func)(T&))
		{
			std::scoped_lock lock{ mutex };
			if (index < nb_elem)
			{
				func(buffer[index]);
				return true;
			}
			return false;
		}

		constexpr void for_each(void(*func)(T&))
		{
			std::scoped_lock lock{ mutex };
			for (size_t i = 0; i < nb_elem; i++)
				func(buffer[i]);
		}

		constexpr void for_each(void(*func)(const T&)) const
		{
			std::scoped_lock lock{ mutex };
			for (size_t i = 0; i < nb_elem; i++)
				func(buffer[i]);
		}

		constexpr size_t size() const { return nb_elem; }

		//Returns a pointer to the beginning of the data. NON-THREAD SAFE
		constexpr const T* data() const { return buffer; }
		
		//Returns a pointer to the beginning of the data. NON-THREAD SAFE
		constexpr T* data() { return buffer; }

	public: //MEMBERS

		T buffer[nb_elem];
		
		mutable std::mutex mutex{};
	};

	template<typename T, size_t nb_elem>
	//Non Thread Safe fixed-size array
	class array<T, nb_elem, NonThreadSafe>
	{
		static_assert(nb_elem != 0, "Array size should be greater than 0!");

	public:		
		constexpr void fill(const T& obj)
		{
			for (size_t i = 0; i < nb_elem; i++)
				buffer[i] = obj;
		}

		//Returns the object at 'index'. 
		//If the index is greater than nb_elem - 1, throws std::out_of_range
		constexpr T& operator[](size_t index)
		{
			if (index < nb_elem)
				return buffer[index];
			throw std::out_of_range("vale::array: index was greater than size!");
		}

		//Returns the object at 'index'. 
		//If the index is greater than nb_elem - 1, throws std::out_of_range
		constexpr const T& operator[](size_t index) const
		{
			if (index < nb_elem)
				return buffer[index];
			throw std::out_of_range("vale::array: index was greater than size!");
		}

		//Returns the last object in the array
		constexpr const T& back() const { return buffer[nb_elem - 1]; }

		//Returns the last object
		constexpr T& back() { return buffer[nb_elem - 1]; }

		//Returns the first object in the array
		constexpr const T& front() const { return buffer[0]; }

		//Returns the first object
		constexpr T& front() { return buffer[0]; }

		constexpr size_t size() const { return nb_elem; }

		//Returns a pointer to the beginning of the data. NON-THREAD SAFE
		constexpr const T* data() const { return buffer; }

		//Returns a pointer to the beginning of the data. NON-THREAD SAFE
		constexpr T* data() { return buffer; }

	public: //MEMBERS

		T buffer[nb_elem];
	};

	template <class First, class... Rest>
	array(First, Rest...)->array<typename is_parameter_pack_of_same_type<First, Rest...>::type, 1 + sizeof...(Rest)>;

	template<typename T, size_t size, typename ThreadSafety>
	std::ostream& operator<<(std::ostream& os, const array<T, size, ThreadSafety>& var)
	{
		if constexpr (std::is_same_v<ThreadSafety, ThreadSafe>)
		{
			std::scoped_lock lock(var.mutex); //Lock the array
		}
		os << "{";
		for (size_t i = 0; i < size - 1; i++)
			os << *(var.data() + i) << ", ";
		os << *(var.data() + size - 1) << '}';
		return os;
	}
}