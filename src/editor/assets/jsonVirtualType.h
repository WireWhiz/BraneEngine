//
// Created by eli on 8/20/2022.
//

#ifndef BRANEENGINE_JSONVIRTUALTYPE_H
#define BRANEENGINE_JSONVIRTUALTYPE_H

#include <json/json.h>
#include <ecs/virtualType.h>

class JsonVirtualType
{
public:
	static Json::Value fromVirtual(byte* data, VirtualType::Type type);
	static void toVirtual(byte* data, const Json::Value& source, VirtualType::Type type);
};


#endif //BRANEENGINE_JSONVIRTUALTYPE_H
