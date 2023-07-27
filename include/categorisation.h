#ifndef CATEGORISATION_H
#define CATEGORISATION_H

#include <errno.h>
#include <regex.h>
#include <stdlib.h>

#include "common.h"
#include "csv.h"

#include "gperf/tcat.h"
#include "gperf/ttype.h"

#define CATRULES_ALLOC_NUM	128

typedef struct {
	regex_t regex;
	ttype_t ttype;
	tcat_t tcat;
} catrule_t;

typedef struct {
	catrule_t *rules;
	size_t size;
	size_t capacity;
} catrule_list_t;

int load_categorisation_rules(catrule_list_t *, char *);

tcat_t categorise_transaction(catrule_list_t *, transaction_t *);

#endif
