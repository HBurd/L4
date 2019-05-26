# L4
3D multiplayer space flight simulator.

Not much to do yet.

## How to build

Windows:

Open vs/L4.sln, build the x64 configuration from Visual Studio.
After the first build, manually copy `lib/SDL2.dll`, `lib/glew32.dll` and `resources` into `vs/x64`.
  
Linux:

This project uses a custom build script. Game code and libraries are compiled as independent unity builds. 

    cd build
    ./compile lib     # compiles everything, including some libraries
    ./compile         # only compiles core game files, a bit faster

This will build the client and server application, `L4client` and `L4server`.
