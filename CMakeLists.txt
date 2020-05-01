cmake_minimum_required(VERSION 3.0.0)
  
project(ffmpeg-cpp CXX)
  
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
  
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE OFF)

if (CMAKE_SYSTEM MATCHES Linux)
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -DDEBUG -DLinux")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

file(GLOB_RECURSE INC source/*.h)
file(GLOB_RECURSE SRC source/*.cpp)

add_library(${PROJECT_NAME} ${SRC}) 


target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/source/ffmpeg-cpp )

if (UNIX)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/source/ffmpeg-cpp/ffmpeg-cpp )
endif()

target_link_libraries(${PROJECT_NAME} ffmpeg vpx va va-drm va-glx)

