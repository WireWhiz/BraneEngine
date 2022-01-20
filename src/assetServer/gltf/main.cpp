//
// Created by wirewhiz on 1/19/22.
//
#include <iostream>
#include "gltfLoader.h"

int main()
{
    gltfLoader loader;

    std::cout << "GLTF file: ";
    std::string gltfFile;
    std::cin >> gltfFile;

    std::cout << "bin file: ";
    std::string binFile;
    std::cin >> binFile;

    if(!loader.loadGltfFromFile(gltfFile, binFile))
    {
        std::cout << "Failed to open a file" << std::endl;
        return 0;
    }

    loader.printInfo();
    loader.printPositions(0,0);


    return 0;
}