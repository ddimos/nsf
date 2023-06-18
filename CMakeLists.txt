cmake_minimum_required(VERSION 3.0.0)
project(NSFML VERSION 0.1.0)

# Add SFML 
find_package(SFML 2.5 COMPONENTS graphics network system REQUIRED)

file(GLOB TARGET_SRC "./src/*.cpp" )

add_executable(main ${TARGET_SRC})

target_link_libraries(main sfml-graphics sfml-system sfml-network)

target_include_directories(main PUBLIC
                            "${PROJECT_BINARY_DIR}"
                            "${PROJECT_SOURCE_DIR}/include/NSFML"
                            "${PROJECT_SOURCE_DIR}/src/NSFML"
                            )