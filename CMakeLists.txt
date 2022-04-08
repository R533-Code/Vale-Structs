cmake_minimum_required(VERSION 3.4)
# This project is for debugging structs
project(ValeStructs VERSION 0.0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Gets all files that ends with a '.cpp'
file(GLOB_RECURSE vale_structs_units "vale_structs/*.cpp")
# Gets all files that ends with a '.h'
file(GLOB_RECURSE vale_structs_headers "vale_structs/*.h")

# Compile an executable
add_executable(ValeStructsTest
	${vale_structs_units} ${vale_structs_headers}
)

# Configure 'config.h'
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/vale_structs/utils/config.h.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/vale_structs/utils/config.h")

# We add the 'include' directory to the preprocessor path
target_include_directories(ValeStructsTest PUBLIC 
	"vale_structs/include" "vale_structs/utils" "vendor/nanobench/src/include")

# Setup precompiled header
target_precompile_headers(ValeStructsTest PUBLIC 
	"$<$<COMPILE_LANGUAGE:CXX>:${PROJECT_SOURCE_DIR}/vale_structs/utils/comppch.h>"
)