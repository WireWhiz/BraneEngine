# Prerequisites
In the following steps you can read more on how to get started with the repository.
### Environment setup
##### Libraries
The following libraries are required and should be installed.
- Vulkan SDK
- OpenGL
- GLFW
- OpenSSL
- Boost
- GLM
- JsonCPP
- Gtest

Read on how to install them in the following steps.
##### Ubuntu (debian-based distros)
1. Install basic tools such as cmake, git and default compilers.
    ```shell
    sudo apt update
    sudo apt install git cmake build-essential
    ```
   Clone the repository, initialize submodules
    ```
    git clone https://github.com/WireWhiz/BraneEngine.git
    cd BraneEngine
    git submodule update --init
    ```
2. Install Vulkan SDK, [read the tutorial!](https://vulkan.lunarg.com/doc/view/latest/linux/getting_started_ubuntu.html). The following only applies to Jammy Jellyfish (22.04). 
    ```shell
    wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc
    sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-jammy.list http://packages.lunarg.com/vulkan/lunarg-vulkan-jammy.list
    sudo apt update
    sudo apt install vulkan-sdk
    ```
3. Install required libraries
    ```shell
    sudo apt update
    # libgl1-mesa-dev 
    sudo apt install cmake libopengl-dev libxrandr libxinerama-dev libxi-dev libxxf86vm-dev libglfw3-dev libssl-dev libboost-dev libglm-dev libjsoncpp-dev libgtest-dev
    ```
##### Arch Linux
If you are on Arch Linux, I believe you can manage most of this stuff on your own. 
1. Install the basic tools
    ```shell
    yay -Syu
    yay -S cmake git base-devel 
    ```
   Clone the repository, and **initialize the submodules**!
    ```shell
    git submodule update --init
    ```
2. Install required libraries
    ```shell
    yay -S vulkan-devel opengl openssl boost glm jsoncpp gtest
    ```
### Compiling
Make and enter the build directory, generate cmake and then build. 
```shell
mkdir build && cd build
cmake ..
cmake --build .
```