//
// Created by eli on 8/18/2022.
//

#ifndef BRANEENGINE_JSONVERSIONER_H
#define BRANEENGINE_JSONVERSIONER_H

#include <json/json.h>
#include <deque>

class VersionedJson;
class JsonChange
{
	VersionedJson* _json = nullptr;
	Json::Path _path;
	Json::Value _before;
	Json::Value _after;
public:
	JsonChange(Json::Path path, Json::Value newValue, VersionedJson*  json);
	JsonChange(Json::Path path, Json::Value oldValue, Json::Value newValue, VersionedJson*  json);
	const VersionedJson* json();
	void undo();
	void redo();
};

class JsonVersionTracker
{
	std::deque<std::unique_ptr<JsonChange>> _changes;
	std::deque<std::unique_ptr<JsonChange>>::iterator _currentChange;

	//TODO add this to config
	size_t maxChanges = 200;
public:
	void recordChange(std::unique_ptr<JsonChange> change);
	void clearChanges(const VersionedJson* json);
	void undo();
	void redo();
};

class VersionedJson
{
	JsonVersionTracker& _tkr;
	Json::Value _root;
	size_t _version = 0;
	size_t _lastCleanedVersion = 0;

	struct UncompletedChange{
		std::string path;
		Json::Value before;
	};
	std::unique_ptr<UncompletedChange> _uncompletedChange;
	friend class JsonChange;
public:
	VersionedJson(JsonVersionTracker& tkr);
	void initialize(Json::Value& value);
	void changeValue(const std::string& path, const Json::Value& newValue, bool changeComplete = true);
	void markClean();
	bool dirty() const;
	const Json::Value& data() const;
};


#endif //BRANEENGINE_JSONVERSIONER_H
