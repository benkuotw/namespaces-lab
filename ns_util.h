#ifndef NS_UTIL_H
#define NS_UTIL_H

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// print the namespace inode numbers in /proc/PID/ns/
static int print_ns_inodes(const char *who) {
	char *ns_list[] = {//"cgroup",
			   //"ipc",
			   //"net",
			   //"mnt",
			   //"pid",
			   //"time",
			   //"user",
			   "uts"};

	pid_t pid = getpid();
	char path[64];
	char buf[64];
	int ret = 0;

	// readlink in /proc/PID/ns
	for (size_t i = 0; i < sizeof(ns_list) / sizeof(ns_list[0]); i++) {
		char *ns = ns_list[i];
		memset(path, 0, sizeof(path));
		int n = snprintf(path, sizeof(path), "/proc/%d/ns/%s", pid, ns);
		if (n < 0 || (size_t)n >= sizeof(path)) {
			fprintf(stderr, "snprintf: path too long for %s\n", ns);
			ret = -1;
			continue;
		}

		memset(buf, 0, sizeof(buf));
		if (readlink(path, buf, sizeof(buf)) == -1) {
			perror(path);
			ret = -1;
			continue;
		}
		fprintf(stderr, "%s[%u]: %s\n", who, pid, buf);
	}

	return ret;
}

#endif
