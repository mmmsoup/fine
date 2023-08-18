#include "categorisation.h"

int load_categorisation_rules(catrule_list_t *list, char *path) {
	csv_t csv;
	if (csv_open(&csv, path) != EXIT_SUCCESS) {
		fwprintf(stderr, L"csv_open(): %s\n", strerror(errno));
		return errno;
	}

	list->rules = malloc(sizeof(catrule_t)*CATRULES_ALLOC_NUM);
	list->capacity = CATRULES_ALLOC_NUM;
	list->size = 0;

	csv_line_t line;
	catrule_t *rule = list->rules;
	while (csv_next_line(&csv, &line)) {
		if (line.num_fields == 0) continue;
		else if (line.num_fields != 3) {
			fwprintf(stderr, L"skipping rule with invalid number of fields (expected 3, got %li)\n", line.num_fields);
			continue;
		}

		if (list->size == list->capacity) {
			list->capacity += CATRULES_ALLOC_NUM;
			list->rules = realloc(list->rules, sizeof(catrule_t)*list->capacity);
			rule = list->rules + list->size;
		}

		if (regcomp(&(rule->regex), line.fields[0], 0) != EXIT_SUCCESS) {
			fwprintf(stderr, L"skipping invalid regular expression /%s/\n", line.fields[0]);
		} else {
			rule->ttype = 0;
			char *start = line.fields[1];
			char *end = start;
			while (1) {
				if (*end == ';') {
					*end = '\0';
					const gsiv_t *gsiv = ttype_lookup(start, end-start);
					if (gsiv == NULL) fwprintf(stderr, L"skipping invalid transaction type '%s'\n", start);
					else rule->ttype |= (ttype_t)(gsiv->value);
					start = end+1;
				} else if (*end == '\0') {
					const gsiv_t *gsiv = ttype_lookup(start, end-start);
					if (gsiv == NULL) fwprintf(stderr, L"skipping invalid transaction type '%s'\n", start);
					else rule->ttype |= (ttype_t)(gsiv->value);
					break;
				}
				end++;
			}
			rule->ttype = (rule->ttype | TTYPE_CREDIT) - TTYPE_CREDIT;

			const gsiv_t *gsiv = tcat_lookup(line.fields[2], strlen(line.fields[2]));
			if (gsiv == NULL) {
				fwprintf(stderr, L"skipping invalid transaction category '%s'\n", line.fields[2]);
				regfree(&(rule->regex));
			} else {
				rule->tcat = (tcat_t)(gsiv->value);
				list->size++;
				rule++;
			}
		}
	}
	
	return EXIT_SUCCESS;
}

int free_categorisation_rules(catrule_list_t *list) {
	for (int i = 0; i < list->size; i++) {
		regfree(&(list->rules[i].regex));
	}
	return EXIT_SUCCESS;
}

tcat_t categorise_transaction(catrule_list_t *list, transaction_t *transaction) {
	if (transaction->category != TCAT_NONE) return transaction->category;

	for (int i = 0; i < list->size; i++) {
		if (regexec(&(list->rules[i].regex), transaction->description, 0, NULL, 0) == 0) {
			if (list->rules[i].ttype & transaction->type) {
				transaction->category = list->rules[i].tcat;
				break;
			}
		}
	}

	return transaction->category;
}
