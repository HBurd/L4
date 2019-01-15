cmake_minimum_required (VERSION 3.0)

project (L4)
set(CMAKE_BUILD_TYPE Debug)
include_directories (include code)

if (WIN32)
    include_directories (include/SDL/windows include/SDL/windows/SDL)
else ()
    include_directories (include/SDL/linux include/SDL/linux/SDL)
endif ()
    

link_directories (L4 ${PROJECT_SOURCE_DIR}/lib)

add_library (StbImage code/impl_stb.cpp)
add_library (ImGui code/impl_imgui.cpp)
add_library (TinyObj code/impl_tinyobjloader.cpp)

add_executable (L4client code/client_main.cpp)

target_link_libraries (L4client SDL2 SDL2main StbImage ImGui TinyObj)

if (WIN32)
    target_link_libraries(L4client opengl32 glew32 Ws2_32)
else ()
    target_link_libraries(L4client GL GLEW sndio dl pthread)
endif ()

add_definitions (-DFAST_BUILD=Y)

add_custom_command(TARGET L4client POST_BUILD      # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy               # which executes "cmake - E copy_if_different..."
        "${PROJECT_SOURCE_DIR}/lib/SDL2.dll"       # <--this is in-file
        $<TARGET_FILE_DIR:L4client>)               # <--this is out-file path

add_custom_command(TARGET L4client POST_BUILD      # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy               # which executes "cmake - E copy_if_different..."
        "${PROJECT_SOURCE_DIR}/lib/glew32.dll"     # <--this is in-file
        $<TARGET_FILE_DIR:L4client>)               # <--this is out-file path

add_custom_command(TARGET L4client POST_BUILD      # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_directory     # which executes "cmake - E copy_if_different..."
        "${PROJECT_SOURCE_DIR}/resources"          # <--this is in-directory containing files
        ${PROJECT_BINARY_DIR}/resources)          # <--this is out-file path
