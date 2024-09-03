// Features code from Randy Bryant and Dave O'Halloran's original "Data Lab"
// Improvements by Jack Kolb <jhkolb@umn.edu>
#include <assert.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

#include "bits_impl.h"
#include "dl_protocol.h"
#include "sema.h"
#include "utils.h"

#define SHMID_STRLEN 32
#define SERVER_PROG "./btest_server"

// Globals for signal handling
jmp_buf envbuf;
int timeout_limit = 10;

void handle_sig(int signo) {
    if (signo == SIGALRM) {
        // Timeout case
        siglongjmp(envbuf, 1);
    } else if (signo == SIGSEGV) {
        // Segfault case
        siglongjmp(envbuf, 2);
    } else if (signo == SIGFPE) {
        // Floating point fault
        siglongjmp(envbuf, 3);
    }
}

int install_signal_handler() {
    struct sigaction sigact;
    sigact.sa_handler = handle_sig;
    if (sigemptyset(&sigact.sa_mask) == -1) {
        perror("sigemptyset");
        return -1;
    }
    sigact.sa_flags = SA_RESTART;

    if (sigaction(SIGALRM, &sigact, NULL) == -1 || sigaction(SIGSEGV, &sigact, NULL) == -1) {
        perror("sigaction");
        return -1;
    }

    return 0;
}

// Restrict to brief output for grading purposes?
int grade_mode = 0;

// Point totals
unsigned total_points_possible = 0;
unsigned total_points_earned = 0;

// Display usage info
static void usage(char *cmd) {
    printf("Usage: %s [-hg] [-f <name> [-1|-2|-3 <val>]*] [-T <time limit>]\n", cmd);
    printf("  -1 <val>  Specify first function argument\n");
    printf("  -2 <val>  Specify second function argument\n");
    printf("  -3 <val>  Specify third function argument\n");
    printf("  -f <name> Test only the named function\n");
    printf("  -g        Compact output for grading (with no error msgs)\n");
    printf("  -h        Print this message\n");
    printf("  -T <lim>  Set timeout limit to lim\n");
    exit(1);
}

void reportFunctionResult(enum test_outcome outcome, function_result_t *result) {
    const char *func_name = getFuncName(result->function_id);
    unsigned rating = getFuncRating(result->function_id);
    total_points_possible += rating;

    if (outcome == SUCCESS) {
        printf(" %d\t%d\t%d\t%s\n", rating, rating, 0, func_name);
        total_points_earned += rating;
    } else {
        if (!grade_mode) {
            if (outcome == FAILURE) {
                switch (getNumArgs(result->function_id)) {
                    case 2: {
                        printf("ERROR: Test %s(%d[0x%x], %d[0x%x]) failed...\n",
                            func_name, result->arg1, result->arg1,
                            result->arg2, result->arg2);
                        break;
                    }
                    case 1: {
                        printf("ERROR: Test %s(%d[0x%x]) failed...\n",
                            func_name, result->arg1, result->arg1);
                        break;
                    }
                    case 0: {
                        printf("ERROR: Test %s() failed...\n", func_name);
                        break;
                    }
                    default: {
                        // Should never happen
                        printf("Error: Invalid function ID received from server\n");
                    }
                }
                printf("...Gives %d[0x%x].  Should be %d[0x%x]\n",
                    result->actual_output, result->actual_output,
                    result->expected_output, result->expected_output);
            } else if (outcome == TIMEOUT) {
                printf("ERROR: Test %s failed.\n", func_name);
                printf("  Timed out after %d secs (probably infinite loop)\n", timeout_limit);
            } else if (outcome == SEGFAULT) {
                printf("ERROR: Test %s failed.\n", func_name);
                printf("  Segmentation Fault\n");
            } else if (outcome == FLOAT_ERROR) {
                printf("ERROR: Test %s failed.\n", func_name);
                printf("  Floating Point Operation Exception\n");
            }
        }
        printf(" %d\t%d\t%d\t%s\n", 0, rating, 1, func_name);
    }
}

int main(int argc, char **argv) {
    // Parse command line args
    char c;
    while ((c = getopt(argc, argv, "hgf:T:1:2:3:")) != -1)
        switch (c) {
            // Ignore this, it's passed to server
            case 'f':
                break;

            // Don't care what these are specifically
            // Only need to verify that we can parse them
            // Then just pass them to server
            // Apparently "optarg" is already defined for us with the getopt stuff
            case '1':
            case '2':
            case '3': {
                unsigned u = 0;
                if (get_num_val(optarg, &u) == 0) {
                    printf("Bad argument '%s'\n", optarg);
                    return 0;
                }
                break;
            }

            case 'g': // grading option for autograder
                grade_mode = 1;
                break;

            case 'T': // Set timeout limit
                timeout_limit = atoi(optarg);
                break;

            case 'h': // help
                usage(argv[0]);
                return 0;

            default:
                usage(argv[0]);
                return 0;
    }

    if (install_signal_handler() == -1) {
        return 1;
    }

    // Set up memory to share with server
    int shm_id = shmget(IPC_PRIVATE, sizeof(shmem_buf_t), IPC_CREAT|S_IRUSR|S_IWUSR);
    if (shm_id == -1) {
        perror("shmget");
        return 1;
    }
    shmem_buf_t *sh_buf = shmat(shm_id, NULL, 0);
    if (sh_buf == (void *) -1) {
        perror("shmat");
        return 1;
    }
    // This doesn't immediately remove shmem segment
    // Just marks it for automatic removal once last process detaches
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        return 1;
    }
    sh_buf->ready_for_client = 0;
    sh_buf->ready_for_server = 0;

    // Launch server process
    pid_t child_pid = fork();
    if (child_pid == 0) {
        // Detach from inherited memory segment (will reattach after exec)
        if (shmdt(sh_buf) == -1) {
            perror("shmdt");
            return 1;
        }

        // Store shm_id in a string for use with exec below
        char shmid_str[SHMID_STRLEN];
        snprintf(shmid_str, SHMID_STRLEN, "%d", shm_id);

        // Prepare command-line arguments for server.
        // Includes shm_id plus all args passed to this process (client)
        char *server_argv[argc + 2]; // All args plus shm_id plus NULL sentinel
        server_argv[0] = SERVER_PROG;
        server_argv[1] = shmid_str;
        for (int i = 1; i < argc; i++) {
            server_argv[i + 1] = argv[i];
        }
        server_argv[argc + 1] = NULL;

        // Launch btest_server, 32-bit binary and all
        if (execv(SERVER_PROG, server_argv) == -1) {
            perror("execv");
            return 1;
        }
        // Successful exec does not return
    } else if (child_pid == -1) {
        perror("fork");
        shmdt(sh_buf);
        return 1;
    }

    // Print header
    printf("Score\tRating\tErrors\tFunction\n");

    // Converse with server as long as needed
    while (1) {
        // Wait until server has sent us something
        if (sema_wait(&sh_buf->ready_for_client) == -1) {
            perror("sema_wait");
            shmdt(sh_buf);
            return 1;
        }

        switch (sh_buf->type) {
            case TEST_INPUT_BATCH: {
                test_batch_t *test_batch = (test_batch_t *) sh_buf->payload;
                if (test_batch->previous_outcome != ONGOING) {
                    reportFunctionResult(test_batch->previous_outcome, &test_batch->previous_result);
                }

                int rc = sigsetjmp(envbuf, 1);
                if (rc == 1) {
                    // We jumped here due to a timeout in student func execution
                    sh_buf->type = TIMEOUT_FAILURE;
                    break;
                } else if (rc == 2) {
                    // We jumped here due to a segfault in stuent func execution
                    sh_buf->type = SEGFAULT_FAILURE;
                    break;
                } else if (rc == 3) {
                    // We jumped here due to a floating point exception in student func execution
                    sh_buf->type = SIGFPE_FAILURE;
                }

                unsigned num_args = getNumArgs(test_batch->function_id);
                if (num_args == -1) {
                    printf("Error: Invalid function ID received from server\n");
                    shmdt(sh_buf);
                    return 1;
                }

                // Read in elements from payload's buffer.
                // They occur in chunks of num_args + 1
                // Each element has space for all arguments plus a result
                int arg1;
                int arg2;
                for (int i = 0; i < test_batch->n_test_cases; i++) {
                    int j = i * (num_args + 1);
                    arg1 = test_batch->elems[j];
                    if (num_args == 2) {
                        j++;
                        arg2 = test_batch->elems[j];
                    }
                    j++; // j now refers to location for function's output

                    // Set up alarm for timeout enforcement
                    if (timeout_limit > 0) {
                        alarm(timeout_limit);
                    }

                    switch (test_batch->function_id) {
                        case BIT_MATCH: {
                            test_batch->elems[j] = bitMatch(arg1, arg2);
                            break;
                        }
                        case EVEN_BITS: {
                            test_batch->elems[j] = evenBits();
                            break;
                        }
                        case ALL_ODD_BITS: {
                            test_batch->elems[j] = allOddBits(arg1);
                            break;
                        }
                        case FLOAT_ABS_VAL: {
                            test_batch->elems[j] = floatAbsVal(arg1);
                            break;
                        }
                        case IMPLICATION: {
                            test_batch->elems[j] = implication(arg1, arg2);
                            break;
                        }
                        case IS_NEGATIVE: {
                            test_batch->elems[j] = isNegative(arg1);
                            break;
                        }
                        case SIGN: {
                            test_batch->elems[j] = sign(arg1);
                            break;
                        }
                        case IS_GREATER: {
                            test_batch->elems[j] = isGreater(arg1, arg2);
                            break;
                        }
                        case LOGICAL_SHIFT: {
                            test_batch->elems[j] = logicalShift(arg1, arg2);
                            break;
                        }
                        case ROTATE_RIGHT: {
                            test_batch->elems[j] = rotateRight(arg1, arg2);
                            break;
                        }
                        case FLOAT_SCALE_4: {
                            test_batch->elems[j] = floatScale4(arg1);
                            break;
                        }
                        case GREATEST_BIT_POS: {
                            test_batch->elems[j] = greatestBitPos(arg1);
                            break;
                        }
                        default: {
                            // Should never happen
                            printf("Error: Received invalid function ID from server\n");
                            shmdt(sh_buf);
                            return 1;
                        }
                    }
                }
                // Cancel alarm
                alarm(0);
                // Prep reply to server
                sh_buf->type = TEST_RESULT_BATCH;
                break;
            }

            case END: {
                // We're done and can terminate
                // But first need to check on result of previous function's tests
                test_batch_t *test_batch = (test_batch_t *) sh_buf->payload;
                assert(test_batch->previous_outcome != ONGOING);
                reportFunctionResult(test_batch->previous_outcome, &test_batch->previous_result);

                printf("Total points: %d/%d\n", total_points_earned, total_points_possible);

                int exit_status = 0;
                if (shmdt(sh_buf) == -1) {
                    perror("shmdt");
                    exit_status = 1;
                }
                // Wait for server to terminate
                if (wait(NULL) == -1) {
                    perror("wait");
                    exit_status = 1;
                }
                return exit_status;
            }

            default: {
                // Should never happen
                printf("Error: Invalid message type received from server\n");
                shmdt(sh_buf);
                return 1;
            }
        }

        // Indicate to server that client's reply is ready
        if (sema_post(&sh_buf->ready_for_server) == -1) {
            perror("sem_post");
            shmdt(sh_buf);
            return 1;
        }
    }
}
