# L4
3D multiplayer space flight simulator.

Not much to do yet.

## How to build
You need CMake to build this project.

Windows:

    mkdir build
    cd build
    cmake -G "Visual Studio 15 2017 Win64" ..
    
This will generate the Visual Studio solution. In theory this should also work with other versions of Visual Studio
  
Linux:

    mkdir build
    cd build
    cmake ..
    make

This will build the client and server application, `L4client and L4server`.
