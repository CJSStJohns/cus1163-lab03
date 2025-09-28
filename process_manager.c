#include "process_manager.h"

/*
 * Function 1: Basic Producer-Consumer Demo
 * Creates one producer child (sends 1,2,3,4,5) and one consumer child (adds them up)
 */
int run_basic_demo(void) {
    int pipe_fd[2];
    pid_t producer_pid, consumer_pid;
    int status;
    
    printf("\nParent process (PID: %d) creating children...\n", getpid());
    
    // TODO 1: Create a pipe for communication
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        return -1;
    }

    // TODO 2: Fork the producer process
    producer_pid = fork();
    if (producer_pid < 0) {
        perror("fork failed");
        return -1;
    } else if (producer_pid == 0) {
        close(pipe_fd[0]); // close read end
        producer_process(pipe_fd[1], 1); // start with 1
        exit(0);
    } else {
        printf("Created producer child (PID: %d)\n", producer_pid);
    }

    // TODO 3: Fork the consumer process
    consumer_pid = fork();
    if (consumer_pid < 0) {
        perror("fork failed");
        return -1;
    } else if (consumer_pid == 0) {
        close(pipe_fd[1]); // close write end
        consumer_process(pipe_fd[0], 0); // pair_id 0
        exit(0);
    } else {
        printf("Created consumer child (PID: %d)\n", consumer_pid);
    }

    // TODO 4: Parent cleanup - close pipe ends and wait for children
    close(pipe_fd[0]);
    close(pipe_fd[1]);

    waitpid(producer_pid, &status, 0);
    printf("Producer child (PID: %d) exited with status %d\n", producer_pid, WEXITSTATUS(status));

    waitpid(consumer_pid, &status, 0);
    printf("Consumer child (PID: %d) exited with status %d\n", consumer_pid, WEXITSTATUS(status));

    printf("\nSUCCESS: Basic producer-consumer completed!\n");
    return 0;
}

/*
 * Function 2: Multiple Producer-Consumer Pairs
 * Creates multiple pairs: pair 1 uses numbers 1-5, pair 2 uses 6-10, etc.
 */
int run_multiple_pairs(int num_pairs) {
    pid_t pids[20]; // Store all child PIDs (2 per pair)
    int pid_count = 0;

    printf("\nParent creating %d producer-consumer pairs...\n", num_pairs);

    // TODO 5: Create multiple producer-consumer pairs
    for (int i = 0; i < num_pairs; i++) {
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe failed");
            return -1;
        }

        int start_num = i * NUM_VALUES + 1;

        printf("\n=== Pair %d ===\n", i + 1);

        // Fork producer
        pid_t producer_pid = fork();
        if (producer_pid < 0) {
            perror("fork failed");
            return -1;
        } else if (producer_pid == 0) {
            close(pipe_fd[0]);
            producer_process(pipe_fd[1], start_num);
            exit(0);
        } else {
            pids[pid_count++] = producer_pid;
        }

        // Fork consumer
        pid_t consumer_pid = fork();
        if (consumer_pid < 0) {
            perror("fork failed");
            return -1;
        } else if (consumer_pid == 0) {
            close(pipe_fd[1]);
            consumer_process(pipe_fd[0], i + 1);
            exit(0);
        } else {
            pids[pid_count++] = consumer_pid;
        }

        // Parent closes pipe ends
        close(pipe_fd[0]);
        close(pipe_fd[1]);
    }

    // TODO 6: Wait for all children
    int status;
    for (int i = 0; i < pid_count; i++) {
        waitpid(pids[i], &status, 0);
        printf("Child (PID: %d) exited with status %d\n", pids[i], WEXITSTATUS(status));
    }

    printf("\nSUCCESS: Multiple pairs completed!\n");
    return 0;
}

/*
 * Producer Process - Sends 5 sequential numbers starting from start_num
 */
void producer_process(int write_fd, int start_num) {
    printf("Producer (PID: %d) starting...\n", getpid());
    
    for (int i = 0; i < NUM_VALUES; i++) {
        int number = start_num + i;
        if (write(write_fd, &number, sizeof(number)) != sizeof(number)) {
            perror("write");
            exit(1);
        }
        printf("Producer: Sent number %d\n", number);
        usleep(100000);
    }

    printf("Producer: Finished sending %d numbers\n", NUM_VALUES);
    close(write_fd);
    exit(0);
}

/*
 * Consumer Process - Receives numbers and calculates sum
 */
void consumer_process(int read_fd, int pair_id) {
    int number;
    int sum = 0;

    printf("Consumer (PID: %d) starting...\n", getpid());

    while (read(read_fd, &number, sizeof(number)) > 0) {
        sum += number;
        printf("Consumer: Received %d, running sum: %d\n", number, sum);
    }

    printf("Consumer: Final sum: %d\n", sum);
    close(read_fd);
    exit(0);
}
