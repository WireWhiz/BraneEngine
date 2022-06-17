//
// Created by wirewhiz on 4/13/22.
//

#ifndef BRANEENGINE_MODULE_H
#define BRANEENGINE_MODULE_H

// Modules will contain all behaviour for the entire engine. Additionally, they may end up being the way that external scripts will be stored
class Module
{
public:
	virtual ~Module() = default;
	virtual const char* name() = 0;
	virtual void start();
	virtual void stop();

};


#endif //BRANEENGINE_MODULE_H
