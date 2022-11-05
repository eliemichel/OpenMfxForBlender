add_subdirectory(c)
add_subdirectory(cpp)

if (OPENMFX_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif ()
