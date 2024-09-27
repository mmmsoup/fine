#include "balance.h"

int print_balance(transaction_list_collection_t collection, flags_t flags) {
	char *cash = "Cash";

	const wchar_t *esc_bold		= (flags & FLAG_COLOUR) ? ESC_BOLD		: ESC_NONE;
	const wchar_t *esc_positive	= (flags & FLAG_COLOUR) ? ESC_POSITIVE	: ESC_NONE;
	const wchar_t *esc_negative	= (flags & FLAG_COLOUR) ? ESC_NEGATIVE	: ESC_NONE;
	const wchar_t *esc_zero		= (flags & FLAG_COLOUR) ? ESC_ZERO		: ESC_NONE;
	const wchar_t *esc_end		= (flags & FLAG_COLOUR) ? ESC_END		: ESC_NONE;

	char *names[collection.num_accounts];
	char *banks[collection.num_accounts];
	date_t lastdates[collection.num_accounts];
	int balances[collection.num_accounts];
	int num_distinct_accounts = 0;
	int new;

	for (int i = 0; i < collection.num_accounts; i++) {
		new = 1;

		for (int j = 0; j < num_distinct_accounts; j++) {
			if (strcmp(collection.lists[i].name, names[j]) == 0 && strcmp(collection.lists[i].bank, banks[j]) == 0) {
				if (earlier_date(collection.lists[i].transactions[collection.lists[i].size-1].date, lastdates[j])) {
					balances[j] = collection.lists[i].balance;
					lastdates[j] = collection.lists[i].transactions[collection.lists[i].size-1].date;
				}
				new = 0;
				break;
			}
		}

		if (new) {
			names[num_distinct_accounts] = collection.lists[i].name;
			banks[num_distinct_accounts] = collection.lists[i].bank;
			lastdates[num_distinct_accounts] = collection.lists[i].transactions[collection.lists[i].size-1].date;
			balances[num_distinct_accounts] = collection.lists[i].balance;
			num_distinct_accounts++;
		}
	}

	int name_column_width = 5;	// "Total"
	int balance_column_width = 0; 
	int total = 0;
	for (int i = 0; i < num_distinct_accounts; i++) {
		if (strcmp(names[i], "Cash Transactions") == 0) names[i] = cash;

		int namelen = strlen(names[i]);
		name_column_width = MAX(name_column_width, namelen);

		int balancelen = balances[i] < 0 ? intstrlen(-1*balances[i]) + 3 : intstrlen(balances[i]) + 2;
		balance_column_width = MAX(balance_column_width, balancelen);

		total += balances[i];
	}
	int totallen = total < 0 ? intstrlen(-1*total) + 3 : intstrlen(total) + 2;
	balance_column_width = MAX(balance_column_width, totallen);

	char spaces[MAX(name_column_width, balance_column_width)];
	memset(spaces, ' ', sizeof(spaces)/sizeof(char));

	for (int i = 0; i < num_distinct_accounts; i++) {
		wprintf(L"%s%.*s ", names[i], name_column_width - strlen(names[i]), spaces);

		unsigned int n_balance_digits = intstrlen(ABS(balances[i])) + 2*(balances[i] == 0);

		if (balances[i] < 0) {
			wprintf(L"%.*s%ls-£%.02f%ls\n", balance_column_width - (n_balance_digits + 2), spaces, esc_negative, (-1*balances[i])/100.0f, esc_end);
		} else {
			wprintf(L"%.*s%ls£%.02f%ls\n", balance_column_width - (n_balance_digits + 1), spaces, esc_positive, (balances[i])/100.0f, esc_end);
		}
	}

	for (int i = 0; i < name_column_width+balance_column_width+2; i++) {
		wprintf(L"%lc", TABLE_HLINE_BOLD);
	}
	wprintf(L"\nTotal%.*s  ", name_column_width - 5, spaces);
	if (total < 0) {
		wprintf(L"%.*s%ls-£%.02f%ls\n", balance_column_width - totallen, spaces, esc_negative, (-1*total)/100.0f, esc_end);
	} else {
		wprintf(L"%.*s%ls£%.02f%ls\n", balance_column_width - totallen, spaces, esc_positive, total/100.0f, esc_end);
	}

	return EXIT_SUCCESS;
}
