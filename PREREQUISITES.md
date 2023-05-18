# Prerequisites
Project requires some important and well-known libraries, these libraries can be installed with the help of any popular package manager.

## Libraries
The following libraries should be installed from your favourite package manager:
- Basic Development tools (gcc, cmake, git)
- Vulkan (also Vulkan Tools)
- OpenGL
- OpenSSL
- Boost
- GLM  
- ImGUI 
- JsonCPP
- Gtest
##### With apt (Debian-based distros)
Install Vulkan SDK, [read the tutorial!](https://vulkan.lunarg.com/doc/view/latest/linux/getting_started_ubuntu.html)
```shell
sudo apt update
sudo apt install build-essential libopengl-dev libopenssl-dev libboost-dev libglm-dev libimgui-dev libjsoncpp-dev libgtest-dev
```
##### With *yay*/*pacman* (Arch-linux)
```shell
yay -Syu
yay -S git base-devel vulkan-devel opengl openssl boost glm imgui jsoncpp gtest
```
