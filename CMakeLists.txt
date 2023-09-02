cmake_minimum_required(VERSION 3.22)
project(suffixtree CXX)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(
		-pedantic -Wall -Wextra
		-Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization
		-Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations
		-Wmissing-include-dirs -Wold-style-cast -Wredundant-decls -Wshadow
		-Wsign-conversion -Wsign-promo -Wstrict-null-sentinel
		-Wstrict-overflow -Wsuggest-override -Wswitch-default -Wundef

		#-Werror
	)
else()
	message(WARNING "Unknown compiler - not turning on warnings")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



add_library(suffixtree SHARED
	src/Node.cpp
	src/SuffixTree.cpp
)

target_include_directories(suffixtree PUBLIC
	include/
)


add_executable(test
	src/test.cpp
)

target_link_libraries(test
	suffixtree
)

