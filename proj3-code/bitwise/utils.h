#ifndef UTILS_H
#define UTILS_H

// Features code from Randy Bryant and Dave O'Halloran's original "Data Lab"

/*
 * Extract hex/decimal or float value from string
 * *valp MUST be initialized to 0
 * Returns 1 if value successfully parsed, 0 on failure
 */
int get_num_val(char *sval, unsigned *valp);

unsigned getNumArgs(enum function_id id);

const char *getFuncName(enum function_id id);

unsigned getFuncRating(enum function_id id);

int getFuncMinArg(enum function_id id, int arg_pos);

int getFuncMaxArg(enum function_id id, int arg_pos);

enum function_id getFuncId(const char *name);

#endif // UTILS_H
