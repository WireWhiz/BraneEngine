//
// Created by eli on 8/17/2022.
//

#ifndef BRANEENGINE_FILEWATCHER_H
#define BRANEENGINE_FILEWATCHER_H

#include <filesystem>
#include <vector>
#include <unordered_map>
#include <functional>

class FileWatcher
{
	std::unordered_map<std::string, const std::function<void(const std::filesystem::path& file)>> _fileWatchers;
	std::vector<std::filesystem::path> _watchedDirectories;
	std::unordered_map<std::string, std::filesystem::file_time_type> _lastUpdate;
public:
	void watchDirectory(const std::filesystem::path& directory);
	void addFileWatcher(const std::string& ext, std::function<void(const std::filesystem::path& file)> callback);
	void scanForChanges();
};


#endif //BRANEENGINE_FILEWATCHER_H
