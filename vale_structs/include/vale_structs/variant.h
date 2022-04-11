#pragma once
#include <vale_structs/common.h>

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
			{
	};
}