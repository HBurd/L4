# L4
3D multiplayer space flight simulator.

Not much to do yet.

https://en.wikipedia.org/wiki/Lagrangian_point#L4_and_L5_points

## How to build

Windows:

Most recent known working Windows commit: 4d18a4242a16fb4b9be4d2efcab138437eae4fa0 (August 10th, 2019)

Open `vs/L4.sln`, build the x64 configuration from Visual Studio.
After the first build, manually copy `lib/SDL2.dll`, `lib/glew32.dll` and `resources` into `vs/x64`.
  
Linux:

This project uses a custom build script. Game code and libraries are compiled as independent unity builds. 

    cd build
    ./compile lib     # compiles everything, including some libraries
    ./compile         # only compiles core game files, a bit faster

This will build the client and server application, `L4client` and `L4server`.
