//
// Created by eli on 6/28/2022.
//

#ifndef BRANEENGINE_EDITOR_H
#define BRANEENGINE_EDITOR_H

#include <runtime/module.h>
#include "common/ui/gui.h"

class Editor : public Module
{
	GUI* _ui;
	net::Connection* _server;
	void addMainWindows();
public:
	void start() override;
	static const char* name();
	net::Connection* server() const;
};


#endif //BRANEENGINE_EDITOR_H
