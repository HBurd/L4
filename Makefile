SRC = code/main.cpp code/hbmath.cpp code/hbrenderer.cpp code/hbtime.cpp code/hbentities.cpp code/hbship.cpp code/hbkeyboard.cpp code/hbplayer_control.cpp code/hbnet.cpp code/hbgui.cpp code/hbprojectile.cpp
SRC_PLATFORM = code/hbnet_unix.cpp
OBJ = hb_imgui.o hb_stb.o hb_tinyobjloader.o
default: L4game

clean:
	rm *.o
	rm L4game

L4game: hb_imgui.o hb_stb.o hb_tinyobjloader.o $(SRC) $(SRC_PLATFORM)
	clang++ $(SRC) $(SRC_PLATFORM) $(OBJ) -o L4game -ggdb3 -Iinclude -Llib -lSDL2 -lGLEW -ldl -lpthread -lsndio -lGL

hb_imgui.o:
	clang++ code/hb_imgui.cpp -c -o hb_imgui.o -Iinclude/SDL -Iinclude

hb_stb.o:
	clang++ code/hb_stb.cpp -c -o hb_stb.o -Iinclude

hb_tinyobjloader.o:
	clang++ code/hb_tinyobjloader.cpp -c -o hb_tinyobjloader.o -Iinclude
