//
// Created by eli on 8/18/2022.
//

#include <cassert>
#include <regex>
#include <iostream>
#include "jsonVersioner.h"

JsonChange::JsonChange(const std::string& path, Json::Value newValue, VersionedJson* json) : JsonChange(path, std::move(newValue), Json::resolvePath(path, json->_root), json)
{
}

JsonChange::JsonChange(const std::string& path, Json::Value oldValue, Json::Value newValue, VersionedJson* json) : _path(path), _before(std::move(oldValue)), _after(std::move(newValue)), _json(json)
{
	redo();
}

void JsonChange::undo()
{
	Json::resolvePath(_path, _json->_root) = _before;
	--_json->_version;
	if(_json->_onChanged)
		_json->_onChanged();
}

void JsonChange::redo()
{
	Json::resolvePath(_path, _json->_root) = _after;
	++_json->_version;
	if(_json->_onChanged)
		_json->_onChanged();
}

const VersionedJson* JsonChange::json()
{
	return _json;
}

void JsonVersionTracker::recordChange(std::unique_ptr<JsonChange> change)
{
	if(_currentChange != _changes.end())
		_changes.erase(_currentChange, _changes.end());
	_changes.push_back(std::move(change));
	while(_changes.size() > maxChanges)
		_changes.pop_front();
	_currentChange = _changes.end();
}

void JsonVersionTracker::undo()
{
	if(_currentChange == _changes.begin())
		return;
	--_currentChange;
	(*_currentChange)->undo();
}

void JsonVersionTracker::redo()
{
	if(_currentChange == _changes.end())
		return;
	(*_currentChange)->redo();
	++_currentChange;
}

void JsonVersionTracker::clearChanges(const VersionedJson* json)
{
	if(_currentChange != _changes.end())
		_changes.erase(_currentChange, _changes.end());
	_changes.erase(std::remove_if(_changes.begin(), _changes.end(), [json](auto& c){
		return c->json() == json;
	}), _changes.end());

	_currentChange = _changes.end();
}

JsonVersionTracker::JsonVersionTracker()
{
	_currentChange = _changes.end();
}

VersionedJson::VersionedJson(JsonVersionTracker& tkr) : _tkr(tkr)
{

}

void VersionedJson::initialize(const Json::Value& value)
{
	_root = value;
	_version = 0;
	_lastCleanedVersion = 0;
	_tkr.clearChanges(this);
}

void VersionedJson::changeValue(const std::string& path, const Json::Value& newValue, bool changeComplete)
{
	if(!changeComplete)
	{
		if(!_uncompletedChange){
			_uncompletedChange = std::make_unique<UncompletedChange>();
			_uncompletedChange->path = path;
			_uncompletedChange->before = Json::resolvePath(path, _root);
		}
		Json::resolvePath(path, _root) = newValue;
		if(_onChanged)
			_onChanged();
		return;
	}

	if(_uncompletedChange)
	{
		assert(_uncompletedChange->path == path);
		_tkr.recordChange(std::make_unique<JsonChange>(path, _uncompletedChange->before, newValue, this));
		_uncompletedChange = nullptr;
		return;
	}

	_tkr.recordChange(std::make_unique<JsonChange>(path, newValue, this));

}

void VersionedJson::markClean()
{
	_lastCleanedVersion = _version;
}

bool VersionedJson::dirty() const
{
	return _lastCleanedVersion != _version;
}

Json::Value& VersionedJson::data()
{
	return _root;
}

void VersionedJson::onChanged(const std::function<void()>& callback)
{
	_onChanged = callback;
}

JsonVersionTracker& VersionedJson::tracker()
{
	return _tkr;
}

const Json::Value& VersionedJson::operator[](const std::string& p) const
{
	return _root[p];
}

const Json::Value& VersionedJson::operator[](const char* p) const
{
	return _root[p];
}

Json::Value& Json::resolvePath(const std::string& path, Json::Value& root)
{
	Json::Value* value = &root;
	std::vector<std::string> components;
	uint32_t begin = 0;
	for(uint32_t end = 1; end < path.size(); ++end)
	{
		if(path[end] == '/' && begin < end)
		{
			components.emplace_back(path.begin() + begin, path.begin() + end);
			begin = end + 1;
		}
	}
	if(begin < path.size())
		components.emplace_back(path.begin() + begin, path.end());
	for(auto& comp : components)
	{
		bool isIndex = true;
		for(char c : comp)
		{
			if(!std::isdigit(c))
			{
				isIndex = false;
				break;
			}
		}
		if(!isIndex)
			value = &(*value)[comp];
		else
			value = &(*value)[std::stoi(comp)];
	}

	return *value;
}
