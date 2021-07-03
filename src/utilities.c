/**
 * @file utilities.c
 * @author bwisn (bwisn_dev (at) outlook (dot) com)
 * @brief 
 * @version 0.1
 * @date 24-06-2021
 * 
 * 
 */

#include <dirent.h>
#include <mntent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include "console.h"

/**
 * @brief Checks if specified directory exists
 * 
 * @param dirpath char* Path to the directory
 * @return uint8_t 1 if exists
 */
uint8_t util_dirExists(const char *dirpath) {
    DIR *d = opendir(dirpath);

    if (d) {
        closedir(d);
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Create directory with 755 chmod
 * 
 * @param dirpath path
 * @param mode mode (chmod)
 */
void util_dirCreate(const char *dirpath, mode_t mode) {
    if (dirpath != NULL) {
        if (!util_dirExists(dirpath)) {
            mkdir(dirpath, mode);
        }
    }
}

/**
 * @brief Checks for mountpoint entry in specified file
 * 
 * @param mntfile file with mountpoints
 * @param dirpath directory to check
 * @return uint8_t 1 if exists
 */
static uint8_t doMountMntryExist(char *mntfile, char *dirpath) {
    struct mntent *mount;
    uint8_t mounted = 0;
    FILE *fp;

    fp = setmntent("", "r");
    if (!fp)
        return 0;

    while ((mount = getmntent(fp))) {
        if (!strcmp(mount->mnt_dir, dirpath)) {
            mounted = 1;
        }
    }
    endmntent(fp);

    return mounted;
}

/**
 * @brief Checks in /proc/mounts if directory is mounted
 * 
 * @param dirpath target directory to check
 * @return uint8_t 1 if mounted
 */
uint8_t util_isMounted(char *dirpath) {
    return doMountMntryExist("/proc/mounts", dirpath);
}

/**
 * @brief Checks in /etc/fstab if mountpoint is in fstab
 * 
 * @param dirpath target directory to check
 * @return uint8_t 1 if exists
 */
uint8_t util_isInFstab(char *dirpath) {
    if (util_dirExists("/etc/fstab")) {
        return doMountMntryExist("/etc/fstab", dirpath);
    }
    return 0;
}
