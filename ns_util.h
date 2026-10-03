#ifndef NS_UTIL_H
#define NS_UTIL_H

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

//print the namespace inode numbers (/proc/self/ns/<ns>)
static int print_ns_inodes(const char *who)
{
	char *ns_list[] = {
		//"cgroup", 
		//"ipc", 
		//"net", 
		//"mnt", 
		//"pid", 
		//"time", 
		//"user", 
		"uts"
	};

	int ret = 0;

	pid_t pid=getpid();
	for (size_t i = 0; i < sizeof(ns_list) / sizeof(ns_list[0]); i++) {
		char *ns = ns_list[i];
		char path[64];
		int n = snprintf(path, sizeof(path), "/proc/self/ns/%s", ns);
		if (n < 0 || (size_t) n >= sizeof(path)) {
			fprintf(stderr, "snprintf: path too long for %s\n", ns);
			ret = -1;
			continue;
		}

		//stat() follows the symlink, so st_ino is the namespace's inode
		struct stat st;
		if (stat(path, &st) == -1) {
			perror(path);
			ret = -1;
			continue;
		}
		fprintf(stderr, "%s[%d]: %s[%lu]\n", who, pid, ns, (unsigned long) st.st_ino);
	}

	return ret;
}

#endif
