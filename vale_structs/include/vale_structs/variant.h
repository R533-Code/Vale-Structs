#pragma once
#include <vale_structs/common.h>
#include <vale_structs/array.h>

namespace vale
{
	/// @brief Thrown if variant.get() is called on an inactive type
	class bad_variant_access
		: public std::exception
	{
	public:
		const char* what() const noexcept override { return "The type was not active!"; }
	};

	/// @brief Thrown if the variant is << to an ostream if it's in an invalid state
	class invalid_variant_access
		: public std::exception
	{
	public:
		const char* what() const noexcept override { return "The variant was in an invalid state!"; }
	};

	template<typename DestructionPolicy, typename ThreadSafety, typename First, typename... Rest>
	/// @brief Unspecialized variant_impl, if the ThreadSafety is not a valid type
	/// @tparam DestructionPolicy The variant's destruction complexity policy
	/// @tparam ThreadSafety The thread safety policy of the variant
	class variant_impl
	{
		static_assert(helpers::is_thread_safety_policy_v<ThreadSafety>,
			"ThreadSafety can only be [Non]ThreadSafe");
	};

	template<typename DestructionPolicy, typename First, typename... Rest>
	/// @brief Non-thread safe variant overload
	/// @tparam DestructionPolicy The variant's destruction complexity policy
	class variant_impl<DestructionPolicy, NonThreadSafe, First, Rest...>
	{
		static_assert(helpers::is_variant_destructor_policy_v<DestructionPolicy>,
			"DestructionPolicy can only be [Auto|Linear|Constant]ComplexityDestruct");

		static_assert(helpers::is_pack_with_no_duplicates_v<First, Rest...>,
			"Parameter pack should contain no duplicates!");

		//We align a stack buffer of size the greatest size of all types passed as parameters.
		//This is to avoid any UB relating to alignment.
		alignas(alignof(helpers::get_type_of_max_size_t<First, Rest...>))
			/// @brief The stack storage on which the active object lives
			char buffer[helpers::get_max_size_of_type_pack_v<First, Rest...>];

		/// @brief The current active type's index in the pack
		uint16_t type = 0;

	public:

		/******************************************
		STATIC HELPERS
		******************************************/

		/// @brief Returns the maximum index that can be active.
		/// @return The number of types in the variant parameter pack
		static constexpr uint64_t max_active_index() noexcept { return sizeof...(Rest); }

		/// @brief Returns the index representing an invalid state
		/// @return max_active_index() + 1
		static constexpr size_t invalid_index() noexcept { return sizeof...(Rest) + 1; }

		/// @brief Check if the variant can be in an invalid state
		/// @return True if all the types are fundamental
		static constexpr bool can_be_invalid() noexcept
		{
			return sizeof...(Rest) + 1 != helpers::count_fundamental_v<First, Rest...>;
		}

		/// @brief Returns the alignment of the type with the highest size
		/// @return size_t representing the alignment
		static constexpr size_t alignment() noexcept { return alignof(helpers::get_type_of_max_size_t<First, Rest...>); }

		/// @brief 
		/// @return 
		static constexpr size_t buffer_byte_size() noexcept { return helpers::get_max_size_of_type_pack_v<First, Rest...>; }

		/// @brief Check the complexity of the algorithm used for destructing active object
		/// This is the algorithm that decides if the destruction algorithm has a complexity of O(1) or O(n).
		static constexpr algorithm destructor_complexity() noexcept
		{
			//If AutoComplexityDestruct, use algorithm to choose complexity
			if constexpr (std::is_same_v<DestructionPolicy, AutoComplexityDestruct>)
			{
				// if more than 9/10 of the types are not fundamental, use the constant time destruction algorithm
				// As we are using integer division, if there are less than 10 types, linear_complexity will be choosed
				if constexpr (helpers::count_non_fundamental_v<First, Rest...> > ((sizeof...(Rest) + 1) * 9) / 10)
					return algorithm::constant_complexity;
				else
					return algorithm::linear_complexity;
			}
			else //If not AutoComplexityDestruct, choose the according policy
			{
				if constexpr (std::is_same_v<DestructionPolicy, ConstantComplexityDestruct>)
					return algorithm::constant_complexity;
				else
					return algorithm::linear_complexity;
			}
		}

		/// @brief Check if the variant's destructor is noexcept (all the types' destructors are noexcept)
		/// @return True if the variant's destructor is noexcept
		static constexpr bool is_noexcept_destructible() noexcept
		{
			return std::conjunction_v<std::is_nothrow_destructible<First>, std::is_nothrow_destructible<Rest>...>;
		}

		/// @brief Check if a variant is copyable
		/// @return True if all the possible type that can be stored by the variant can be copy constructed
		static constexpr bool is_copyable() noexcept
		{
			return std::conjunction_v<std::is_copy_constructible<First>, std::is_copy_constructible<Rest>...>;
		}

		/// @brief Check if a variant is noexcept copyable
		/// @return True if all the possible type that can be stored by the variant can be noexcept copy constructed
		static constexpr bool is_noexcept_copyable() noexcept
		{
			return std::conjunction_v<std::is_nothrow_copy_constructible<First>, std::is_nothrow_copy_constructible<Rest>...>;
		}

		/// @brief Check if a variant is movable
		/// @return True if all the possible type that can be stored by the variant can be move constructed
		static constexpr bool is_movable() noexcept
		{
			return std::conjunction_v<std::is_move_constructible<First>, std::is_move_constructible<Rest>...>;
		}

		/// @brief Check if a variant is movable
		/// @return True if all the possible type that can be stored by the variant can be noexcept move constructed
		static constexpr bool is_noexcept_movable() noexcept
		{
			return std::conjunction_v<std::is_nothrow_move_constructible<First>, std::is_nothrow_move_constructible<Rest>...>;
		}

		/******************************************
		METHODS AND CONSTRUCTORS
		******************************************/

		/// @brief Default construct the first type 
		variant_impl() noexcept(std::is_nothrow_default_constructible_v<First>)
		{
			construct<First>(); //Default construct the first type
		}

		template<typename T>
		/// @brief Creates a variant storing an object of type 'T'
		/// @tparam T The type of the active object
		/// @param object The value of the new object to store
		variant_impl(const T& object) noexcept(std::is_nothrow_copy_constructible_v<T>)
		{
			//The type should be part of the parameter pack of the variant
			static_assert(!helpers::is_type_not_in_pack_v<T, First, Rest...>,
				"Type isn't part of the template parameter pack of the variant!");
			construct<T>(object);
		}

		/// @brief Destroys the variant, destroying the active object
		~variant_impl() noexcept(is_noexcept_destructible())
		{
			destruct_active();
		}

		template<typename T>
		/// @brief Assigns a new value to the variant, destroying the old one
		/// @tparam T The type of the new object to store
		/// @param object The new object to store
		/// @return *this
		variant_impl& operator=(const T& object)
		{
			static_assert(!helpers::is_type_not_in_pack_v<T, First, Rest...>,
				"Type isn't part of the template parameter pack of the variant!");
			destruct_active();
			construct<T>(object); //constructs the new object

			return *this;
		}

		template<typename T>
		/// @brief Gets the value of the object of type T if its active, else throws
		/// @tparam T The type of the object to get
		/// @return Reference to the object, or throws a bad_variant_access
		[[nodiscard]] T& get()
		{
			static_assert(!helpers::is_type_not_in_pack_v<T, First, Rest...>,
				"Type isn't part of the template parameter pack of the variant!");
			if (type == helpers::get_index_of_type_from_pack_v<T, First, Rest...>)
			{
				return *reinterpret_cast<T*>(buffer);
			}
			throw vale::bad_variant_access{};
		}

		template<typename T>
		/// @brief Gets the value of the object of type T if its active, else throws
		/// @tparam T The type of the object to get
		/// @return const reference to the object, or throws a bad_variant_access
		[[nodiscard]] const T& get() const
		{
			static_assert(!helpers::is_type_not_in_pack_v<T, First, Rest...>,
				"Type isn't part of the template parameter pack of the variant!");
			if (type == helpers::get_index_of_type_from_pack_v<T, First, Rest...>)
			{
				return *reinterpret_cast<const T*>(buffer);
			}
			throw vale::bad_variant_access{};
		}

		/// @brief Returns the index of the current active type
		/// @return The active index, or max_index() + 1 to signify invalid state
		[[nodiscard]] uint64_t index() const noexcept { return type; }

		/// @brief Checks if the variant is in a valid state
		/// @return true if the variant is valid, or false
		[[nodiscard]] bool is_valid() const noexcept { return type != sizeof...(Rest) + 1; }

		/// @brief Helper method to print a variant's active content
		/// @param os The ostream in which to << the content
		inline void print(std::ostream& os) const
		{
			//We initialize an array of pointers to the printing method of each
			//type. The index is the function to call.
			static const vale::array dt
				= { &print_variant_ptr<First>,
				&print_variant_ptr<Rest>... };

			if constexpr (can_be_invalid())
			{
				if (is_valid())
					return dt[type](os, buffer); //We return void
				throw vale::invalid_variant_access{};
			}
			else //As the variant cannot be invalid, we can safely call the function
				return dt[type](os, buffer);
		}

		template<typename T, typename... Args>
		/// @brief Constructs an object directly, after destroying the active one
		/// @tparam T The type to construct
		inline void emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
		{
			static_assert(!helpers::is_type_not_in_pack_v<T, First, Rest...>,
				"Type isn't part of the template parameter pack of the variant!");
			destruct_active();
			construct<T>(std::forward<Args>(args)...);
		}

	private:

		template<typename T, typename... Args>
		/// @brief Constructs an object of type 'T' in the buffer
		/// Is noexcept if the variant cannot be invalid.
		/// @tparam ...Args The parameter pack to forward to the constructor
		/// @tparam T The type to construct
		/// @param ...args The arguments to forward to the constructor
		void construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
		{
			type = helpers::get_index_of_type_from_pack_v<T, First, Rest...>;

			//If the constructor can't throw, do not check try/catch exceptions
			if constexpr (std::is_nothrow_constructible_v<T, Args...>)
			{
				new(buffer) T(std::forward<Args>(args)...);
			}
			else
			{
				try
				{
					new(buffer) T(std::forward<Args>(args)...);
				}
				catch (...) //The constructor throwed an error
				{
					set_invalid_state();
					throw; //Rethrow the error from the constructor
				}
			}
		}

		/// @brief Destruct the active object if the variant is valid.
		/// Chooses the best algorithm for destroying.
		inline void destruct_active() noexcept(is_noexcept_destructible())
		{
			if constexpr (std::conjunction_v<std::is_fundamental<First>, std::is_fundamental<Rest>...>)
			{
				//If all the types are fundamental, we shouldn't bother doing anything
			}
			else
			{
				//we only want to check if the variant is valid if it can be invalid
				if constexpr (can_be_invalid())
				{
					if (is_valid())
					{
						if constexpr (destructor_complexity() == algorithm::constant_complexity)
							impl_destruct_active_constant();
						else
							impl_destruct_active_linear<0, First, Rest...>();
					}
					//There is no need to do anything if the variant is not valid
				}
				else
				{
					// if more than 9/10 of the types are not fundamental, use the constant time destruction algortihm
					if constexpr (destructor_complexity() == algorithm::constant_complexity)
						impl_destruct_active_constant();
					else
						impl_destruct_active_linear<0, First, Rest...>();
				}
			}
		}

		/// @brief Sets the variant to an invalid state			
		inline void set_invalid_state() noexcept
		{
			type = sizeof...(Rest) + 1;
		}

		/******************************************
		LINEAR COMPLEXITY ALGORITHM
		******************************************/

		template<size_t index_t = 0, typename FirstT, typename... RestT>
		/// @brief Destructs the active type, using recursion, to check for the type to destroy.
		/// O(n) implementation of destruct_active().
		/// @tparam FirstT The first type
		/// @tparam ...RestT The rest of the pack
		void impl_destruct_active_linear() noexcept(is_noexcept_destructible())
		{
			if (index_t == type)
				reinterpret_cast<const FirstT*>(buffer)->~FirstT(); //Call destructor
			else //We recurse, popping the First type from the pack, and incrementing the index
				impl_destruct_active_linear<index_t + 1, RestT...>();
		}

		template<size_t index_t>
		/// @brief Overload of destruct_active for when there are no longer a type.
		/// As we have checked for all the types in the pack, this means that
		/// active type was not any of the types in the pack.
		void impl_destruct_active_linear() noexcept {}

		/******************************************
		CONSTANT COMPLEXITY ALGORITHM
		******************************************/

		/// @brief Destroys the active object in constant time.
		/// This might be faster if there are no trivial types,
		/// or a lot of non-trivial type. SHOULD NOT BE CALLED IF THE VARIANT IS INVALID
		inline void impl_destruct_active_constant() noexcept(is_noexcept_destructible())
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
		static void destruct_active_constant_delete(void* buffer) noexcept(is_noexcept_destructible())
		{
			reinterpret_cast<const T*>(buffer)->~T();
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
	};

	template<typename DestructionPolicy, typename First, typename... Rest>
	class variant_impl<DestructionPolicy, ThreadSafe, First, Rest...>
	{

	};

	template<typename First, typename... Rest>
	/// @brief writes the content of the active object in the variant to 'os'
	static std::ostream& operator<<(std::ostream& os, const variant_impl<First, Rest...>& var)
	{
		var.print(os);
		return os;
	}

	template<typename First, typename... Rest>
	/// @brief Typedef for variant, whose destructor's complexity is automatically chosen
	using variant = variant_impl<AutoComplexityDestruct, NonThreadSafe, First, Rest...>;

	template<typename First, typename... Rest>
	/// @brief Typedef for variant, whose destructor's complexity is automatically chosen
	using ts_variant = variant_impl<AutoComplexityDestruct, ThreadSafe, First, Rest...>;
}