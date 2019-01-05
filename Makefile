SRC_CLIENT = code/main.cpp
SRC_SERVER = code/server.cpp
SRC_SHARED = code/hbmath.cpp code/hbrenderer.cpp code/hbtime.cpp code/hbentities.cpp code/hbship.cpp code/hbkeyboard.cpp code/hbplayer_control.cpp code/hbnet.cpp code/hbgui.cpp code/hbprojectile.cpp code/hbentity_update_step.cpp code/hbpackets.cpp code/hbclient.cpp
OBJ = hb_imgui.o hb_stb.o hb_tinyobjloader.o
default: L4game L4server

clean:
	rm *.o
	rm L4game
	rm L4server

L4game: $(OBJ) $(SRC_CLIENT) $(SRC_SHARED)
	clang++ $(SRC_CLIENT) $(SRC_SHARED) $(OBJ) -o L4game -ggdb3 -Iinclude -Llib -lSDL2 -lGLEW -ldl -lpthread -lsndio -lGL

L4server: $(OBJ) $(SRC_SERVER) $(SRC_SHARED)
	clang++ $(SRC_SERVER) $(SRC_SHARED) $(OBJ) -o L4server -ggdb3 -Iinclude -Llib -lSDL2 -lGLEW -ldl -lpthread -lsndio -lGL

hb_imgui.o:
	clang++ code/hb_imgui.cpp -c -o hb_imgui.o -Iinclude/SDL -Iinclude

hb_stb.o:
	clang++ code/hb_stb.cpp -c -o hb_stb.o -Iinclude

hb_tinyobjloader.o:
	clang++ code/hb_tinyobjloader.cpp -c -o hb_tinyobjloader.o -Iinclude
