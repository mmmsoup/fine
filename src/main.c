#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "banks.h"
#include "categorisation.h"
#include "common.h"
#include "date.h"
#include "table.h"

int main(int argc, char **argv) {
	const enum { OUTPUT_TABLE, OUTPUT_UNCATEGORISED } output_types;

	int no_colour = 0;
	int config_loaded = 0;
	int output_type = OUTPUT_TABLE;
	int date_format = DATEFMT_NONE;
	int division_type = DIV_MONTHS;
	catrule_list_t rules;
	transaction_list_collection_t collection;
	collection.num_accounts = 0;
	transaction_list_t transactions[argc/2]; // this should always be plenty
	collection.lists = transactions;

	int c;
	int digit_optind;
	struct option long_options[] = {
		{ "config",			required_argument,	0,	'c' },
		{ "date-format",	required_argument,	0,	'd' },
		{ "divisions",		required_argument,	0,	'D' },
		{ "no-colour",		no_argument,		0,	'n' },
		{ "output",			required_argument,	0,	'o' },
		// load csv files from various banks
		{ "nationwide",		required_argument,	0,	0 },
		{ "natwest",		required_argument,	0,	0 },
		{ 0, 0, 0, 0 }
	};
	const char *short_options = "c:d:D:no:";
	while (1) {
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		int status = EXIT_SUCCESS;
		gsiv_t *gsiv;
		
		c = getopt_long(argc, argv, short_options, long_options, &option_index);
		if (c == -1) break;
		switch (c) {
			case 0:
				if (strcmp(long_options[option_index].name, "nationwide") == 0) {
					status = parse_nationwide(optarg, transactions+collection.num_accounts);
				} else if (strcmp(long_options[option_index].name, "natwest") == 0) {
					status = parse_natwest(optarg, transactions+collection.num_accounts);
				} else break;

				if (status == EXIT_SUCCESS) {
					collection.num_accounts++;
				} else {
					fprintf(stderr, "error loading transaction file '%s'", optarg);
				}
				break;
			case 'c':
				if (load_categorisation_rules(&rules, optarg) == EXIT_SUCCESS) config_loaded = 1;
				break;
			case 'd':
				gsiv = (gsiv_t*)date_format_lookup(optarg, strlen(optarg));
				if (gsiv == NULL) {
					fprintf(stderr, "unknown date format '%s'\n", optarg);
					exit(EXIT_FAILURE);
				} else {
					date_format = gsiv->value;
				}
				break;
			case 'D':
				gsiv = (gsiv_t*)division_type_lookup(optarg, strlen(optarg));
				if (gsiv == NULL) {
					fprintf(stderr, "unknown division type '%s'\n", optarg);
					exit(EXIT_FAILURE);
				} else {
					division_type = gsiv->value;
				}
				break;
			case 'o':
				if (strcmp(optarg, "table") == 0) output_type = OUTPUT_TABLE;
				else if (strcmp(optarg, "uncategorised") == 0) output_type = OUTPUT_UNCATEGORISED;
				else {
					fprintf(stderr, "unknown output type '%s'\n", optarg);
					exit(EXIT_FAILURE);
				}
				break;
			case 'n':
			case '?':
				break;
			default:
				fprintf(stderr, "hmmm this definitely shouldn't be happening whoops\n");
				exit(EXIT_FAILURE);
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
			print_table(collection, division_type, date_format);
			break;
		case OUTPUT_UNCATEGORISED:
			for (int i = 0; i < collection.num_accounts; i++) {
				transaction_list_t *tlist = transactions+i;
				for (int j = 0; j < tlist->size; j++) {
					if (tlist->transactions[j].category == TCAT_NONE) {
						printf("%s %s ", datestr(tlist->transactions[j].date, date_format), get_ttype_pretty_string(tlist->transactions[j].type));
						if (tlist->transactions[j].amount < 0) {
							printf("-£%.02f\t", (-1*(tlist->transactions[j].amount))/100.0f);
						} else {
							printf("£%.02f\t", (tlist->transactions[j].amount)/100.0f);
						}
						printf(" %s\n", tlist->transactions[j].description);
					}
				}
			}
			break;
		default:
			break;
	}

	return EXIT_SUCCESS;
}
