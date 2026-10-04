#define _GNU_SOURCE
#include <limits.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ns_util.h"

int main(int argc, char *argv[]) {
	pid_t pid = getpid();
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
	if (unshare(flags) == -1) {
		perror("unshare");
		exit(EXIT_FAILURE);
	}
	print_ns_inodes(argv[0]);

	// change hostname in new uts namespace
	char set_hostname[] = "NEW HOSTNAME";
	if (sethostname(set_hostname, strlen(set_hostname)) == -1) {
		perror("sethostname");
		exit(EXIT_FAILURE);
	}

	char get_hostname[HOST_NAME_MAX + 1];
	if (gethostname(get_hostname, sizeof(get_hostname)) == -1) {
		perror("gethostname");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "%s[%d]: hostname=%s\n", argv[0], pid, get_hostname);

	// sleep(600);

	return EXIT_SUCCESS;
}
