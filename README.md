## PBDynamics

PBDynamics is a simple physics engine based on Position Based Dynamics. It is written in C++ and uses OpenGL for rendering.

### Dependencies:
- [GLFW](https://www.glfw.org/)
- [GLM](https://glm.g-truc.net/0.9.9/index.html)
- [GLEW](http://glew.sourceforge.net/)
- [ImGui]


### To build the project:
```
mkdir build 
sudo bash ./build.sh -c
./PBDynamics
``` 

### To run the simulation:
- Press `space` to start the simulation
- Press `q` to enable and disable the camera movement
- Move the camera using `w`, `a`, `s`, `d`
- Press `ctrl` to exit the simulation

### TODO:
- [x] Fix orthographic projection