#pragma once
#include <filesystem>
#include <stdexcept>
#include <system_error>

inline void GetFilesWithID(std::string& dirPath, std::vector<std::string>& fileNames, std::string& id)
{
	//iterate through the directory
	std::filesystem::recursive_directory_iterator iter(dirPath);

	std::filesystem::recursive_directory_iterator end;

	while (iter != end)
	{
		if (!is_directory(iter->path()))
		{
			auto filename = iter->path().filename().string();

			if (filename.find(id) != std::string::npos)
			{
				fileNames.push_back(iter->path().filename().string());
				iter.disable_recursion_pending();
			}
		}
		std::error_code ec;
		iter.increment(ec);
		if (ec)
		{
			throw std::runtime_error("Error accessing " + ec.message());
		}
	}
}
