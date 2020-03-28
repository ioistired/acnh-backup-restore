#include <switch.h>

#include <stdio.h>
#include <time.h>

#include "util.hpp"
#include "io.hpp"
#include "main.hpp"

//This example shows how to access savedata for (official) applications/games.

int main(int argc, char **argv) {
	consoleInit(NULL);
	print("Hello!\n");
	print("Press B to backup your ACNH save.\n");
	print("Press A to restore a save.\n");
	print("Press + to exit.\n");

	int selection = 0;
	enum state state = STATE_MAIN;
	std::vector<char *> backups;

	while(appletMainLoop()) {
		//Scan all the inputs. This should be done once for each frame
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

		if (kDown & KEY_PLUS) {
			break; // break in order to return to hbmenu
		} else if (kDown & KEY_B && state == STATE_MAIN) {
			backup();
			state = STATE_DONE;
		} else if (kDown & KEY_A && state == STATE_MAIN) {
			state = STATE_RESTORE_MENU;
			restore_menu(&state, &backups);
		} else if (state == STATE_RESTORE_MENU) {
			process_restore_input(&state, &backups, &selection, kDown);
		} else if (state == STATE_RESTORE) {
			restore(backups[selection]);
			state = STATE_DONE;
		}

		consoleUpdate(NULL);
	}

	consoleExit(NULL);
	return 0;
}

void backup() {
	Result rc = 0;

	DIR* dir = NULL;

	rc = fsdevMountDeviceSaveData("save", APPLICATION_ID);
	if (R_FAILED(rc)) {
		print("fsdevMountDeviceSaveData() failed: 0x%x\n", rc);
	}

	if (R_SUCCEEDED(rc)) {
		dir = opendir("save:/");
		if(dir == NULL) {
			print("Failed to open dir.\n");
		} else {
			io::createDirectory(OUTPUT_PATH);

			time_t unixTime = time(NULL);
			struct tm* now = localtime((const time_t *)&unixTime);
			char buf[FS_MAX_PATH];
			snprintf(
				buf, sizeof(buf),
				"%s/%d-%02d-%02d %02d%02d%02d/",
				OUTPUT_PATH,
				now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
				now->tm_hour, now->tm_min, now->tm_sec
			);
			std::string dstPath(buf);
			io::createDirectory(dstPath);
			print("Copying save file to %s...\n", dstPath.c_str());
			rc = io::copyDirectory("save:/", dstPath);
			if (R_FAILED(rc)) {
				io::deleteFolderRecursively(dstPath + "/");
				print("Failed to copy directory %s with result 0x%08lX. Skipping...\n", dstPath.c_str(), rc);
			}
		}

		fsdevUnmountDevice("save");
	}
	if (R_SUCCEEDED(rc)) {
		print("Done! Have a nice day :)\n");
	} else {
		print("Failed due to errors above.\n");
	}

	print("Press + to exit.\n");
}

void restore_menu(enum state* state, std::vector<char *>* backups) {
	DIR* dir = NULL;
	dir = opendir(OUTPUT_PATH);
	struct dirent* ent;

	if (dir == NULL) {
		print("Failed to open output dir: %s. Make sure it exists.\n", strerror(errno));
		print("Press + to exit.\n");
		*state = STATE_DONE;
		return;
	}

	print("\x1b[5;0H");
	print("Choose a backup to restore:\n");

	while ((ent = readdir(dir))) {
		if (ent->d_type == DT_DIR) {
			backups->push_back(strdup(ent->d_name));
		}
	}
	closedir(dir);

	std::sort(backups->begin(), backups->end(), [](char* a, char* b) {
		return strcmp(a, b) < 0;
	});

	for (const auto& v : *backups) {
		print("  %s\n", v);
	}
}

void process_restore_input(enum state* state, std::vector<char *>* backups, int* selection, u64 key_down) {
	if (key_down & KEY_A) {
		*state = STATE_RESTORE;
		print("\x1b[2J");  // clear screen
		return;
	}

	if (key_down & KEY_DOWN) {
		print("\x1b[%d;0H ", 6 + *selection);
		(*selection)++;
	} else if (key_down & KEY_UP) {
		print("\x1b[%d;0H ", 6 + *selection);
		(*selection)--;
	}
	if (*selection < 0) {
		*selection = backups->size() - 1;
	} else if (*selection >= backups->size()) {
		*selection = 0;
	}

	print("\x1b[%d;0H>", 6 + *selection);
}

void restore(char *backup) {
	Result rc = 0;
	std::string path(OUTPUT_PATH);
	path += "/";
	path += backup;
	path += "/";

	rc = fsdevMountDeviceSaveData("save", APPLICATION_ID);
	if (R_FAILED(rc)) {
		print("fsdevMountDeviceSaveData() failed: 0x%x\n", rc);
		goto done;
	}

	rc = io::deleteFolderRecursively("save:/");
	if (R_FAILED(rc)) {
		print("Failed to delete the target save directory with error: %s\n", strerror(rc));
		goto done;
	}

	rc = io::copyDirectory(path, "save:/");
	if (R_FAILED(rc)) {
		print("Restore failed: 0x%x\n", rc);
	} else {
		print("Restore succeeded! Have a nice day :)\n");
	}

	done:
	fsdevUnmountDevice("save");
}
