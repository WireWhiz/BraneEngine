//
// Created by eli on 8/18/2022.
//

#include <cassert>
#include "jsonVersioner.h"

JsonChange::JsonChange(Json::Path path, Json::Value newValue, VersionedJson* json) : _path(std::move(path)), _after(std::move(newValue)), _json(json)
{
	_before = _path.resolve(_json->data());
	_path.make(_json->_root) = _after;
	++_json->_version;
}

JsonChange::JsonChange(Json::Path path, Json::Value oldValue, Json::Value newValue, VersionedJson* json) : _path(std::move(path)), _before(std::move(oldValue)), _after(std::move(newValue)), _json(json)
{
	_path.make(_json->_root) = _after;
	++_json->_version;
}

void JsonChange::undo()
{
	_path.make(_json->_root)= _before;
	--_json->_version;
}

void JsonChange::redo()
{
	_path.make(_json->_root)= _after;
	++_json->_version;
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

VersionedJson::VersionedJson(JsonVersionTracker& tkr) : _tkr(tkr)
{

}

void VersionedJson::initialize(Json::Value& value)
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
			_uncompletedChange->before = Json::Path(path).resolve(_root);
		}
		Json::Path(path).make(_root) = newValue;
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

const Json::Value& VersionedJson::data() const
{
	return _root;
}
