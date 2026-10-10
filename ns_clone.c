#include "ns_util.h"

#define STACK_SIZE (1024 * 1024)

int child_func(void *arg)
{
	char *who = (char *)arg;

	set_ns();

	print_ns_inodes(who);

	sleep(60);

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	char *stack = malloc(STACK_SIZE);
	if (!stack) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	// print the current namespaces
	pid_t pid = getpid();
	print_ns_inodes(argv[0]);

	pid_t child_pid =
	    clone(child_func, stack + STACK_SIZE, NS_FLAGS | SIGCHLD, argv[0]);
	if (child_pid == -1) {
		perror("clone");
		exit(EXIT_FAILURE);
	}

	printf("%s[%d]: created a child[%d] for the new namespaces\n", argv[0],
	       pid, child_pid);

	wait_and_exit(child_pid);
}
