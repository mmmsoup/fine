#include "banks.h"

int parse_cash(char *path, transaction_list_t *list) {
	csv_t csv;
	if (csv_open(&csv, path) != EXIT_SUCCESS) {
		fprintf(stderr, "csv_open(): %s\n", strerror(errno));
		return errno;
	}

	transaction_list_create(list, "Cash Transactions", "Cash", path, TRANSACTIONS_ALLOC_NUM);
	list->balance = 0;

	csv_line_t line;
	transaction_t *transaction = list->transactions;
	while (csv_next_line(&csv, &line)) {
		if (list->size == list->capacity) {
			transaction_list_resize(list, list->capacity+TRANSACTIONS_ALLOC_NUM);
			transaction = list->transactions + list->size;
		}

		transaction->date.year	= (line.fields[0][0]-0x30)*1000 + (line.fields[0][1]-0x30)*100 + (line.fields[0][2]-0x30)*10 + (line.fields[0][3]-0x30);
		transaction->date.month	= (line.fields[0][5]-0x30)*10 + (line.fields[0][6]-0x30);
		transaction->date.day	= (line.fields[0][8]-0x30)*10 + (line.fields[0][9]-0x30);
		transaction->date.days_since_epoch = days_since_epoch(transaction->date.day, transaction->date.month, transaction->date.year);

		transaction->amount = parse_string_price(line.fields[1]);
		transaction->type = transaction->amount < 0 ? TTYPE_CASH_DEBIT : TTYPE_CASH_CREDIT;

		transaction->description = malloc(sizeof(char)*(strlen(line.fields[2])+1));
		strcpy(transaction->description, line.fields[2]);

		transaction->category = TCAT_NONE;
		if (line.num_fields == 4) {
			const gsiv_t *gsiv = tcat_lookup(line.fields[3], strlen(line.fields[3]));
			if (gsiv == NULL) fwprintf(stderr, L"invalid transaction category '%s'\n", line.fields[3]);
			else transaction->category = gsiv->value;
		}

		list->balance += transaction->amount;

		csv_free_line(&line);
		list->size++;
		transaction++;
	}

	csv_close(&csv);

	return EXIT_SUCCESS;
}

int parse_nationwide(char *path, transaction_list_t *list) {
	csv_t csv;
	if (csv_open(&csv, path) != EXIT_SUCCESS) {
		fprintf(stderr, "csv_open(): %s\n", strerror(errno));
		return errno;
	}

	csv_line_t line;
	csv_next_line(&csv, &line);
	transaction_list_create(list, line.fields[1], "Nationwide", path, TRANSACTIONS_ALLOC_NUM);
	csv_free_line(&line);

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

		transaction->date.day	= (line.fields[0][0]-0x30)*10 + (line.fields[0][1]-0x30);
		transaction->date.month	= month_abbr_lookup(line.fields[0]+3)->value;
		transaction->date.year	= (line.fields[0][7]-0x30)*1000 + (line.fields[0][8]-0x30)*100 + (line.fields[0][9]-0x30)*10 + (line.fields[0][10]-0x30);
		transaction->date.days_since_epoch = days_since_epoch(transaction->date.day, transaction->date.month, transaction->date.year);

		list->balance = parse_string_price(line.fields[5]+1);

		csv_free_line(&line);
		list->size++;
		transaction++;
	}

	csv_close(&csv);

	return EXIT_SUCCESS;
}

int parse_natwest(char *path, transaction_list_t *list) {
	csv_t csv;
	if (csv_open(&csv, path) != EXIT_SUCCESS) {
		fprintf(stderr, "csv_open(): %s\n", strerror(errno));
		return errno;
	}
	
	csv_skip_lines(&csv, 1);

	csv_line_t line;
	csv_next_line(&csv, &line);
	transaction_list_create(list, line.fields[6], "Natwest", path, TRANSACTIONS_ALLOC_NUM);
	list->balance = parse_string_price(line.fields[4]);

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

		transaction->date.day	= (line.fields[0][0]-0x30)*10 + (line.fields[0][1]-0x30);
		transaction->date.month	= month_abbr_lookup(line.fields[0]+3)->value;
		transaction->date.year	= (line.fields[0][7]-0x30)*1000 + (line.fields[0][8]-0x30)*100 + (line.fields[0][9]-0x30)*10 + (line.fields[0][10]-0x30);
		transaction->date.days_since_epoch = days_since_epoch(transaction->date.day, transaction->date.month, transaction->date.year);

		csv_free_line(&line);
		list->size++;
		transaction++;
	} while (csv_next_line(&csv, &line));

	csv_close(&csv);

	transaction_t temp;
	for (int i = 0; i < list->size/2; i++) {
		memcpy(&temp, &(list->transactions[i]), sizeof(transaction_t));
		memcpy(&(list->transactions[i]), &(list->transactions[list->size-i-1]), sizeof(transaction_t));
		memcpy(&(list->transactions[list->size-i-1]), &temp, sizeof(transaction_t));
	}

	return EXIT_SUCCESS;
}
