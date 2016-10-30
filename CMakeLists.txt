cmake_minimum_required(VERSION 3.5)
project(cl4ptp)

### sources
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(LIBRARIES lib)
set(SOURCES
        src/main.cpp
        src/LongPoll.cpp
        )
include_directories(
        ${LIBRARIES}/tgbot-cpp/include
        /usr/local/include
)

### libs
# threads
find_package(Threads REQUIRED)

# openssl
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# boost
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS system iostreams unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

### building project
add_subdirectory(${LIBRARIES}/tgbot-cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(
        ${PROJECT_NAME}
        TgBot
        ${Boost_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
)