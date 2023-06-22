set (NSFML_EXAMPLE_SOURCE_FILES

	main.cpp
)

add_executable(nsfmlExample ${NSFML_EXAMPLE_SOURCE_FILES})
target_include_directories(nsfmlExample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nsfmlExample PUBLIC NSFML sfml-graphics sfml-window sfml-network)
set_target_properties(nsfmlExample PROPERTIES
	CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)
