#include <switch.h>
#include <dirent.h>
#include <vector>
#include <algorithm>

// Animal Crossing: New Horizons
const u64 APPLICATION_ID = 0x01006F8002326000;
const char* OUTPUT_PATH = "sdmc:/switch/acnh-backup-restore";

enum state {
	STATE_MAIN,
	STATE_DONE,
	STATE_BACKUP,
	STATE_RESTORE_MENU,
	STATE_RESTORE
};

void backup(void);
void restore_menu(enum state& state, std::vector<char *>& backups);
void process_restore_input(enum state& state, std::vector<char *>& backups, int& selection, u64 key_down);
void restore(char* backup);
