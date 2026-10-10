#include "ns_util.h"

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Usage: %s PID CMD [ARGs]\n", argv[0]);
		return EXIT_FAILURE;
	}

	pid_t join_pid = atoi(argv[1]);
	if (join_pid == 0) {
		perror("atoi");
		exit(EXIT_FAILURE);
	}

	int fd = syscall(SYS_pidfd_open, join_pid, 0);
	if (fd == -1) {
		perror("pidfd_open");
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

	// child joins the namespaces
	if (setns(fd, NS_FLAGS) == -1) {
		perror("setns");
		return EXIT_FAILURE;
	}

	// the child still in the host pid namespace
	// the grandchild does in the new pid namespaces
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

	// child/grandchild
	print_ns_inodes(argv[0]);

	// execute the command in the new namespace
	if (execvp(argv[2], &argv[2]) == -1) {
		perror("execvp");
		return EXIT_FAILURE;
	}
}
