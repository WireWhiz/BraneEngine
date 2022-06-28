//
// Created by eli on 5/22/2022.
//

#ifndef BRANEENGINE_EDITOREVENT_H
#define BRANEENGINE_EDITOREVENT_H

#include <string>

class EditorEvent
{
	std::string _name;
public:
	EditorEvent(std::string name);
	virtual ~EditorEvent() = default;
	const std::string& name() const;
};


#endif //BRANEENGINE_EDITOREVENT_H
