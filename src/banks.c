#include "banks.h"

int parse_nationwide(char *path, transaction_list_t *list) {
	csv_t csv;
	if (csv_open(&csv, path) != EXIT_SUCCESS) {
		fprintf(stderr, "%s\n", strerror(errno));
		return 0;
	}

	csv_line_t line;
	csv_next_line(&csv, &line);
	transaction_list_create(list, line.fields[1], TRANSACTIONS_ALLOC_NUM);

	csv_skip_lines(&csv, 4);

	transaction_t *transaction = list->transactions;
	while (csv_next_line(&csv, &line)) {
		if (list->size == list->capacity) {
			transaction_list_resize(list, list->capacity+TRANSACTIONS_ALLOC_NUM);
			transaction = list->transactions + list->size;
		}

		const gsiv_t *gsiv = ttype_lookup_nationwide(line.fields[1], strlen(line.fields[1]));
		transaction->type = gsiv != NULL ? (ttype_t)(gsiv->value) : TTYPE_UNKNOWN;
		char *endptr;
		if (line.fields[3][0] == '\0') {
			transaction->type |= TTYPE_CREDIT;
			transaction->amount = parse_string_price(line.fields[4]+1);
		} else {
			transaction->type |= TTYPE_DEBIT;
			transaction->amount = -parse_string_price(line.fields[3]+1);
		}
		transaction->description = malloc(sizeof(char)*(strlen(line.fields[2])+1));
		strcpy(transaction->description, line.fields[2]);
		transaction->category = TCAT_NONE;

		transaction->date.day = (line.fields[0][0]-0x30)*10 + (line.fields[0][1]-0x30);
		transaction->date.month = month_abbr_lookup(line.fields[0]+3)->value;
		transaction->date.year = (line.fields[0][7]-0x30)*1000 + (line.fields[0][8]-0x30)*100 + (line.fields[0][9]-0x30)*10 + (line.fields[0][10]-0x30);
		transaction->date.days_since_epoch = days_since_epoch(transaction->date.day, transaction->date.month, transaction->date.year);

		csv_free_line(&line);
		list->size++;
		transaction++;
	}

	list->daterange[0] = list->transactions[0].date;
	list->daterange[1] = list->transactions[list->size-1].date;

	return EXIT_SUCCESS;
}

int parse_natwest(char *path, transaction_list_t *list) {
	csv_t csv;
	if (csv_open(&csv, path) != EXIT_SUCCESS) {
		fprintf(stderr, "%s\n", strerror(errno));
		return 0;
	}
	
	csv_skip_lines(&csv, 3);

	csv_line_t line;
	csv_next_line(&csv, &line);
	transaction_list_create(list, line.fields[6], TRANSACTIONS_ALLOC_NUM);

	transaction_t *transaction = list->transactions;
	do {
		if (list->size == list->capacity) {
			transaction_list_resize(list, list->capacity+TRANSACTIONS_ALLOC_NUM);
			transaction = list->transactions + list->size;
		}

		const gsiv_t *gsiv = ttype_lookup_natwest(line.fields[1], 3);
		transaction->type = gsiv != NULL ? (ttype_t)(gsiv->value) : TTYPE_UNKNOWN;
		transaction->amount = parse_string_price(line.fields[3]);
		transaction->category = TCAT_NONE;

		// remove card digits, date, and sometimes 'C' from start of each card transaction
		if (transaction->type == TTYPE_CARD_CREDIT || transaction->type == TTYPE_CARD_DEBIT) {
			if (*(line.fields[2]) != '\0') {
				char *ptr = line.fields[2] + 1;
				while (*ptr != '\0') {
					if (*ptr == ',') {
						if (*(ptr-1) == ' ' && *(ptr+1) == ' ') {
							line.fields[2] = ptr+2;
							break;
						}
					}
					ptr++;
				}
			}
		}

		transaction->description = malloc(sizeof(char)*(strlen(line.fields[2])+1));
		strcpy(transaction->description, line.fields[2]);

		transaction->date.day = (line.fields[0][0]-0x30)*10 + (line.fields[0][1]-0x30);
		transaction->date.month = (line.fields[0][3]-0x30)*10 + (line.fields[0][4]-0x30);
		transaction->date.year = (line.fields[0][6]-0x30)*1000 + (line.fields[0][7]-0x30)*100 + (line.fields[0][8]-0x30)*10 + (line.fields[0][9]-0x30);
		transaction->date.days_since_epoch = days_since_epoch(transaction->date.day, transaction->date.month, transaction->date.year);

		csv_free_line(&line);
		list->size++;
		transaction++;
	} while (csv_next_line(&csv, &line));

	list->daterange[0] = list->transactions[list->size-1].date;
	list->daterange[1] = list->transactions[0].date;

	return EXIT_SUCCESS;
}
