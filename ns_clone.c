#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <limits.h>
#include <signal.h>
#include <sys/wait.h>

#include "ns_util.h"

#define STACK_SIZE (1024 * 1024)

int child_func(void *arg)
{
	pid_t pid=getpid();
	print_ns_inodes(" child");

	//change hostname in new uts namespace
	char set_hostname[]="NEW HOSTNAME";
	if (sethostname(set_hostname,strlen(set_hostname)) == -1) {
		perror("sethostname");
		exit(EXIT_FAILURE);
	} 

	char get_hostname[HOST_NAME_MAX+1];
	if (gethostname(get_hostname,sizeof(get_hostname)) == -1) {
		perror("gethostname");
		exit(EXIT_FAILURE);
	} 
	fprintf(stderr, " child[%d]: hostname=%s\n", pid, get_hostname);


	//sleep(600);

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	char *stack = malloc(STACK_SIZE);
	if (!stack) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	pid_t pid=getpid();

	int flags=
		//CLONE_NEWCGROUP | 
		//CLONE_NEWIPC | 
		//CLONE_NEWNET | 
		//CLONE_NEWNS | 
		//CLONE_NEWPID | 
		//CLONE_NEWTIME | 
		//CLONE_NEWUSER | 
		CLONE_NEWUTS | 
		SIGCHLD; 
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
