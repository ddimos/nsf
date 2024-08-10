cmake_minimum_required(VERSION 3.15)

project("nsf"
    VERSION 0.1.0
    DESCRIPTION "Network SFML"
    )

# find_package(SFML 2.6 COMPONENTS network REQUIRED)
include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

option(NSF_BUILD_EXAMPLE "Build the nsf example" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NSF_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
    PRIVATE
        src/channel/ChannelManager.cpp
        src/channel/MessageHeader.cpp
        src/channel/Peer.cpp
        src/connection/Connection.cpp
        src/connection/ConnectionManager.cpp
        src/connection/PacketHeader.cpp
        src/connection/UdpSocket.cpp
        src/packetManager/PacketManager.cpp
        src/NSFImpl.cpp
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
    include/nsf/NetworkAddress.hpp
    include/nsf/NetworkMessage.hpp
    include/nsf/NetworkPlayer.hpp
    include/nsf/NSF.hpp
    include/nsf/Types.hpp
)

if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wextra -pedantic-errors)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Installing)

# https://decovar.dev/blog/2021/03/08/cmake-cpp-library/