#include "banks.h"
#include "categorisation.h"
#include "common.h"
#include "date.h"

#define TABLE_HLINE '-'
#define TABLE_VLINE '|'
#define TABLE_CROSS '+'

/*
#define TABLE_HLINE '━'
#define TABLE_VLINE '┃'
#define TABLE_CROSS '╋'
*/

int main(int argc, char **argv) {
	catrule_list_t rules;
	load_categorisation_rules(&rules, "category_config.csv");

	size_t num_accounts = 2;

	transaction_list_t transactions[num_accounts];
	parse_nationwide("nationwide6.csv", transactions);
	parse_natwest("natwest0707-current.csv", transactions+1);

	date_t daterange[2] = { transactions[0].daterange[0], transactions[0].daterange[1] };
	for (int i = 1; i < num_accounts; i++) {
		if (earlier_date(daterange[0], transactions[i].daterange[0])) daterange[0] = transactions[i].daterange[0];
		if (earlier_date(transactions[i].daterange[1], daterange[1])) daterange[1] = transactions[i].daterange[1];
	}

	const enum { DIV_YEARS, DIV_MONTHS, DIV_WEEKS_SUN, DIV_WEEKS_MON } division_types;
	const char *const date_division_headings[] = {
		"year",
		"month",
		"wk comm",
		"wk comm"
	};
	int date_format = DATEFMT_DDMMYYYY;
	int division_type;
	int num_date_divisions;
	date_t *division_dates;
	int (*get_date_division)(date_t, date_t);

	//division_type = DIV_YEARS;
	//division_type = DIV_MONTHS;
	//division_type = DIV_WEEKS_SUN;
	division_type = DIV_WEEKS_MON;

	switch (division_type) {
		case DIV_YEARS:
			get_date_division = &date_division_index_year;
			num_date_divisions = daterange[1].year - daterange[0].year + 1;
			division_dates = malloc(sizeof(date_t)*num_date_divisions);
			for (int i = 0; i < num_date_divisions; i++) {
				division_dates[i].day = 1;
				division_dates[i].month = 1;
				division_dates[i].year = daterange[0].year + i;
			}
			date_format = DATEFMT_YYYY;
			break;
		case DIV_MONTHS:
			get_date_division = &date_division_index_month;
			num_date_divisions = (daterange[1].year-daterange[0].year)*12 + (daterange[1].month-daterange[0].month+1);
			division_dates = malloc(sizeof(date_t)*num_date_divisions);
			for (int i = 0; i < num_date_divisions; i++) {
				division_dates[i].day = 1;
				division_dates[i].month = ((daterange[0].month + i - 1) % 12) + 1;
				division_dates[i].year = daterange[0].year + (daterange[0].month + i) / 12;
			}
			date_format = DATEFMT_MMMYYYY;
			break;
		case DIV_WEEKS_SUN:
			get_date_division = &date_division_index_week_sun;
			int first_sun = daterange[0].days_since_epoch - weekday(daterange[0].day, daterange[0].month, daterange[0].year);
			num_date_divisions = (daterange[1].days_since_epoch - first_sun) / 7;
			division_dates = malloc(sizeof(date_t)*num_date_divisions);
			division_dates[0] = prev_sun(daterange[0]);
			division_dates[0].days_since_epoch = days_since_epoch(division_dates[0].day, division_dates[0].month, division_dates[0].year);
			for (int i = 1; i < num_date_divisions; i++) {
				division_dates[i] = next_week(division_dates[i-1]);
			}
			daterange[0] = division_dates[0];
			date_format = DATEFMT_DDMMYYYY;
			break;
		case DIV_WEEKS_MON:
			get_date_division = &date_division_index_week_mon;
			int start_weekday = weekday(daterange[0].day, daterange[0].month, daterange[0].year);
			int first_mon = daterange[0].days_since_epoch + (start_weekday == 0 ? -6 : 1 - start_weekday);
			num_date_divisions = (daterange[1].days_since_epoch - first_mon) / 7;
			division_dates = malloc(sizeof(date_t)*num_date_divisions);
			division_dates[0] = prev_mon(daterange[0]);
			division_dates[0].days_since_epoch = days_since_epoch(division_dates[0].day, division_dates[0].month, division_dates[0].year);
			for (int i = 1; i < num_date_divisions; i++) {
				division_dates[i] = next_week(division_dates[i-1]);
			}
			daterange[0] = division_dates[0];
			date_format = DATEFMT_DDMMYYYY;
			break;
		default:
			break;
	}

	int category_totals[NUM_CATEGORIES*num_date_divisions*num_accounts];
	memset(category_totals, 0, sizeof(category_totals));
	
	for (int i = 0; i < num_accounts; i++) {
		transaction_list_t *tlist = transactions+i;
		for (int j = 0; j < tlist->size; j++) {
			categorise_transaction(&rules, &(tlist->transactions[j]));
			int date_division = (*get_date_division)(tlist->transactions[j].date, daterange[0]);
			category_totals[(NUM_CATEGORIES*num_date_divisions*i)+(NUM_CATEGORIES*date_division)+(tlist->transactions[j].category)] += tlist->transactions[j].amount;
		}
	}

	const int MIN_COL_WIDTH = 9; // probably not a great way of doing this but anyway
	int colwidths[NUM_CATEGORIES];
	int dateheadinglen = MAX(date_format_lengths[date_format], strlen(date_division_headings[division_type]));
	char dateheading[dateheadinglen+1];
	strcpy(dateheading, date_division_headings[division_type]);
	memset(dateheading+strlen(date_division_headings[division_type]), ' ', dateheadinglen-strlen(date_division_headings[division_type]));
	dateheading[dateheadinglen] = '\0';
	const char totalheading[] = { ' ', TABLE_VLINE, ' ', 't', 'o', 't', 'a', 'l', ' ', ' ', ' ', ' ', '\0' };
	char *spaces = malloc(sizeof(char)*MIN_COL_WIDTH);
	memset(spaces, ' ', MIN_COL_WIDTH);
	int linewidth = strlen(dateheading) + strlen(totalheading);
	printf(dateheading);
	for (int i = 0; i < NUM_CATEGORIES; i++) {
		printf(" %c ", TABLE_VLINE);
		colwidths[i] = strlen(tcat_pretty_strings[i]);
		if (colwidths[i] < MIN_COL_WIDTH) {
			printf("%s%.*s", tcat_pretty_strings[i], MIN_COL_WIDTH-colwidths[i], spaces);
			colwidths[i] = MIN_COL_WIDTH;
		} else {
			printf(tcat_pretty_strings[i]);
		}
		colwidths[i] += 2;
		linewidth += colwidths[i] + 1;
	}
	spaces = realloc(spaces, sizeof(char)*linewidth);
	memset(spaces+MIN_COL_WIDTH, ' ', linewidth-MIN_COL_WIDTH);
	printf("%s\n", totalheading);

	char hrule[linewidth+2];
	memset(hrule, TABLE_HLINE, linewidth);
	hrule[linewidth] = '\n';
	hrule[linewidth+1] = '\0';
	int pos = dateheadinglen+1;
	for (int i = 0; i < NUM_CATEGORIES; i++) {
		hrule[pos] = '+';
		pos += colwidths[i] + 1;
	}
	hrule[pos] = '+';

	printf(hrule);

	int grandtotal = 0;
	for (int i = 0; i < num_date_divisions; i++) {
		printf("%s%.*s%c", datestr(division_dates[i], date_format), dateheadinglen-date_format_lengths[date_format]+1, spaces, TABLE_VLINE);
		int rowtotal = 0;
		for (int j = 0; j < NUM_CATEGORIES; j++) {
			int celltotal = 0;
			for (int k = 0; k < num_accounts; k++) {
				 celltotal += category_totals[(NUM_CATEGORIES*num_date_divisions*k)+(NUM_CATEGORIES*i)+j];
			}
			unsigned int len;
			char cellstr[colwidths[j]+1];
			memset(cellstr, ' ', colwidths[j]);
			if (celltotal < 0) {
				len = intstrlen((-1*celltotal)/100) + 5;
				printf("%.*s-£%.02f %c", colwidths[j]-len-1, spaces, (-1*celltotal)/100.0f, TABLE_VLINE);
			} else {
				len = intstrlen(celltotal/100) + 4;
				printf("%.*s£%.02f %c", colwidths[j]-len-1, spaces, celltotal/100.0f, TABLE_VLINE);
			}
			rowtotal += TCAT_NETSAME & (1 << j) ? 0 : celltotal;
		}
		if (rowtotal < 0) {
			printf("%.*s-£%.02f\n", (int)(strlen(totalheading)-intstrlen((-1*rowtotal)/100)-7), spaces, (-1*rowtotal)/100.0f);
		} else {
			printf("%.*s£%.02f\n", (int)(strlen(totalheading)-intstrlen(rowtotal/100)-6), spaces, rowtotal/100.0f);
		}
		grandtotal += rowtotal;
	}

	if (grandtotal < 0) {
		printf("%.*stotal %c%.*s-£%.02f\n", pos-6, spaces, TABLE_VLINE, (int)(strlen(totalheading)-intstrlen((-1*grandtotal)/100)-7), spaces, (-1*grandtotal)/100.0f);
	} else {
		printf("%.*stotal %c%.*s£%.02f\n", pos-6, spaces, TABLE_VLINE, (int)(strlen(totalheading)-intstrlen(grandtotal/100)-6), spaces, grandtotal/100.0f);
	}

	free(spaces);

	return EXIT_SUCCESS;
}
