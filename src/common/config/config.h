#pragma once
#include <json/json.h>
#include <fstream>
#include <iostream>
#include <cstdint>

class Config
{
	const static char* configFileName;
	static Json::Value root;

public:
	static void loadConfig();

	static Json::Value json();
};