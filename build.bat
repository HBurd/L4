IF NOT DEFINED BUILDENV (CALL "%VCINSTALLDIR%\Auxiliary\Build\vcvarsall.bat" amd64)
SET BUILDENV=Y

CALL cl code\client_main.cpp code\impl_imgui.cpp code\impl_stb.cpp code\impl_tinyobjloader.cpp lib\SDL2.lib opengl32.lib lib\glew32.lib Ws2_32.lib -FeL4client -Zi -EHsc -std:c++14 -Iinclude -Iinclude\SDL\windows -Iinclude\SDL\windows\SDL -Icode -DFAST_BUILD -link -SUBSYSTEM:CONSOLE