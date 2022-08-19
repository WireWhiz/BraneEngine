//
// Created by eli on 8/18/2022.
//

#ifndef BRANEENGINE_JSONVERSIONER_H
#define BRANEENGINE_JSONVERSIONER_H

#include <json/json.h>
#include <deque>
#include <functional>

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
	JsonVersionTracker();
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
	std::function<void()> _onChanged;
	friend class JsonChange;
public:
	VersionedJson(JsonVersionTracker& tkr);
	void initialize(const Json::Value& value);
	void changeValue(const std::string& path, const Json::Value& newValue, bool changeComplete = true);
	void markClean();
	void onChanged(const std::function<void()>& callback);
	bool dirty() const;
	Json::Value& data();
	JsonVersionTracker& tracker();
};


#endif //BRANEENGINE_JSONVERSIONER_H
