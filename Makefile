SRC = code/main.cpp code/hbmath.cpp code/hbrenderer.cpp code/hbtime.cpp code/hbentities.cpp code/hbship.cpp
OBJ = hb_imgui.o hb_stb.o hb_tinyobjloader.o
L4game: hb_imgui.o hb_stb.o hb_tinyobjloader.o $(SRC)
	clang++ $(SRC) $(OBJ) -o L4game -ggdb3 -Iinclude -Llib -lSDL2 -lGLEW -ldl -lpthread -lsndio -lGL
	cp resources/* .

hb_imgui.o:
	clang++ code/hb_imgui.cpp -c -o hb_imgui.o -Iinclude/SDL -Iinclude

hb_stb.o:
	clang++ code/hb_stb.cpp -c -o hb_stb.o -Iinclude

hb_tinyobjloader.o:
	clang++ code/hb_tinyobjloader.cpp -c -o hb_tinyobjloader.o -Iinclude
