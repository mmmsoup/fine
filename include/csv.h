#ifndef CSV_H
#define CSV_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

// lines in csv files beginning with an '#' will be returned as blank lines by csv_next_line
#define CSV_COMMENTS 1

typedef struct {
	FILE *file;
} csv_t;

typedef struct {
	char *ptr;
	char **fields;
	size_t num_fields;
} csv_line_t;

int csv_open(csv_t *, char *);
int csv_close(csv_t *);

int csv_next_line(csv_t *, csv_line_t *);
int csv_free_line(csv_line_t *);
int csv_skip_lines(csv_t *, unsigned int);

#endif
