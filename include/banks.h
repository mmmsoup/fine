#ifndef BANKS_H
#define BANKS_H

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "common.h"
#include "date.h"
#include "csv.h"

#include "gperf/tcat.h"
#include "gperf/ttype.h"
#include "gperf/ttype_nationwide.h"
#include "gperf/ttype_natwest.h"

#define TRANSACTIONS_ALLOC_NUM 256

int parse_stdfmt(char *, transaction_list_t *, ttype_t, ttype_t);
extern int parse_stdfmt_acct(char *, transaction_list_t *);
extern int parse_stdfmt_cash(char *, transaction_list_t *);

int parse_nationwide(char *, transaction_list_t *);
int parse_natwest(char *, transaction_list_t *);

#endif
