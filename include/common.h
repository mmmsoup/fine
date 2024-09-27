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

#define ABS(a) (a < 0 ? -(a) : a)

#define ESC_BOLD		L"\e[1m"
#define ESC_POSITIVE	L"\e[32m"
#define ESC_NEGATIVE	L"\e[31m"
#define ESC_ZERO		L"\e[30m"
#define ESC_END			L"\e[0m"
#define ESC_NONE		L""

typedef enum {
	FLAG_COLOUR = 0b00000001
} flags;
typedef uint8_t flags_t;

typedef struct {
	int amount;
	date_t date;
	char *description;
	ttype_t type;
	tcat_t category;
} transaction_t;

// transactions always in date order (oldest to newest)
typedef struct {
	transaction_t *transactions;
	char *name;
	char *bank;
	char *file;
	size_t size;
	size_t capacity;
	int balance;
} transaction_list_t;

typedef struct {
	transaction_list_t *lists;
	size_t num_accounts;
	date_t daterange[2]; // 0: earliest, 1: latest
} transaction_list_collection_t;

int transaction_list_create(transaction_list_t *, char *, char *, char *, size_t);
int transaction_list_resize(transaction_list_t *, size_t);
int transaction_list_destroy(transaction_list_t *);

int exponent(int, unsigned int);

unsigned int intstrlen(unsigned int);

int parse_string_price(char *);

int remove_escapes(char *);

#endif
