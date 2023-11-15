set(CMAKE_CXX_STANDARD 17)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_subdirectory(External/OctaneGUI)

set(SOURCE Main.cpp)

add_executable(
    LevelSketch
    ${SOURCE}
)

target_include_directories(
    LevelSketch
    PRIVATE External
)

target_link_libraries(
    LevelSketch
    OctaneGUI
)
