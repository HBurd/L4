cmake_minimum_required (VERSION 3.0)

project (L4)
include_directories (include include/SDL/windows include/SDL/windows/SDL code)

link_directories (L4 ${PROJECT_SOURCE_DIR}/lib)

add_library (StbImage code/impl_stb.cpp)
add_library (ImGui code/impl_imgui.cpp)
add_library (TinyObj code/impl_tinyobjloader.cpp)

add_executable (L4client code/client_main.cpp)

target_link_libraries (L4client SDL2 SDL2main glew32 opengl32 StbImage ImGui TinyObj Ws2_32)
add_compile_definitions (FAST_BUILD=Y)

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