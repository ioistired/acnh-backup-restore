#ifndef IO_HPP
#define IO_HPP

#include <switch.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <string>

namespace io {
	Result copyDirectory(const std::string& srcPath, const std::string& dstPath);
	Result copyFile(const std::string& srcPath, const std::string& dstPath);
	Result createDirectory(const std::string& path);
	Result deleteFolderRecursively(const std::string& path);
	bool directoryExists(const std::string& path);
	bool fileExists(const std::string& path);
}
# endif
