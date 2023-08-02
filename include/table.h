#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
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

typedef enum {
	TABLEFLAG_COLOUR = 0b00000001
} table_flags;
typedef uint8_t table_flags_t;

int print_table(transaction_list_collection_t, int, int, table_flags_t);

#endif
