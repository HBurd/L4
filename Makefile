SRC_CLIENT = code/client_main.cpp
SRC_SERVER = code/server_main.cpp
OBJ = impl_imgui.o impl_stb.o impl_tinyobjloader.o
default: L4client L4server

clean:
	rm *.o
	rm L4client
	rm L4server

L4client: $(OBJ) $(SRC_CLIENT)
	clang++ $(SRC_CLIENT) $(OBJ) -o L4client -ggdb3 -Iinclude -Icode -Llib -lSDL2 -lGLEW -ldl -lpthread -lsndio -lGL -DFAST_BUILD

L4server: $(OBJ) $(SRC_SERVER)
	clang++ $(SRC_SERVER) $(OBJ) -o L4server -ggdb3 -Iinclude -Icode -Llib -lSDL2 -lGLEW -ldl -lpthread -lsndio -lGL -DFAST_BUILD

impl_imgui.o:
	clang++ code/impl_imgui.cpp -c -o impl_imgui.o -Iinclude/SDL -Iinclude

impl_stb.o:
	clang++ code/impl_stb.cpp -c -o impl_stb.o -Iinclude

impl_tinyobjloader.o:
	clang++ code/impl_tinyobjloader.cpp -c -o impl_tinyobjloader.o -Iinclude
