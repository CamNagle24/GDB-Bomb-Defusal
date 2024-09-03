#ifndef DL_PROTOCOL_H
#define DL_PROTOCOL_H

#include <stdint.h>

// One GiB should be small enough for most systems, right?
#define MSG_BUF_SIZE 1073741824
#define NUM_PUZZLES 12

enum message_type {
    TEST_INPUT_BATCH,   // 1) Server -> Client. Batch of inputs to test on student implementation.
                        // Optionally with failure info from a previous pbatch.
    TEST_RESULT_BATCH,  // 2a) Client -> Server. Batch of results from student implementation.
    TIMEOUT_FAILURE,    // 2b) Client -> Server. Student function exceeded timeout.
    SEGFAULT_FAILURE,   // 2c) Client -> Server. Student function caused segmentation fault
    SIGFPE_FAILURE,     // 2d) Client -> Server. Student function caused floating point exception.
    END                 // 4) Server -> Client. All requested functions tested. Time to exit.
                        // Includes failure info from most recent batch.
};

enum function_id {
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

enum test_outcome {
    SUCCESS,            // Tests finished for previous function. Report pass.
    TIMEOUT,            // Previous tests encountered a timeout failure
    SEGFAULT,           // Previous tests encountered a segfault
    FLOAT_ERROR,        // Previous test caused a floating point exception
    FAILURE,            // Tests finished but incorrect result was found.
    ONGOING             // Tests still pending for current function. Nothing to report.
};

typedef struct {
    uint32_t ready_for_client;   // Is input ready for client to consume?
    uint32_t ready_for_server;   // Is input ready for server to consume?
    enum message_type type;      // Message type ID
    char payload[MSG_BUF_SIZE];  // Payload. Structure depends on message type.
} shmem_buf_t;

typedef struct {
    enum function_id function_id;   // ID of function with results
    int arg1;                       //  First argument for failure case (if applicable)
    int arg2;                       // Second argument for failure case (if applicable)
    int expected_output;            // Result expected from student function (if applicable)
    int actual_output;              // Result produced by student function (if applicable)
} function_result_t;

typedef struct {
    enum test_outcome previous_outcome; // Outcome to report for previous function?
    function_result_t previous_result;  // Information on results for previous function (if applicable)
    enum function_id function_id;       // Current function to be tested
    unsigned n_test_cases;              // Total number of test cases included in this batch
    int elems[];                        // Input test values (and space for outputs)
} test_batch_t;

#endif // DL_PROTOCOL_H
