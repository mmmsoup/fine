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

#define TABLE_HLINE			L'─'
#define TABLE_VLINE			L'│'
#define TABLE_TOPLEFT		L'┌'
#define TABLE_BOTTOMLEFT	L'└'
#define TABLE_TOPRIGHT		L'┐'
#define TABLE_BOTTOMRIGHT	L'┘'
#define TABLE_CROSS			L'┼'

#define TABLE_HLINE_BOLD		L'━'
#define TABLE_VLINE_BOLD		L'┃'
#define TABLE_TOPLEFT_BOLD		L'┏'
#define TABLE_BOTTOMLEFT_BOLD	L'┗'
#define TABLE_TOPRIGHT_BOLD		L'┓'
#define TABLE_BOTTOMRIGHT_BOLD	L'┛'
#define TABLE_CROSS_BOLD		L'╋'

#define TABLE_CROSS_BOLDHORI	L'┿'

int print_table(transaction_list_collection_t, int, int, flags_t);

#endif
