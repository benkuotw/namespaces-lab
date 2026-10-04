#define _GNU_SOURCE
#include <limits.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ns_util.h"
#include <sys/mount.h>

#define STACK_SIZE (1024 * 1024)

int child_func(void *arg) {
	// prevent propagation, man mount_namespaces.7
	if (mount(NULL, "/", NULL, MS_SLAVE | MS_REC, NULL) == -1) {
		perror("mount");
		exit(EXIT_FAILURE);
	}

	if (mount("proc", "/proc", "proc", 0, NULL) == -1) {
		perror("mount");
		exit(EXIT_FAILURE);
	}

	print_ns_inodes(" child");

	if (set_and_print_hostname(" child", "NEW HOSTNAME") == -1) {
		exit(EXIT_FAILURE);
	}

	// sleep(600);

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
	char *stack = malloc(STACK_SIZE);
	if (!stack) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	pid_t pid = getpid();

	int flags =
	    // CLONE_NEWCGROUP |
	    // CLONE_NEWIPC |
	    // CLONE_NEWNET |
	    CLONE_NEWNS | CLONE_NEWPID |
	    // CLONE_NEWTIME |
	    // CLONE_NEWUSER |
	    CLONE_NEWUTS | SIGCHLD;
	pid_t child_pid = clone(child_func, stack + STACK_SIZE, flags, NULL);
	if (child_pid == -1) {
		perror("clone");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "parent[%d]: created child[%d]\n", pid, child_pid);

	if (waitpid(child_pid, NULL, 0) == -1) {
		perror("waitpid");
		exit(EXIT_FAILURE);
	}
	free(stack);

	print_ns_inodes("parent");

	return EXIT_SUCCESS;
}
