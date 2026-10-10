#include "ns_util.h"

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s CMD [ARGs]\n", argv[0]);
		return EXIT_FAILURE;
	}

	// print the current namespaces
	pid_t pid = getpid();
	print_ns_inodes(argv[0]);

	pid_t child_pid = fork();
	if (child_pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (child_pid > 0) {
		printf("%s[%d]: created a child[%d] for new namespaces\n",
		       argv[0], pid, child_pid);
		wait_and_exit(child_pid);
	}

	// child creates new namespaces
	if (unshare(NS_FLAGS) == -1) {
		perror("unshare");
		exit(EXIT_FAILURE);
	}

	// the child still in host's PID namespace
	// the grandchild does in the new PID namespace
	if (NS_FLAGS & CLONE_NEWPID) {
		pid_t grandchild_pid = fork();
		if (grandchild_pid == -1) {
			perror("fork");
			exit(EXIT_FAILURE);
		}

		if (grandchild_pid > 0) {
			printf("%s[%d]: created a child[%d](grandchild) for "
			       "the new PID namespace\n",
			       argv[0], getpid(), grandchild_pid);
			wait_and_exit(grandchild_pid);
		}
	}

	if (set_ns() == -1)
		exit(EXIT_FAILURE);

	// child/grandchild
	print_ns_inodes(argv[0]);

	// execute the command in the new namespaces
	if (execvp(argv[1], &argv[1]) == -1) {
		perror("execvp");
		return EXIT_FAILURE;
	}
}
