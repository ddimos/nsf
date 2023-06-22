cmake_minimum_required(VERSION 3.15)

project("NSFML"
    VERSION 0.1.0
    DESCRIPTION "Network SFML"
    )

find_package(SFML 2.5 COMPONENTS network REQUIRED)

option(NSFML_BUILD_EXAMPLE "Build the NSFML example" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NSFML_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
    PRIVATE
        src/Connection.cpp
        src/Network.cpp
        src/Peer.cpp
        src/Transport.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC sfml-network)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set(public_headers
    include/NSFML/Network.hpp
    include/NSFML/Connection.hpp
    include/NSFML/InternalPacketType.hpp
    include/NSFML/NetworkAddress.hpp
    include/NSFML/NetworkEvent.hpp
    include/NSFML/Network.hpp
    include/NSFML/NetworkMessage.hpp
    include/NSFML/NetworkPlayer.hpp
    include/NSFML/NetworkUtils.hpp
    include/NSFML/PacketHeader.hpp
    include/NSFML/Peer.hpp
    include/NSFML/Transport.hpp
)

if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${PROJECT_NAME} PRIVATE -Werror)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Installing)

# https://decovar.dev/blog/2021/03/08/cmake-cpp-library/