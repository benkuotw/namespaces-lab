#define _GNU_SOURCE

#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "ns_util.h"

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s PID\n", argv[0]);
		return EXIT_FAILURE;
	}

	pid_t pid = atoi(argv[1]);
	if (pid == 0) {
		perror("atoi");
		exit(EXIT_FAILURE);
	}

	int fd = syscall(SYS_pidfd_open, pid, 0);
	if (fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	print_ns_inodes(argv[0]);

	int flags =
	    // CLONE_NEWCGROUP |
	    // CLONE_NEWIPC |
	    // CLONE_NEWNET |
	    // CLONE_NEWNS |
	    // CLONE_NEWPID |
	    // CLONE_NEWTIME |
	    // CLONE_NEWUSER |
	    CLONE_NEWUTS;
	// join the namespace
	if (setns(fd, flags) == -1) {
		perror("setns");
		return EXIT_FAILURE;
	}

	print_ns_inodes(argv[0]);

	return EXIT_SUCCESS;
}
