//
// Created by wirewhiz on 1/19/22.
//
#include <iostream>
#include "gltfLoader.h"

int main()
{
    gltfLoader loader;

	std::cout << "GLTF modes \n1: gltf\n2: .glb\nMode: ";
	int mode;
	std::cin >> mode;
	switch(mode)
	{
		case 1:
		{
			std::cout << "GLTF file: ";
			std::string gltfFile;
			std::cin >> gltfFile;

			if(!loader.loadGltfFromFile(gltfFile))
			{
				std::cout << "Failed to open a file" << std::endl;
				return 0;
			}
			break;
		}
		case 2:
		{
			std::cout << "GLB file: ";
			std::string glbFile;
			std::cin >> glbFile;

			if(!loader.loadGlbFromFile(glbFile))
			{
				std::cout << "Failed to open a file" << std::endl;
				return 0;
			}
			break;
		}
		default:
			std::cout << "Unknown mode." << std::endl;
			return 0;
	}


    loader.printInfo();
    loader.printPositions(0,0);


    return 0;
}