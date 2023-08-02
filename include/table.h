#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stdio.h>
#include <wchar.h>

#include "common.h"
#include "date.h"

#include "gperf/division_types.h"

/*
#define TABLE_HLINE '-'
#define TABLE_VLINE '|'
#define TABLE_CROSS '+'
*/

#define TABLE_HLINE			L'━'
#define TABLE_VLINE			L'┃'
#define TABLE_TOPLEFT		L'┏'
#define TABLE_BOTTOMLEFT	L'┗'
#define TABLE_TOPRIGHT		L'┓'
#define TABLE_BOTTOMRIGHT	L'┛'
#define TABLE_CROSS			L'╋'

typedef enum {
	TABLEFLAG_COLOUR = 0b00000001
} table_flags;
typedef uint8_t table_flags_t;

int print_table(transaction_list_collection_t, int, int, table_flags_t);

#endif
