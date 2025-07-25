cmake_minimum_required(VERSION 2.8)
project(sylar)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(cmake/utils.cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")
include_directories(.)

# include_directories(/apps/sylar/include)
# link_directories(/apps/sylar/lib)
# link_directories(/apps/sylar/lib64)
set(LIB_SRC
    sylar/log.cc
    sylar/util.cc
    sylar/config.cc
    sylar/thread.cc
    sylar/mutex.cc
)

add_library(sylar SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(sylar)
#target_link_libraries(sylar yaml-cpp)
#add_library(sylar_static STATIC ${LIB_SRC})
#set_target_properties(sylar_static PROPERTIES OUTPUT_NAME "sylar")

set(LIBS
    sylar
    pthread
    yaml-cpp
    )

add_executable(test tests/test.cc)
add_dependencies(test sylar)
target_link_libraries(test ${LIBS})

add_executable(test_config tests/test_config.cc)
add_dependencies(test_config sylar)
target_link_libraries(test_config ${LIBS})

add_executable(test_thread tests/test_thread.cc)
add_dependencies(test_thread sylar)
target_link_libraries(test_thread ${LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)