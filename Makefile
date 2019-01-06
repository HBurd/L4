SRC_CLIENT = code/main.cpp
SRC_SERVER = code/server.cpp
SRC_SHARED = code/math.cpp code/renderer.cpp code/time.cpp code/entities.cpp code/ship.cpp code/keyboard.cpp code/player_control.cpp code/net.cpp code/gui.cpp code/projectile.cpp code/entity_update_step.cpp code/packets.cpp code/client.cpp
OBJ = impl_imgui.o impl_stb.o impl_tinyobjloader.o
default: L4game L4server

clean:
	rm *.o
	rm L4game
	rm L4server

L4game: $(OBJ) $(SRC_CLIENT) $(SRC_SHARED)
	clang++ $(SRC_CLIENT) $(SRC_SHARED) $(OBJ) -o L4game -ggdb3 -Iinclude -Icode -Llib -lSDL2 -lGLEW -ldl -lpthread -lsndio -lGL

L4server: $(OBJ) $(SRC_SERVER) $(SRC_SHARED)
	clang++ $(SRC_SERVER) $(SRC_SHARED) $(OBJ) -o L4server -ggdb3 -Iinclude -Icode -Llib -lSDL2 -lGLEW -ldl -lpthread -lsndio -lGL

impl_imgui.o:
	clang++ code/impl_imgui.cpp -c -o impl_imgui.o -Iinclude/SDL -Iinclude

impl_stb.o:
	clang++ code/impl_stb.cpp -c -o impl_stb.o -Iinclude

impl_tinyobjloader.o:
	clang++ code/impl_tinyobjloader.cpp -c -o impl_tinyobjloader.o -Iinclude
