/*
 * btest.c - A test harness that checks a student's solution in bits.c
 *           for correctness.
 *
 * Copyright (c) 2001-2011, R. Bryant and D. O'Hallaron, All rights
 * reserved.  May not be used, modified, or copied without permission.
 *
 * This is an improved version of btest that tests large windows
 * around zero and tmin and tmax for integer puzzles, and zero, norm,
 * and denorm boundaries for floating point puzzles.
 *
 * Note: not 64-bit safe. Always compile with gcc -m32 option.
 */
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include "bits_test.h"
#include "dl_protocol.h"
#include "utils.h"
#include "sema.h"

/* For functions with a single argument, generate TEST_RANGE values
   above and below the min and max test values, and above and below
   zero. Functions with two or three args will use square and cube
   roots of this value, respectively, to avoid combinatorial
   explosion */
#define TEST_RANGE 500000

/* This defines the maximum size of any test value array. The
   gen_vals() routine creates k test values for each value of
   TEST_RANGE, thus MAX_TEST_VALS must be at least k*TEST_RANGE */
#define MAX_TEST_VALS 13*TEST_RANGE

enum function_id all_funcs[] = {
    BIT_MATCH,
    EVEN_BITS,
    ALL_ODD_BITS,
    FLOAT_ABS_VAL,
    IMPLICATION,
    IS_NEGATIVE,
    SIGN,
    IS_GREATER,
    LOGICAL_SHIFT,
    ROTATE_RIGHT,
    FLOAT_SCALE_4,
    GREATEST_BIT_POS
};

/* If non-NULL, test only one function (-f) */
static char *test_fname = NULL;

/* Special case when only use fixed argument(s) (-1, -2, or -3) */
static int has_arg[] = {0, 0};
static unsigned argval[] = {0, 0};

// random_val - Return random integer value between min and max
static int random_val(int min, int max) {
    double weight = rand()/(double) RAND_MAX;
    int result = min * (1-weight) + max * weight;
    return result;
}

// gen_vals - Generate the integer values we'll use to test a function
static int gen_vals(int test_vals[], int min, int max, int test_range, int arg_pos, int is_float) {
    int test_count = 0;

    /* Special case: If the user has specified a specific function
       argument using the -1, -2, or -3 flags, then simply use this
       argument and return */
    if (has_arg[arg_pos] != 0) {
        test_vals[0] = argval[arg_pos];
        return 1;
    }

    /*
     * Special case: Generate test vals for floating point functions
     * where the input argument is an unsigned bit-level
     * representation of a float. For this case we want to test the
     * regions around zero, the smallest normalized and largest
     * denormalized numbers, one, and the largest normalized number,
     * as well as inf and nan.
     */
    if (is_float) {
        unsigned smallest_norm = 0x00800000;
        unsigned one = 0x3f800000;
        unsigned largest_norm = 0x7f000000;

        unsigned inf = 0x7f800000;
        unsigned nan =  0x7fc00000;
        unsigned sign = 0x80000000;

        /* Test range should be at most 1/2 the range of one exponent
           value */
        if (test_range > (1 << 23)) {
            test_range = 1 << 23;
        }

        /* Functions where the input argument is an unsigned bit-level
           representation of a float. The number of tests generated
           inside this loop body is the value k referenced in the
           comment for the global variable MAX_TEST_VALS. */
        for (int i = 0; i < test_range; i++) {
            /* Denorms around zero */
            test_vals[test_count++] = i;
            test_vals[test_count++] = sign | i;

            /* Region around norm to denorm transition */
            test_vals[test_count++] = smallest_norm + i;
            test_vals[test_count++] = smallest_norm - i;
            test_vals[test_count++] = sign | (smallest_norm + i);
            test_vals[test_count++] = sign | (smallest_norm - i);

            /* Region around one */
            test_vals[test_count++] = one + i;
            test_vals[test_count++] = one - i;
            test_vals[test_count++] = sign | (one + i);
            test_vals[test_count++] = sign | (one - i);

            /* Region below largest norm */
            test_vals[test_count++] = largest_norm - i;
            test_vals[test_count++] = sign | (largest_norm - i);
        }

        /* special vals */
        test_vals[test_count++] = inf;        /* inf */
        test_vals[test_count++] = sign | inf; /* -inf */
        test_vals[test_count++] = nan;        /* nan */
        test_vals[test_count++] = sign | nan; /* -nan */

        return test_count;
    }


    /*
     * Normal case: Generate test vals for integer functions
     */

    /* If the range is small enough, then do exhaustively */
    if (max - MAX_TEST_VALS <= min) {
        for (int i = min; i <= max; i++)
            test_vals[test_count++] = i;
        return test_count;
    }

    /* Otherwise, need to sample.  Do so near the boundaries, around
       zero, and for some random cases. */
    for (int i = 0; i < test_range; i++) {
        /* Test around the boundaries */
        test_vals[test_count++] = min + i;
        test_vals[test_count++] = max - i;

        /* If zero falls between min and max, then also test around zero */
        if (i >= min && i <= max) {
            test_vals[test_count++] = i;
        }
        if (-i >= min && -i <= max) {
            test_vals[test_count++] = -i;
        }
        /* Random case between min and max */
        test_vals[test_count++] = random_val(min, max);

    }
    return test_count;
}

void validate_test_results(int *batch_elems, unsigned batch_size, enum function_id func,
        unsigned num_args, enum test_outcome *previous_outcome, function_result_t *previous_result) {

    for (int i = 0; i < batch_size; i++) {
        int j = i * (num_args + 1);
        int arg1 = batch_elems[j];
        int arg2;
        j++;
        if (num_args == 2) {
            arg2 = batch_elems[j];
            j++;
        }
        int actual_result = batch_elems[j];

        int expected_result;
        switch (func) {
            case BIT_MATCH:
                expected_result = test_bitMatch(arg1, arg2);
                break;
            case EVEN_BITS:
                expected_result = test_evenBits();
                break;
            case ALL_ODD_BITS:
                expected_result = test_allOddBits(arg1);
                break;
            case FLOAT_ABS_VAL:
                expected_result = test_floatAbsVal(arg1);
                break;
            case IMPLICATION:
                expected_result = test_implication(arg1, arg2);
                break;
            case IS_NEGATIVE:
                expected_result = test_isNegative(arg1);
                break;
            case SIGN:
                expected_result = test_sign(arg1);
                break;
            case IS_GREATER:
                expected_result = test_isGreater(arg1, arg2);
                break;
            case LOGICAL_SHIFT:
                expected_result = test_logicalShift(arg1, arg2);
                break;
            case ROTATE_RIGHT:
                expected_result = test_rotateRight(arg1, arg2);
                break;
            case FLOAT_SCALE_4:
                expected_result = test_floatScale4(arg1);
                break;
            case GREATEST_BIT_POS:
                expected_result = test_greatestBitPos(arg1);
                break;
            default:
                // Should never happen
                fprintf(stderr, "Invalid function ID for validate_test_results\n");
                return;
        }

        if (actual_result != expected_result) {
            *previous_outcome = FAILURE;
            previous_result->function_id = func;
            previous_result->arg1 = arg1;
            previous_result->arg2 = arg2;
            previous_result->expected_output = expected_result;
            previous_result->actual_output = actual_result;
            return;
        }
    }
    *previous_outcome = SUCCESS;
    previous_result->function_id = func;
}

int run_test(shmem_buf_t *sh_buf, enum function_id func, enum test_outcome *previous_outcome,
        function_result_t *previous_result) {
    test_batch_t *test_batch = (test_batch_t *) sh_buf->payload;
    test_batch->function_id = func;
    unsigned num_args = getNumArgs(func);
    // Each test case needs to store arguments and another int for the result
    size_t remaining_payload_bytes = MSG_BUF_SIZE - (sizeof(enum test_outcome) + sizeof(function_result_t) +
            sizeof(enum function_id) + sizeof(unsigned));
    size_t max_batch_size = remaining_payload_bytes / (sizeof(int) * (num_args + 1));

    int test_counts[2];         /* number of test values for each arg */
    int arg_test_range[2] = {}; /* test range for each argument */

    /* These are the test values for each arg. Declared with the
       static attribute so that the array will be allocated in bss
       rather than the stack */
    static int arg_test_vals[2][MAX_TEST_VALS];
    /* Assign range of argument test vals so as to conserve the total
       number of tests, independent of the number of arguments */
    if (num_args == 0) {
        arg_test_range[0] = 0;
    } else if (num_args == 1) {
        arg_test_range[0] = TEST_RANGE;
    }
    else {
        assert(num_args == 2);
        arg_test_range[0] = pow((double)TEST_RANGE, 0.5);  /* sqrt */
        arg_test_range[1] = arg_test_range[0];
    }

    /* Sanity check on the ranges */
    if (arg_test_range[0] < 1) {
        arg_test_range[0] = 1;
    }
    if (arg_test_range[1] < 1) {
        arg_test_range[1] = 1;
    }

    /* Create a test set for each argument */
    for (int i = 0; i < num_args; i++) {
        int is_float;
        switch (func) {
            case FLOAT_ABS_VAL:
            case FLOAT_SCALE_4:
                is_float = 1;
                break;
            default:
                is_float = 0;
        }
        test_counts[i] =  gen_vals(arg_test_vals[i], getFuncMinArg(func, i+1),
                getFuncMaxArg(func, i+1), arg_test_range[i], i, is_float);
    }

    unsigned pending_batch_size = 0;
    unsigned batches_sent = 0;
    int a1 = 0;
    int a2 = 0;

    // Ugly hack to make 0-arg functions work
    // Need while loop immediately below to iterate at least one time
    int iterated_once = 0;

    while (!iterated_once || (a1 < test_counts[0] && (num_args == 1 || a2 < test_counts[1]))) {
        iterated_once = 1;
        while (pending_batch_size < max_batch_size && a1 < test_counts[0] && (num_args == 1 || a2 < test_counts[1])) {
            // Each test case takes up num_args + 1 int slots in memory, so this is our starting index for current test case
            int j = pending_batch_size * (num_args + 1);
            test_batch->elems[j] = arg_test_vals[0][a1];
            if (num_args == 2) {
                test_batch->elems[j+1] = arg_test_vals[1][a2];
            }
            pending_batch_size++;
            if (num_args == 2) {
                a2++;
                if (a2 == test_counts[1]) {
                    a2 = 0;
                    a1++;
                }
            } else {
                a1++;
            }
        }
        if (batches_sent == 0) {
            // If first batch of tests for new func, report results from previous function
            test_batch->previous_outcome = *previous_outcome;
            test_batch->previous_result = *previous_result;
        } else {
            // Otherwise, just tell client tests are still ongoing for same function
            test_batch->previous_outcome = ONGOING;
        }
        if (num_args == 0) {
            test_batch->n_test_cases = 1;
            pending_batch_size = 1;
        } else {
            test_batch->n_test_cases = pending_batch_size;
        }
        sh_buf->type = TEST_INPUT_BATCH;

        // Now notify client and wait for its reply
        if (sema_post(&sh_buf->ready_for_client) == -1) {
            perror("sema_post");
            return -1;
        }
        if (sema_wait(&sh_buf->ready_for_server) == -1) {
            perror("sema_wait");
            return -1;
        }

        switch (sh_buf->type) {
            case TIMEOUT_FAILURE: {
                *previous_outcome = TIMEOUT;
                previous_result->function_id = func;
                return 0;
            }
            case SEGFAULT_FAILURE: {
                *previous_outcome = SEGFAULT;
                previous_result->function_id = func;
                return 0;
            }
            case SIGFPE_FAILURE: {
                *previous_outcome = FLOAT_ERROR;
                previous_result->function_id = func;
                return 0;
            }
            case TEST_RESULT_BATCH: {
                validate_test_results(test_batch->elems, pending_batch_size, func, num_args,
                        previous_outcome, previous_result);
                if (*previous_outcome == FAILURE) {
                    return 0;
                }
                break;
            }
            default:
                // Should never happen
                fprintf(stderr, "run_test: Invalid message type received\n");
                return -1;
        }

        pending_batch_size = 0;
        batches_sent++;
    }

    return 0;
}

void run_tests(shmem_buf_t *sh_buf) {
    enum test_outcome previous_outcome = ONGOING;
    function_result_t previous_result;

    if (test_fname != NULL) {
        run_test(sh_buf, getFuncId(test_fname), &previous_outcome, &previous_result);
    } else {
        for (int i = 0; i < NUM_PUZZLES; i++) {
            run_test(sh_buf, all_funcs[i], &previous_outcome, &previous_result);
        }
    }

    sh_buf->type = END;
    test_batch_t *test_batch = (test_batch_t  *)sh_buf->payload;
    test_batch->previous_outcome = previous_outcome;
    test_batch->previous_result = previous_result;
    if (sema_post(&sh_buf->ready_for_client) == -1) {
        perror("sema_post");
        // No need for further cleanup at this point - that's handled in main()
    }
}

int main(int argc, char *argv[]) {
    assert(argc >= 2);
    int shmid = atoi(argv[1]);
    shmem_buf_t *sh_buf = shmat(shmid, NULL, 0);
    if (sh_buf == (void *)-1) {
        perror("shmat");
        return 1;
    }

    char c;
    while ((c = getopt(argc, argv, "hgf:T:1:2:3:")) != -1)
        switch (c) {
        case 'h': /* help */
            // Handled by client, ignore it here
        break;
    case 'g': /* grading option for autograder */
        // Handled by client, ignore it here
        break;
    case 'f': /* test only one function */
        test_fname = strdup(optarg);
        break;
    case '1': /* Get first argument */
        has_arg[0] = get_num_val(optarg, &argval[0]);
        if (!has_arg[0]) {
            exit(1);
        }
        break;
    case '2': /* Get second argument */
        has_arg[1] = get_num_val(optarg, &argval[1]);
        if (!has_arg[1]) {
            exit(1);
        }
        break;
    case 'T': /* Set timeout limit */
        // Handled by client, ignore it here
        break;
    default:
        // Shouldn't happen as any errors caught by cilent
        exit(1);
    }

    /* test each function */
    run_tests(sh_buf);
    if (shmdt(sh_buf) == -1) {
        perror("shmdt");
        return 1;
    }

    return 0;
}
