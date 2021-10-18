cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(Telnet LANGUAGES CXX ASM)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_executable(${PROJECT_NAME} server.cpp server.hpp main.cpp spawn.hpp asio_ns.hpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
find_package(Boost REQUIRED COMPONENTS context chrono system container log thread )
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
if(Boost_FOUND)
  target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} PRIVATE Boost::context Boost::chrono Boost::system Boost::container Boost::log Boost::thread)
endif(Boost_FOUND)
