#include "table.h"

int print_table(transaction_list_collection_t collection, int division_type, int date_format, table_flags_t flags) {
	int num_date_divisions;
	date_t *division_dates;
	int (*get_date_division)(date_t, date_t);

	switch (division_type) {
		case DIV_YEARS:
			get_date_division = &date_division_index_year;
			num_date_divisions = collection.daterange[1].year - collection.daterange[0].year + 1;
			division_dates = malloc(sizeof(date_t)*num_date_divisions);
			for (int i = 0; i < num_date_divisions; i++) {
				division_dates[i].day = 1;
				division_dates[i].month = 1;
				division_dates[i].year = collection.daterange[0].year + i;
			}
			date_format = date_format == DATEFMT_NONE ? DATEFMT_YYYY : date_format;
			break;
		case DIV_MONTHS:
			get_date_division = &date_division_index_month;
			num_date_divisions = (collection.daterange[1].year-collection.daterange[0].year)*12 + (collection.daterange[1].month-collection.daterange[0].month+1);
			division_dates = malloc(sizeof(date_t)*num_date_divisions);
			for (int i = 0; i < num_date_divisions; i++) {
				division_dates[i].day = 1;
				division_dates[i].month = ((collection.daterange[0].month + i - 1) % 12) + 1;
				division_dates[i].year = collection.daterange[0].year + (collection.daterange[0].month + i) / 12;
			}
			date_format = date_format == DATEFMT_NONE ? DATEFMT_MMMYYYY : date_format;
			break;
		case DIV_WEEKS_SUN:
			get_date_division = &date_division_index_week_sun;
			int first_sun = collection.daterange[0].days_since_epoch - weekday(collection.daterange[0].day, collection.daterange[0].month, collection.daterange[0].year);
			num_date_divisions = ((collection.daterange[1].days_since_epoch - first_sun) / 7) + 1;
			division_dates = malloc(sizeof(date_t)*num_date_divisions);
			division_dates[0] = prev_sun(collection.daterange[0]);
			division_dates[0].days_since_epoch = days_since_epoch(division_dates[0].day, division_dates[0].month, division_dates[0].year);
			for (int i = 1; i < num_date_divisions; i++) {
				division_dates[i] = next_week(division_dates[i-1]);
			}
			collection.daterange[0] = division_dates[0];
			date_format = date_format == DATEFMT_NONE ? DATEFMT_DDMMYYYY : date_format;
			break;
		case DIV_WEEKS_MON:
			get_date_division = &date_division_index_week_mon;
			int start_weekday = weekday(collection.daterange[0].day, collection.daterange[0].month, collection.daterange[0].year);
			int first_mon = collection.daterange[0].days_since_epoch + (start_weekday == 0 ? -6 : 1 - start_weekday);
			num_date_divisions = ((collection.daterange[1].days_since_epoch - first_mon) / 7) + 1;
			division_dates = malloc(sizeof(date_t)*num_date_divisions);
			division_dates[0] = prev_mon(collection.daterange[0]);
			division_dates[0].days_since_epoch = days_since_epoch(division_dates[0].day, division_dates[0].month, division_dates[0].year);
			for (int i = 1; i < num_date_divisions; i++) {
				division_dates[i] = next_week(division_dates[i-1]);
			}
			collection.daterange[0] = division_dates[0];
			date_format = date_format == DATEFMT_NONE ? DATEFMT_DDMMYYYY : date_format;
			break;
		default:
			break;
	}

	int category_totals_size = sizeof(int)*(NUM_CATEGORIES*num_date_divisions*collection.num_accounts);
	int *category_totals = malloc(category_totals_size);
	memset(category_totals, 0, category_totals_size);
	for (int i = 0; i < collection.num_accounts; i++) {
		transaction_list_t *tlist = collection.lists+i;
		for (int j = 0; j < tlist->size; j++) {
			int date_division = (*get_date_division)(tlist->transactions[j].date, collection.daterange[0]);
			category_totals[(NUM_CATEGORIES*num_date_divisions*i)+(NUM_CATEGORIES*date_division)+(tlist->transactions[j].category)] += tlist->transactions[j].amount;
		}
	}

	const wchar_t *const date_division_headings[] = {
		L"year",
		L"month",
		L"wk comm",
		L"wk comm"
	};

	const int MIN_COL_WIDTH = 9; // probably not a great way of doing this but anyway
	int colwidths[NUM_CATEGORIES];
	int dateheadinglen = MAX(date_format_lengths[date_format], wcslen(date_division_headings[division_type]));
	wchar_t dateheading[dateheadinglen+1];
	wcscpy(dateheading, date_division_headings[division_type]);
	for (int i = wcslen(date_division_headings[division_type]); i < dateheadinglen; i++) {
		dateheading[i] = ' ';
	}
	dateheading[dateheadinglen] = '\0';
	const wchar_t *totalheading = L"total    ";
	wchar_t *spaces = malloc(sizeof(wchar_t)*MIN_COL_WIDTH);
	for (int i = 0; i < MIN_COL_WIDTH; i++) {
		spaces[i] = ' ';
	}
	int linewidth = wcslen(dateheading) + wcslen(totalheading) + 3;
	if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[1m");
	wprintf(dateheading);
	if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[0m");
	for (int i = 0; i < NUM_CATEGORIES; i++) {
		wprintf(L" %lc ", TABLE_VLINE);
		colwidths[i] = wcslen(tcat_pretty_strings[i]);
		if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[1m");
		if (colwidths[i] < MIN_COL_WIDTH) {
			wprintf(L"%ls%.*ls", tcat_pretty_strings[i], MIN_COL_WIDTH-colwidths[i], spaces);
			colwidths[i] = MIN_COL_WIDTH;
		} else {
			wprintf(tcat_pretty_strings[i]);
		}
		if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[0m");
		colwidths[i] += 2;
		linewidth += colwidths[i] + 1;
	}
	spaces = realloc(spaces, sizeof(wchar_t)*linewidth);
	memset(spaces+MIN_COL_WIDTH, ' ', linewidth-MIN_COL_WIDTH);
	for (int i = MIN_COL_WIDTH; i < linewidth-MIN_COL_WIDTH; i++) {
		spaces[i] = ' ';
	}
	wprintf(L" %lc ", TABLE_VLINE);
	if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[1m");
	wprintf(L"%ls\n", totalheading);
	if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[0m");

	wchar_t hrule[linewidth+2];
	for (int i = 0; i < linewidth; i++) {
		hrule[i] = TABLE_HLINE;
	}
	hrule[linewidth] = '\n';
	hrule[linewidth+1] = '\0';
	int pos = dateheadinglen+1;
	for (int i = 0; i < NUM_CATEGORIES; i++) {
		hrule[pos] = TABLE_CROSS;
		pos += colwidths[i] + 1;
	}
	hrule[pos] = TABLE_CROSS;

	wprintf(hrule);

	int grandtotal = 0;
	for (int i = 0; i < num_date_divisions; i++) {
		wprintf(L"%ls%.*ls%lc", datestr(division_dates[i], date_format), dateheadinglen-date_format_lengths[date_format]+1, spaces, TABLE_VLINE);
		int rowtotal = 0;
		for (int j = 0; j < NUM_CATEGORIES; j++) {
			int celltotal = 0;
			for (int k = 0; k < collection.num_accounts; k++) {
				 celltotal += category_totals[(NUM_CATEGORIES*num_date_divisions*k)+(NUM_CATEGORIES*i)+j];
			}
			unsigned int len;
			char cellstr[colwidths[j]+1];
			memset(cellstr, ' ', colwidths[j]);
			if (celltotal < 0) {
				len = intstrlen((-1*celltotal)/100) + 5;
				if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[31m");
				wprintf(L"%.*ls-£%.02f", colwidths[j]-len-1, spaces, (-1*celltotal)/100.0f);
				if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[0m");
			} else {
				len = intstrlen(celltotal/100) + 4;
				if (celltotal == 0) {
					if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[30m");
					wprintf(L"%.*ls£0.00", colwidths[j]-6, spaces);
					if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[0m");
				} else {
					if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[32m");
					wprintf(L"%.*ls£%.02f", colwidths[j]-len-1, spaces, celltotal/100.0f);
					if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[0m");
				}
			}
			wprintf(L" %lc", TABLE_VLINE);
			rowtotal += j == TCAT_MOVEMONEY ? 0 : celltotal;
		}
		if (rowtotal < 0) {
			if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[31m");
			wprintf(L"%.*ls-£%.02f", (int)(wcslen(totalheading)-intstrlen((-1*rowtotal)/100)-4), spaces, (-1*rowtotal)/100.0f);
			if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[0m");
		} else {
			if (rowtotal == 0) {
				if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[30m");
				wprintf(L"%.*ls£0.00", (int)(wcslen(totalheading)-4), spaces);
				if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[0m");
			} else {
				if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[32m");
				wprintf(L"%.*ls£%.02f", (int)(wcslen(totalheading)-intstrlen(rowtotal/100)-3), spaces, rowtotal/100.0f);
				if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[0m");
			}
		}
		wprintf(L"\n");
		grandtotal += rowtotal;
	}

	if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[1m");
	wprintf(L"%.*lstotal", pos-6, spaces);
	if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[0m");
	wprintf(L" %lc", TABLE_VLINE);
	if (grandtotal < 0) {
		if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[31m");
		wprintf(L"%.*ls-£%.02f", (int)(wcslen(totalheading)-intstrlen((-1*grandtotal)/100)-4), spaces, (-1*grandtotal)/100.0f);
		if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[0m");
	} else {
		if (grandtotal == 0) {
			if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[30m");
			wprintf(L"%.*ls£0.00", (int)(wcslen(totalheading)-intstrlen(grandtotal/100)-3), spaces);
			if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[0m");
		} else {
			if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[32m");
			wprintf(L"%.*ls£%.02f", (int)(wcslen(totalheading)-intstrlen(grandtotal/100)-3), spaces, grandtotal/100.0f);
			if (flags & TABLEFLAG_COLOUR) wprintf(L"\e[0m");
		}
	}
	wprintf(L"\n");

	free(category_totals);
	free(division_dates);
	free(spaces);

	return EXIT_SUCCESS;
}
