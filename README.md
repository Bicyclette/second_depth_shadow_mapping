# SECOND\_DEPTH\_SHADOW\_MAPPING
Master project for the CG3D lesson. <br/>
C++ implementation of the research paper made by Yulan WANG and Steven MOLNAR.

# install dependencies (ubuntu)
Must be performed as root:
+ apt-get install libsdl2-dev
+ apt-get install libglew-dev
+ apt-get install libassimp-dev
+ apt-get install libomp-dev

# how to build (GNU/linux systems)
1) in the root directory, create a build directory: mkdir build
2) then run the **generate\_makefile.py** script
3) checkout to build directory: cd build
4) compile: make
5) run: ./SecondDepthShadowMapping

# how to use
- middle mouse button to look around
- SHIFT + middle mouse button to pan the view
- scroll to zoom in or out
