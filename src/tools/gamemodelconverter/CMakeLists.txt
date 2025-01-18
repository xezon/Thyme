set(CMAKE_CXX_STANDARD 17)

set(GAMEMODELCONVERTER_SOURCES
        src/gamemodelcommon.h
        src/gamemodelfile.cpp
        src/gamemodelfile.h
        src/commands.cpp
        src/commands.h
        src/log.cpp
        src/log.h
        src/main.cpp
        src/processor.cpp
        src/processor.h
)

set(GAMEMODELCONVERTER_INCLUDES src)

add_executable(GameModelConverter ${GAMEMODELCONVERTER_SOURCES})
target_include_directories(GameModelConverter PRIVATE ${GAMEMODELCONVERTER_INCLUDES})
target_link_libraries(GameModelConverter PRIVATE thyme_lib)
