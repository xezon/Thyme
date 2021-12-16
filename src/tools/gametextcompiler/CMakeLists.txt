set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE GAMETEXTCOMPILER_SOURCES src/*.cpp src/*.h)
set(GAMETEXTCOMPILER_INCLUDES src)

add_definitions(-DHAVE_STD_CLAMP)
add_executable(gametextcompiler ${GAMETEXTCOMPILER_SOURCES})
target_include_directories(gametextcompiler PRIVATE ${GAMETEXTCOMPILER_INCLUDES})
target_link_libraries(gametextcompiler PRIVATE thyme_lib)
