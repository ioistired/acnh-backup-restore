#include "util.hpp"
#include "io.hpp"

bool fileExists(const std::string& path)
{
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

Result io::copyFile(const std::string& srcPath, const std::string& dstPath)
{
	printf("Copying %s\n", srcPath.c_str());

	FILE* src = fopen(srcPath.c_str(), "rb");
	if (src == NULL) {
		printf("Failed to open source file %s during copy with error: %s. Skipping...\n",
			srcPath.c_str(), strerror(errno)
		);
		return errno;
	}
	FILE* dst = fopen(dstPath.c_str(), "wb");
	if (dst == NULL) {
		printf("Failed to open destination file %s during copy with error: %s. Skipping...\n",
			dstPath.c_str(), strerror(errno)
		);
		fclose(src);
		return errno;
	}

	fseek(src, 0, SEEK_END);
	u64 sz = ftell(src);
	rewind(src);

	u8* buf	   = new u8[BUFSIZ];
	u64 offset = 0;

	while (offset < sz) {
		u32 count = fread((char*)buf, 1, BUFSIZ, src);
		fwrite((char*)buf, 1, count, dst);
		offset += count;
	}

	delete[] buf;
	fclose(src);
	fclose(dst);

	// commit each file to the save
	if (dstPath.rfind("save:/", 0) == 0) {
		printf("Committing file %s to the save archive.\n", dstPath.c_str());
		fsdevCommitDevice("save");
	}

	return 0;
}

Result io::copyDirectory(const std::string& srcPath, const std::string& dstPath)
{
	print("Entering directory %s...\n", srcPath.c_str());

	Result res = 0;
	bool quit  = false;
	DIR* dir = opendir(srcPath.c_str());
	struct dirent *ent;

	if (dir == NULL) {
		return errno;
	}

	while ((ent = readdir(dir)) && !quit) {
		std::string newsrc = srcPath + ent->d_name;
		std::string newdst = dstPath + ent->d_name;

		if (ent->d_type == DT_DIR) {
			res = io::createDirectory(newdst);
			if (R_SUCCEEDED(res)) {
				newsrc += "/";
				newdst += "/";
				res = io::copyDirectory(newsrc, newdst);
			}
			else {
				quit = true;
			}
		}
		else {
			io::copyFile(newsrc, newdst);
		}
	}

	closedir(dir);
	return 0;
}

Result io::createDirectory(const std::string& path)
{
	mkdir(path.c_str(), 777);
	return 0;
}

bool io::directoryExists(const std::string& path)
{
	struct stat sb;
	return (stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode));
}

Result io::deleteFolderRecursively(const std::string& path)
{
	DIR* dir = opendir(path.c_str());
	struct dirent* ent;
	if (dir == NULL) {
		return errno;
	}

	while ((ent = readdir(dir))) {
		if (ent->d_type == DT_DIR) {
			std::string newpath = path + "/" + ent->d_name + "/";
			deleteFolderRecursively(newpath);
			newpath = path + ent->d_name;
			rmdir(newpath.c_str());
		}
		else {
			std::string newpath = path + ent->d_name;
			std::remove(newpath.c_str());
		}
	}

	closedir(dir);
	rmdir(path.c_str());
	return 0;
}
