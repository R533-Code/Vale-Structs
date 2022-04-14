#pragma once
#include <vale_structs/common.h>
#include <vale_structs/array.h>

namespace vale
{
	template<typename First, typename... Rest>
	class variant
	{
		static_assert(helpers::is_pack_with_no_duplicates_v<First, Rest...>,
			"Parameter pack should contain no duplicates!");

		//We align a stack buffer of size the greatest size of all types passed as parameters.
		//This is to avoid any UB relating to alignment.
		alignas(helpers::get_max_size_of_type_pack_v<First, Rest...>)
			/// @brief The stack storage on which the active object lives
			char buffer[helpers::get_max_size_of_type_pack_v<First, Rest...>];

		/// @brief The current active type's index in the pack
		uint16_t type = 0;

	public:
		template<typename T>
		/// @brief Creates a variant storing an object of type 'T'
		/// @tparam T The type of the active object
		/// @param object The value of the new object to store
		variant(const T& object)
		{
			//The type should be part of the parameter pack of the variant
			static_assert(!helpers::is_type_not_in_pack_v<T, First, Rest...>,
				"Type isn't part of the template parameter pack of the variant!");
			construct<T>(object);
		}

		/// @brief Destroys the variant, destroying the active object
		~variant()
		{
			//destruct_active_linear<0, First, Rest...>();
			destruct_active_constant();
		}

		template<typename T>
		/// @brief Assigns a new value to the variant, destroying the old one
		/// @tparam T The type of the new object to store
		/// @param object The new object to store
		/// @return *this
		variant& operator=(const T& object)
		{
			static_assert(!helpers::is_type_not_in_pack_v<T, First, Rest...>,
				"Type isn't part of the template parameter pack of the variant!");
			destruct_active_linear<0, First, Rest...>(); //destroy the active object
			construct<T>(object); //constructs the new object

			return *this;
		}

		template<typename T>
		/// @brief Gets the value of the object of type T if its active, else throws
		/// @tparam T The type of the object to get
		/// @return Reference to the object, or throws a bad_variant_access
		T& get()
		{
			static_assert(!helpers::is_type_not_in_pack_v<T, First, Rest...>,
				"Type isn't part of the template parameter pack of the variant!");
			if (type == helpers::get_index_of_type_from_pack_v<T, First, Rest...>)
			{
				return *reinterpret_cast<T*>(buffer);
			}
			throw /*vale::bad_variant_access{}*/;
		}

		template<typename T>
		/// @brief Gets the value of the object of type T if its active, else throws
		/// @tparam T The type of the object to get
		/// @return const reference to the object, or throws a bad_variant_access
		const T& get() const
		{
			static_assert(!helpers::is_type_not_in_pack_v<T, First, Rest...>,
				"Type isn't part of the template parameter pack of the variant!");
			if (type == helpers::get_index_of_type_from_pack_v<T, First, Rest...>)
			{
				return *reinterpret_cast<const T*>(buffer);
			}
			throw /*vale::bad_variant_access{}*/;
		}

		/// @brief Returns the index of the current active type
		/// @return The active index, or max_index() + 1 to signify empty state
		uint64_t index() const noexcept { return type; }

		/// @brief Returns the maximum index that can be active.
		/// @return The number of types in the variant parameter pack
		constexpr uint64_t max_index() const noexcept { return sizeof...(Rest); }

	private:

		template<typename T, typename... Args>
		/// @brief Constructs an object of type 'T' in the buffer
		/// @tparam ...Args The parameter pack to forward to the constructor
		/// @tparam T The type to construct
		/// @param ...args The arguments to forward to the constructor
		void construct(Args&&... args)
		{
			type = helpers::get_index_of_type_from_pack_v<T, First, Rest...>;
			new(buffer) T(std::forward<Args>(args)...);
		}

		/*LINEAR-COMPLEXITY ALGORITHMS*/

		template<size_t index_t = 0, typename FirstT, typename... RestT>
		/// @brief Destructs the active type, using recursion, to check for the type to destroy.
		/// O(n) implementation of destruct_active().
		/// @tparam FirstT The first type
		/// @tparam ...RestT The rest of the pack
		void destruct_active_linear()
		{
			if (index_t == type)
				reinterpret_cast<const FirstT*>(buffer)->~FirstT(); //Call destructor
			else //We recurse, popping the First type from the pack, and incrementing the index
				destruct_active_linear<index_t + 1, RestT...>();
		}

		template<size_t index_t>
		/// @brief Overload of destruct_active for when there are no longer a type.
		/// As we have checked for all the types in the pack, this means that
		/// active type was not any of the types in the pack.
		void destruct_active_linear() {}

		/*CONSTANT-COMPLEXITY ALGORITHMS*/

		/// @brief Destroys the active object in constant time.
		/// This might be faster if there are no trivial types,
		/// or a lot of non-trivial type.
		inline void destruct_active_constant()
		{
			//We initialize an array of pointers to the destructor of each
			//type. The index is the destructor to call.
			static const vale::array dt
				= { &destruct_active_constant_delete<First>,
				&destruct_active_constant_delete<Rest>... };
			dt[type](buffer);
		}

		template<typename T>
		/// @brief Helper static method that calls the destructor of an object
		/// @tparam T The type to cast to, to call its destructor
		static void destruct_active_constant_delete(void* buffer)
		{
			reinterpret_cast<const T*>(buffer)->~T();
		}		

		/// @brief Helper method to print a variant's active content
		/// @param os The ostream in which to << the content
		inline void print_variant(std::ostream& os) const
		{
			//We initialize an array of pointers to the printing method of each
			//type. The index is the function to call.
			static const vale::array dt
				= { &print_variant_ptr<First>,
				&print_variant_ptr<Rest>... };
			dt[type](os, buffer);
		}

		template<typename T>
		/// @brief Helper static method that converts the buffer to the appropriate type and << in 'os'
		/// @tparam T The type to which to cast 'buffer'
		/// @param os The ostream in which to << the casted result
		/// @param buffer The buffer to cast
		static void print_variant_ptr(std::ostream& os, const void* buffer)
		{
			os << *reinterpret_cast<const T*>(buffer);
		}

		//To be able to access print_variant()
		friend std::ostream& operator<<(std::ostream& os, const variant& var);
	};

	template<typename First, typename... Rest>
	/// @brief writes the content of the active object in the variant to 'os'
	std::ostream& operator<<(std::ostream& os, const variant<First, Rest...>& var)
	{
		var.print_variant(os);
		return os;
	}
}