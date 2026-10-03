#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sched.h>

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Usage: %s /proc/PID/ns/FILE cmd [arg...]\n", argv[0]);
		return EXIT_FAILURE;
	}

	int fd = open(argv[1], O_RDONLY|O_CLOEXEC);     
	if (fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	//join the namespace
	if (setns(fd, 0) == -1){
		perror("setns");
		return EXIT_FAILURE;
	}

	//execute the command in the namespace
	if (execvp(argv[2], &argv[2]) == -1) {
		perror("execvp");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

