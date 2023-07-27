#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "date.h"

#include "gperf.h"
#include "gperf/tcat.h"
#include "gperf/ttype.h"

#define MAX(a,b) (a < b ? b : a)
#define MIN(a,b) (a > b ? b : a)

typedef struct {
	int amount;
	date_t date;
	char *description;
	ttype_t type;
	tcat_t category;
} transaction_t;

typedef struct {
	transaction_t *transactions;
	char *name;
	size_t size;
	size_t capacity;
	date_t daterange[2]; // 0: earliest, 1: latest
} transaction_list_t;

typedef struct {
	transaction_list_t *lists;
	size_t num_accounts;
	date_t daterange[2]; // 0: earliest, 1: latest
} transaction_list_collection_t;

int transaction_list_create(transaction_list_t *, char *, size_t);
int transaction_list_resize(transaction_list_t *, size_t);
int transaction_list_destroy(transaction_list_t *);

int exponent(int, unsigned int);

unsigned int intstrlen(unsigned int);

int parse_string_price(char *);

int remove_escapes(char *);

#endif
