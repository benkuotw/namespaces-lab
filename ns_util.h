#ifndef NS_UTIL_H
#define NS_UTIL_H

#include <fcntl.h>
#include <limits.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// clang-format off
#define NS_FLAGS (			  \
	/* CLONE_NEWCGROUP | */   \
	/* CLONE_NEWIPC    | */   \
	/* CLONE_NEWNET    | */   \
	CLONE_NEWNS        |      \
	CLONE_NEWPID       |      \
	/* CLONE_NEWTIME   | */   \
	/* CLONE_NEWUSER   | */   \
	CLONE_NEWUTS       |      \
	0)

// clang-format on

// print the namespace inode numbers in /proc/self/ns/
static inline int print_ns_inodes(const char *who)
{
	char *ns_list[] = {//"cgroup",
			   //"ipc",
			   //"net",
			   "mnt", "pid",
			   //"time",
			   //"user",
			   "uts"};

	pid_t pid = getpid();

	char buf[PATH_MAX + 1];
	char hostname[HOST_NAME_MAX + 1];
	int ret = 0;

	if (realpath("/proc/self/ns", buf) == NULL) {
		perror("/proc/self/ns");
		buf[0] = '\0';
		ret = -1;
	}

	if (gethostname(hostname, HOST_NAME_MAX) == -1) {
		perror("gethostname");
		hostname[0] = '\0';
		ret = -1;
	}

	printf("%s[%d]: %s; %s\n", who, pid, buf, hostname);

	char path[PATH_MAX + 1];
	for (size_t i = 0; i < sizeof(ns_list) / sizeof(ns_list[0]); i++) {
		char *ns = ns_list[i];
		int n = snprintf(path, PATH_MAX, "/proc/self/ns/%s", ns);
		if (n < 0 || (size_t)n >= PATH_MAX) {
			fprintf(stderr, "snprintf: path too long for %s\n", ns);
			ret = -1;
			continue;
		}

		ssize_t len = readlink(path, buf, sizeof(buf) - 1);
		if (len == -1) {
			perror(path);
			ret = -1;
			continue;
		}
		buf[len] = '\0';

		printf("%s[%d]: %s\n", who, pid, buf);
	}

	fflush(stdout);

	return ret;
}

static inline void wait_and_exit(pid_t child_pid)
{
	int status;
	if (waitpid(child_pid, &status, 0) == -1) {
		perror("waitpid");
		exit(EXIT_FAILURE);
	}
	exit(WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status));
}

static inline int set_ns_mount()
{
	// prevent propagation, man mount_namespaces.7
	if (mount(NULL, "/", NULL, MS_SLAVE | MS_REC, NULL) == -1) {
		perror("mount");
		return -1;
	}

	if (mount("proc", "/proc", "proc", 0, NULL) == -1) {
		perror("mount");
		return -1;
	}

	return 0;
}

static inline int set_ns_uts()
{
	char *name = "namespaces-lab";
	if (sethostname(name, strlen(name)) == -1) {
		perror("sethostname");
		return -1;
	}

	return 0;
}

static inline int set_ns()
{
	if ((NS_FLAGS & (CLONE_NEWNS | CLONE_NEWPID)) == (CLONE_NEWNS | CLONE_NEWPID)) {
		if (set_ns_mount() == -1)
			return -1;
	}

	if (NS_FLAGS & CLONE_NEWUTS) {
		if (set_ns_uts() == -1)
			return -1;
	}

	return 0;
}

#endif
