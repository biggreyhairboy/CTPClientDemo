cmake_minimum_required(VERSION 3.5)
project(CTPClientDemo)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_VERBOSE_MAKEFILE ON)
#add_subdirectory(ctpapi_linux64)



set(SOURCE_FILES main.cpp MarketDataHandle.cpp MarketDataHandle.h)
#add_library(TCPClientDemo ${SOURCE_FILES})
#include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}}/ctpapi_linux64)
#link_directories(${PROJECT_SOURCE_DIR}/ctpapi_linux64/)
#find_library(/home/biggreyhairboy/ClionProjects/CTPClientDemo/ctpapi_linux64 thostmduserapi.so )
add_library(thostmduserapi SHARED IMPORTED)
SET_TARGET_PROPERTIES(thostmduserapi PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/ctpapi_linux64/thostmduserapi.so)
add_executable(CTPClientDemo ${SOURCE_FILES})
target_link_libraries( CTPClientDemo thostmduserapi)
