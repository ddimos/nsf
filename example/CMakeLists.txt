set (NSF_EXAMPLE_SOURCE_FILES

	main.cpp
)

add_executable(nsfExample ${NSF_EXAMPLE_SOURCE_FILES})
target_include_directories(nsfExample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nsfExample PUBLIC nsf sfml-graphics sfml-window sfml-network)
set_target_properties(nsfExample PROPERTIES
	CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)
