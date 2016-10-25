cmake_minimum_required(VERSION 3.6)
project(cl4ptp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(Boost_USE_MULTITHREADED ON)
set(LIBRARIES lib/)
set(SOURCES src/main.cpp)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system iostreams unit_test_framework REQUIRED)

include_directories(/usr/local/include lib/ ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIR})

add_executable(cl4ptp ${SOURCES})

target_link_libraries(cl4ptp
        ${CMAKE_SOURCE_DIR}/${LIBRARIES}libTgBot.a
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${OPENSSL_LIBRARIES}
        )