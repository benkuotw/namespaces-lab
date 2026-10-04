#ifndef NS_UTIL_H
#define NS_UTIL_H

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// print the namespace inode numbers in /proc/self/ns/
static inline int print_ns_inodes(const char *who) {
	char *ns_list[] = {//"cgroup",
			   //"ipc",
			   //"net",
			   "mnt", "pid",
			   //"time",
			   //"user",
			   "uts"};

	pid_t pid = getpid();

	char path[PATH_MAX + 1];
	char buf[PATH_MAX + 1];
	int ret = 0;

	memset(buf, 0, sizeof(buf));
	if (realpath("/proc/self/ns", buf) == NULL) {
		perror("/proc/self/ns");
		ret = -1;
	} else {
		fprintf(stderr, "%s[%d]: %s\n", who, pid, buf);
	}

	for (size_t i = 0; i < sizeof(ns_list) / sizeof(ns_list[0]); i++) {
		char *ns = ns_list[i];
		memset(path, 0, sizeof(path));
		int n = snprintf(path, PATH_MAX, "/proc/self/ns/%s", ns);
		if (n < 0 || (size_t)n >= PATH_MAX) {
			fprintf(stderr, "snprintf: path too long for %s\n", ns);
			ret = -1;
			continue;
		}

		memset(buf, 0, sizeof(buf));
		if (readlink(path, buf, PATH_MAX) == -1) {
			perror(path);
			ret = -1;
			continue;
		}
		fprintf(stderr, "%s[%d]: %s\n", who, pid, buf);
	}

	return ret;
}

static inline int set_and_print_hostname(const char *who, const char *name) {
	pid_t pid = getpid();

	// change hostname in new uts namespace
	if (sethostname(name, strlen(name)) == -1) {
		perror("sethostname");
		return -1;
	}

	char get_hostname[HOST_NAME_MAX + 1];
	memset(get_hostname, 0, sizeof(get_hostname));
	if (gethostname(get_hostname, HOST_NAME_MAX) == -1) {
		perror("gethostname");
		return -1;
	}
	fprintf(stderr, "%s[%d]: hostname=%s\n", who, pid, get_hostname);

	return 0;
}

#endif
