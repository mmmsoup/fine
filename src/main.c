#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "banks.h"
#include "categorisation.h"
#include "common.h"
#include "date.h"
#include "table.h"

#define ASSERT_OPTION_HAS_ARGUMENT(argc, argv, i) { \
	if ((i+1) == argc) { \
		fwprintf(stderr, L"expected argument for option '%s'\n", argv+i); \
		return EXIT_FAILURE; \
	} \
} \

int main(int argc, char **argv) {
	setlocale(LC_ALL, "en_GB.UTF-8");

	const enum { OUTPUT_TABLE, OUTPUT_UNCATEGORISED } output_types;

	int output_type = OUTPUT_TABLE;
	int date_format = DATEFMT_NONE;
	int division_type = DIV_MONTHS;

	flags_t flags = FLAG_COLOUR;

	catrule_list_t rules;
	transaction_list_collection_t collection;
	collection.num_accounts = 0;
	transaction_list_t transactions[argc]; // this will always be plenty
	collection.lists = transactions;

	gsiv_t *gsiv;
	int i = 1;
	while (i < argc) {
		if (argv[i][0] == '-' && argv[i][1] == '-') {
			argv[i] += 2;
			if (strcmp(argv[i], "config") == 0) {
				ASSERT_OPTION_HAS_ARGUMENT(argc, argv, i);
				i++;
				if (load_categorisation_rules(&rules, argv[i]) != EXIT_SUCCESS) {
					fwprintf(stderr, L"unable to parse config file '%s'\n", argv[i]);
					return EXIT_FAILURE;
				}
			} else if (strcmp(argv[i], "date-format") == 0) {
				ASSERT_OPTION_HAS_ARGUMENT(argc, argv, i);
				i++;
				gsiv = (gsiv_t*)date_format_lookup(argv[i], strlen(argv[i]));
				if (gsiv == NULL) {
					fwprintf(stderr, L"unknown date format '%s'\n", argv[i]);
					return EXIT_FAILURE;
				} else {
					date_format = gsiv->value;
				}
			} else if (strcmp(argv[i], "divisions") == 0) {
				ASSERT_OPTION_HAS_ARGUMENT(argc, argv, i);
				i++;
				gsiv = (gsiv_t*)division_type_lookup(argv[i], strlen(argv[i]));
				if (gsiv == NULL) {
					fwprintf(stderr, L"unknown division type '%s'\n", argv[i]);
					return EXIT_FAILURE;
				} else {
					division_type = gsiv->value;
				}
			} else if (strcmp(argv[i], "no-colour") == 0) {
				flags |= FLAG_COLOUR;
				flags -= FLAG_COLOUR;
			} else if (strcmp(argv[i], "output") == 0) {
				ASSERT_OPTION_HAS_ARGUMENT(argc, argv, i);
				i++;
				if (strcmp(argv[i], "table") == 0) output_type = OUTPUT_TABLE;
				else if (strcmp(argv[i], "uncategorised") == 0) output_type = OUTPUT_UNCATEGORISED;
				else {
					fwprintf(stderr, L"unknown output type '%s'\n", argv[i]);
					return EXIT_FAILURE;
				}
			} else {
				// banks
				
				int (*parse)(char *, transaction_list_t *);
				if (strcmp(argv[i], "cash") == 0)				parse = parse_cash;
				else if (strcmp(argv[i], "nationwide") == 0)	parse = parse_nationwide;
				else if (strcmp(argv[i], "natwest") == 0)		parse = parse_natwest;
				else {
					fwprintf(stderr, L"unknown option '%s'", argv[i]);
					return EXIT_FAILURE;
				}

				ASSERT_OPTION_HAS_ARGUMENT(argc, argv, i);

				int status = EXIT_SUCCESS;
				int j;
				for (j = i+1; j < argc; j++) {
					if (argv[j][0] == '-' && argv[j][1] == '-') {
						i = j;
						break;
					} else if (parse(argv[j], transactions+collection.num_accounts) == EXIT_SUCCESS) {
						collection.num_accounts++;
					} else {
						fwprintf(stderr, L"error loading transaction file '%s'\n", argv[j]);
					}
				}
				if (j == argc) i = j;
				continue;
			}
			i++;
		} else {
			fwprintf(stderr, L"unexpected non-option argument '%s'\n", argv[i]);
			return EXIT_FAILURE;
		}
	}

	collection.daterange[0] = transactions[0].daterange[0];
	collection.daterange[1] = transactions[0].daterange[1];
	for (int i = 1; i < collection.num_accounts; i++) {
		if (earlier_date(collection.daterange[0], transactions[i].daterange[0])) collection.daterange[0] = transactions[i].daterange[0];
		if (earlier_date(transactions[i].daterange[1], collection.daterange[1])) collection.daterange[1] = transactions[i].daterange[1];
	}

	for (int i = 0; i < collection.num_accounts; i++) {
		transaction_list_t *tlist = transactions+i;
		for (int j = 0; j < tlist->size; j++) {
			categorise_transaction(&rules, &(tlist->transactions[j]));
		}
	}

	switch (output_type) {
		case OUTPUT_TABLE:
			print_table(collection, division_type, date_format, flags);
			break;
		case OUTPUT_UNCATEGORISED:
			for (int i = 0; i < collection.num_accounts; i++) {
				int header_printed = 0;
				transaction_list_t *tlist = transactions+i;
				for (int j = 0; j < tlist->size; j++) {
					if (tlist->transactions[j].category == TCAT_NONE) {
						if (!header_printed) {
							if (flags & FLAG_COLOUR) wprintf(L"%lc%lc \e[1m%s: %s\e[0m (%s)\n", TABLE_TOPLEFT_BOLD, TABLE_HLINE_BOLD, tlist->bank, tlist->name, tlist->file);
							else wprintf(L"%lc%lc %s: %s (%s)\n", TABLE_TOPLEFT, TABLE_HLINE, tlist->bank, tlist->name, tlist->file);
							header_printed = 1;
						}

						wprintf(L"%lc %ls %s ", TABLE_VLINE_BOLD, datestr(tlist->transactions[j].date, date_format), get_ttype_pretty_string(tlist->transactions[j].type));
						if (tlist->transactions[j].amount < 0) {
							wprintf(L"-£%.02f\t", (-1*(tlist->transactions[j].amount))/100.0f);
						} else {
							wprintf(L"£%.02f\t", (tlist->transactions[j].amount)/100.0f);
						}
						wprintf(L" %s\n", tlist->transactions[j].description);
					}
				}
				if (header_printed) {
					wprintf(L"%lc%lc\n", TABLE_BOTTOMLEFT_BOLD, TABLE_HLINE_BOLD);
				}
			}
			break;
		default:
			break;
	}

	for (int i = 0; i < collection.num_accounts; i++) transaction_list_destroy(transactions+i);

	return EXIT_SUCCESS;
}
