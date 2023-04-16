link_libraries(${PROJECT_NAME})

set(prg filtering_audio)
include_directories(../../sources)
add_executable(${prg} ${prg}.cpp)
add_dependencies(${prg} ${PROJECT_NAME})
