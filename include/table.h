#ifndef TABLE_H
#define TABLE_H

#include <stdio.h>

#include "common.h"
#include "date.h"

#include "gperf/division_types.h"

#define TABLE_HLINE '-'
#define TABLE_VLINE '|'
#define TABLE_CROSS '+'

/*
#define TABLE_HLINE '━'
#define TABLE_VLINE '┃'
#define TABLE_CROSS '╋'
*/

int print_table(transaction_list_collection_t, int, int);

#endif
