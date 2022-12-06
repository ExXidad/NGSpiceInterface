cmake_minimum_required(VERSION 3.19)
project(ngspice_test)

set(CMAKE_CXX_STANDARD 17)

include_directories("/usr/local/include")
link_directories("/usr/local/lib")
#add_library(ngspice SHARED "/usr/local/lib/")
#
#set_target_properties(ngspice PROPERTIES LINKER_LANGUAGE CXX)

file(GLOB files *.cpp *.h)

add_executable(ngspice_test ${files})
target_link_libraries(ngspice_test PUBLIC ngspice)