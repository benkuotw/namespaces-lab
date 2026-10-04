#define _GNU_SOURCE
#include <limits.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ns_util.h"

int main(int argc, char *argv[]) {
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

	if (set_and_print_hostname(argv[0], "NEW HOSTNAME") == -1) {
		exit(EXIT_FAILURE);
	}

	// sleep(600);

	return EXIT_SUCCESS;
}
