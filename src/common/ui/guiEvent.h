//
// Created by eli on 5/22/2022.
//

#ifndef BRANEENGINE_GUIEVENT_H
#define BRANEENGINE_GUIEVENT_H

#include <string>

class GUIEvent
{
	std::string _name;
public:
	GUIEvent(std::string name);
	virtual ~GUIEvent() = default;
	const std::string& name() const;
};


#endif //BRANEENGINE_GUIEVENT_H
