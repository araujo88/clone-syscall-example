#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

#define STACK_SIZE 65536 /* Arbitrary size */

void *stack;  /* Global variable for the new stack */

struct thread_args {
    int arg1;
    int arg2;
};

pid_t gettid(void) {
    return syscall(SYS_gettid);
}

int threadFunc(void *arg) {
    struct thread_args *args = (struct thread_args *)arg;

    printf("In child thread with ID: %ld\n", (long)gettid());
    printf("arg1: %d\n", args->arg1);
    printf("arg2: %d\n", args->arg2);
    return 0;
}

int main() {
    stack = malloc(STACK_SIZE);
    if (stack == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    printf("In parent thread with ID: %ld\n", (long)gettid());

    /* Allocates memory for the thread args */
    struct thread_args *args;
    args = (struct thread_args*) malloc(sizeof(struct thread_args) + STACK_SIZE);
    args->arg1 = 5;
    args->arg2 = 7;
    
    /* The child shares the virtual memory, file descriptors and signal handlers with the parent */
    pid_t pid = clone(threadFunc, stack + STACK_SIZE, CLONE_VM | CLONE_FILES | CLONE_SIGHAND, args);
    if (pid == -1) {
        perror("clone");
        exit(EXIT_FAILURE);
    } else {
        printf("Successfully created thread with PID: %d\n", pid);
    }
    
    /* Wait for the child to exit */
    wait(NULL);
    
    /* Free the stack */
    free(stack);

    /* Free args */
    free(args);

    return 0;
}
