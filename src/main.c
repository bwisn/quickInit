/**
 * @file main.c
 * @author bwisn (bwisn_dev (at) outlook (dot) com)
 * @brief 
 * @version 0.1
 * @date 24-06-2021
 * 
 * 
 */
#include <linux/prctl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/klog.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "console.h"
#include "process.h"
#include "signals.h"
#include "svc.h"
#include "tty.h"
#include "utilities.h"

/**
 * @brief Mounts base filesystems
 * 
 */
static void mountBaseFs() {
    if (!util_isMounted("/proc") && mount("none", "/proc", "proc", MS_NODEV | MS_NOSUID | MS_NOEXEC, NULL)) {
        console_error("failed mounting /proc");
    }
    if (util_dirExists("/proc/bus/usb")) {
        mount("none", "/proc/bus/usb", "usbfs", 0, NULL);
    }

    util_dirCreate("/sys", 0755);

    if (!util_isMounted("/sys") && mount("none", "/sys", "sysfs", MS_NODEV | MS_NOSUID | MS_NOEXEC, NULL)) {
        console_error("failed mounting /sys");
    }
    util_dirCreate("/dev/pts", 0755);
    mount("devpts", "/dev/pts", "devpts", 0, "gid=5,mode=620,ptmxmode=0666");

    if (!util_isInFstab("/dev/shm") && !util_isMounted("/dev/shm")) {
        util_dirCreate("/dev/shm", 0755);
        mount("shm", "/dev/shm", "tmpfs", 0, "mode=0777");
    }

    if (util_dirExists("/run") && !util_isMounted("/run"))
        mount("tmpfs", "/run", "tmpfs", MS_NODEV | MS_NOSUID | MS_NOEXEC, "mode=0755");
}

int main() {
    if (getuid() != 0) {
        printf("quickInit: only root can execute this\r\n");
        return EXIT_FAILURE;
    }
    if (getpid() != 1) {
        printf("quickInit: must be run as PID 1\r\n");
        return EXIT_FAILURE;
    }

    signals_setup();
    mountBaseFs();
    klogctl(6, NULL, 0);  // SYSLOG_ACTION_CONSOLE_OFF=6 disable printing printk to console

    console_printVersion();

    chdir("/");
    umask(022);                                                                         // set umask default for root
    setenv("PATH", "/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/sbin:/usr/local/bin", 1);  // overwrite PATH env to default
    setlocale(LC_ALL, "");

    svc_init();
    svc_waitForAll();
    tty_init();

    process_spawnChildren();

    return EXIT_SUCCESS;
}
