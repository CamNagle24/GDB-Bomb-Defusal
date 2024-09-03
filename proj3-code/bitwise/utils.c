#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "dl_protocol.h"
#include "utils.h"

// Features code from Randy Bryant and Dave O'Halloran's original "Data Lab"

int get_num_val(char *sval, unsigned *valp) {
    char *endp;

    // See if it's an integer or floating point
    int ishex = 0;
    int isfloat = 0;
    for (int i = 0; sval[i] != '\0'; i++) {
        switch (sval[i]) {
            case 'x':
            case 'X':
                ishex = 1;
                break;
            case 'e':
            case 'E':
                if (!ishex) {
                    isfloat = 1;
                }
                break;
            case '.':
                isfloat = 1;
                break;
            default:
                break;
        }
    }

    if (isfloat) {
        float fval = strtof(sval, &endp);
        if (!*endp) {
            *valp = *(unsigned *) &fval;
            return 1;
        }
        return 0;
    } else {
        long long int llval = strtoll(sval, &endp, 0);
        long long int upperbits = llval >> 31;
        /* will give -1 for negative, 0 or 1 for positive */
        if (!*valp && (upperbits == 0 || upperbits == -1 || upperbits == 1)) {
            *valp = (unsigned) llval;
            return 1;
        }
        return 0;
    }
}

// Get the number of arguments associated with each function
unsigned getNumArgs(enum function_id id) {
    switch (id) {
        case EVEN_BITS: {
            return 0;
        }

        case ALL_ODD_BITS:
        case FLOAT_ABS_VAL:
        case IS_NEGATIVE:
        case SIGN:
        case FLOAT_SCALE_4:
        case GREATEST_BIT_POS: {
            return 1;
        }

        case BIT_MATCH:
        case IMPLICATION:
        case IS_GREATER:
        case LOGICAL_SHIFT:
        case ROTATE_RIGHT: {
            return 2;
        }
    }

    // Not reached
    return -1;
}

// Get name of a function from its ID value
const char *getFuncName(enum function_id id) {
    switch (id) {
        case EVEN_BITS:
            return "evenBits";
        case ALL_ODD_BITS:
            return "allOddBits";
        case FLOAT_ABS_VAL:
            return "floatAbsVal";
        case IS_NEGATIVE:
            return "isNegative";
        case SIGN:
            return "sign";
        case FLOAT_SCALE_4:
            return "floatScale4";
        case GREATEST_BIT_POS:
            return "greatestBitPos";
        case BIT_MATCH:
            return "bitMatch";
        case IMPLICATION:
            return "implication";
        case IS_GREATER:
            return "isGreater";
        case LOGICAL_SHIFT:
            return "logicalShift";
        case ROTATE_RIGHT:
            return "rotateRight";
    }

    // Not reached
    return NULL;
}

enum function_id getFuncId(const char *name) {
    if (strcmp(name, "bitMatch") == 0) {
        return BIT_MATCH;
    } else if (strcmp(name, "evenBits") == 0) {
        return  EVEN_BITS;
    } else if (strcmp(name, "allOddBits") == 0) {
        return ALL_ODD_BITS;
    } else if (strcmp(name, "floatAbsVal") == 0) {
        return FLOAT_ABS_VAL;
    } else if (strcmp(name, "implication") == 0) {
        return IMPLICATION;
    } else if (strcmp(name, "isNegative") == 0) {
        return IS_NEGATIVE;
    } else if (strcmp(name, "sign") == 0) {
        return SIGN;
    } else if (strcmp(name, "isGreater") == 0) {
        return IS_GREATER;
    } else if (strcmp(name, "logicalShift") == 0) {
        return LOGICAL_SHIFT;
    } else if (strcmp(name, "rotateRight") == 0) {
        return ROTATE_RIGHT;
    } else if (strcmp(name, "floatScale4") == 0) {
        return FLOAT_SCALE_4;
    } else if (strcmp(name, "greatestBitPos") == 0) {
        return GREATEST_BIT_POS;
    }

    // Should never be reached
    return 0;
}

// Get function's difficulty rating
unsigned getFuncRating(enum function_id id) {
    switch (id) {
        case BIT_MATCH:
        case EVEN_BITS:
            return 1;

        case ALL_ODD_BITS:
        case FLOAT_ABS_VAL:
        case IMPLICATION:
        case IS_NEGATIVE:
        case SIGN:
            return 2;

        case IS_GREATER:
        case LOGICAL_SHIFT:
        case ROTATE_RIGHT:
            return 3;

        case FLOAT_SCALE_4:
        case GREATEST_BIT_POS:
            return 4;
    }

    // Not reached
    return 0;
}

int getFuncMinArg(enum function_id id, int arg_pos) {
    switch (arg_pos) {
        case 1: {
            switch (id) {
                case BIT_MATCH:
                case ALL_ODD_BITS:
                case IS_NEGATIVE:
                case SIGN:
                case IS_GREATER:
                case LOGICAL_SHIFT:
                case ROTATE_RIGHT:
                case GREATEST_BIT_POS:
                    return INT_MIN;

                case IMPLICATION:
                    return 0;


                default:
                    // Should never happen. All other puzzles are float-based or take 0 args
                    return -1;
            }
        }

        case 2:
            switch (id) {
                case BIT_MATCH:
                case IS_GREATER:
                    return INT_MIN;

                case LOGICAL_SHIFT:
                case IMPLICATION:
                case ROTATE_RIGHT:
                    return 0;

                default:
                    // Should never happen. All other puzzles are float-based or require only 0/1 args
                    return -1;
            }

        default:
            // Should never happen, all puzzles use 1 or 2 arguments
            return -1;
    }

    // Not reached
    return -1;
}

int getFuncMaxArg(enum function_id id, int arg_pos) {
    switch (arg_pos) {
        case 1: {
            switch (id) {
                case BIT_MATCH:
                case ALL_ODD_BITS:
                case IS_NEGATIVE:
                case SIGN:
                case IS_GREATER:
                case LOGICAL_SHIFT:
                case ROTATE_RIGHT:
                case GREATEST_BIT_POS:
                    return INT_MAX;

                case IMPLICATION:
                    return 1;


                default:
                    // Should never happen. All other puzzles are float-based or take 0 args
                    return -1;
            }
        }

        case 2:
            switch (id) {
                case BIT_MATCH:
                case IS_GREATER:
                    return INT_MAX;

                case IMPLICATION:
                    return 1;

                case LOGICAL_SHIFT:
                case ROTATE_RIGHT:
                    return 31;

                default:
                    // Should never happen. All other puzzles are float-based or require only 0/1 args
                    return -1;
            }

        default:
            // Should never happen, all puzzles use 1 or 2 arguments
            return -1;
    }
}
