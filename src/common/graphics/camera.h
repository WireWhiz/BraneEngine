//
// Created by eli on 5/25/2022.
//

#ifndef BRANEENGINE_CAMERA_H
#define BRANEENGINE_CAMERA_H

#include "glm/mat4x4.hpp"

namespace graphics
{

    class Camera
    {
    public:
        glm::mat4x4 transform;
        glm::mat4x4 view;
    };

} // graphics

#endif //BRANEENGINE_CAMERA_H
