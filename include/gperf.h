#ifndef GPERF_H
#define GPERF_H

#include <stdint.h>

// gperf single integer value
struct gsiv_t {
	char *name;
	long long value;
};
typedef struct gsiv_t gsiv_t;

#define TTYPE_CREDIT	0b10000000
#define TTYPE_DEBIT		0b00000000
#define TTYPE_UNKNOWN	0b01111111

typedef enum {
	TTYPE_ACCT_CREDIT		= 0b00000001 | TTYPE_CREDIT,
	TTYPE_ACCT_DEBIT		= 0b00000010 | TTYPE_DEBIT,
	TTYPE_CARD_CREDIT		= 0b00000100 | TTYPE_CREDIT,
	TTYPE_CARD_DEBIT		= 0b00001000 | TTYPE_DEBIT,
	TTYPE_CASH_CREDIT		= 0b00010000 | TTYPE_CREDIT,
	TTYPE_CASH_DEBIT		= 0b00100000 | TTYPE_DEBIT,
	TTYPE_CHEQUE			= 0b01000000 | TTYPE_CREDIT
} transaction_types;
typedef uint8_t ttype_t;

#define IS_CREDIT(ttype) { return ttype & TTYPE_CREDIT; }

const char *get_ttype_pretty_string(ttype_t);

#endif
