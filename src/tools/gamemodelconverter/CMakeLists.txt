set(CMAKE_CXX_STANDARD 17)

set(GAMEMODELCONVERTER_SOURCES
        src/commands.cpp
        src/commands.h
        src/log.cpp
        src/log.h
        src/main.cpp
        src/processor.cpp
        src/processor.h
)

set(GAMEMODELCONVERTER_INCLUDES src)

add_executable(gamemodelconverter ${GAMEMODELCONVERTER_SOURCES})
target_include_directories(gamemodelconverter PRIVATE ${GAMEMODELCONVERTER_INCLUDES})
target_link_libraries(gamemodelconverter PRIVATE thyme_lib)
