#include "table.h"

int print_table(transaction_list_collection_t collection, int division_type, int date_format, flags_t flags) {
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
	int simple_category_totals[NUM_CATEGORIES];
	memset(category_totals, 0, category_totals_size);
	memset(simple_category_totals, 0, sizeof(simple_category_totals));
	for (int i = 0; i < collection.num_accounts; i++) {
		transaction_list_t *tlist = collection.lists+i;
		for (int j = 0; j < tlist->size; j++) {
			int date_division = (*get_date_division)(tlist->transactions[j].date, collection.daterange[0]);
			category_totals[(NUM_CATEGORIES*num_date_divisions*i)+(NUM_CATEGORIES*date_division)+(tlist->transactions[j].category)] += tlist->transactions[j].amount;
		}
	}

	const wchar_t *const date_division_headings[] = {
		L"year   ", // space at end so we can have 'total' and 'average' underneath
		L"month  ",
		L"wk comm",
		L"wk comm"
	};

	const wchar_t *esc_bold		= (flags & FLAG_COLOUR) ? ESC_BOLD		: ESC_NONE;
	const wchar_t *esc_positive	= (flags & FLAG_COLOUR) ? ESC_POSITIVE	: ESC_NONE;
	const wchar_t *esc_negative	= (flags & FLAG_COLOUR) ? ESC_NEGATIVE	: ESC_NONE;
	const wchar_t *esc_zero		= (flags & FLAG_COLOUR) ? ESC_ZERO		: ESC_NONE;
	const wchar_t *esc_end		= (flags & FLAG_COLOUR) ? ESC_END		: ESC_NONE;

	const int MIN_COL_WIDTH = 9; // probably not a great way of doing this but anyway
	int colwidths[NUM_CATEGORIES];

	int dateheadinglen = MAX(date_format_lengths[date_format], wcslen(date_division_headings[division_type]));
	wchar_t dateheading[dateheadinglen+1];
	wcscpy(dateheading, date_division_headings[division_type]);
	for (int i = wcslen(date_division_headings[division_type]); i < dateheadinglen; i++) dateheading[i] = ' ';
	dateheading[dateheadinglen] = '\0';

	const wchar_t *totalheading = L"total    ";

	wchar_t *spaces = malloc(sizeof(wchar_t)*MIN_COL_WIDTH);
	for (int i = 0; i < MIN_COL_WIDTH; i++) spaces[i] = ' ';

	int linewidth = wcslen(dateheading) + wcslen(totalheading) + 3;

	wprintf(L"%ls%ls%ls", esc_bold, dateheading, esc_end);
	for (int i = 0; i < NUM_CATEGORIES; i++) {
		wprintf(L" %lc ", TABLE_VLINE);
		colwidths[i] = wcslen(tcat_pretty_strings[i]);
		wprintf(L"%ls%ls%.*ls%ls", esc_bold, tcat_pretty_strings[i], colwidths[i] < MIN_COL_WIDTH ? MIN_COL_WIDTH-colwidths[i] : 0, spaces, esc_end);
		colwidths[i] = MAX(colwidths[i], MIN_COL_WIDTH) + 2;
		linewidth += colwidths[i] + 1;
	}

	spaces = realloc(spaces, sizeof(wchar_t)*linewidth);
	memset(spaces+MIN_COL_WIDTH, ' ', linewidth-MIN_COL_WIDTH);
	for (int i = MIN_COL_WIDTH; i < linewidth-MIN_COL_WIDTH; i++) spaces[i] = ' ';

	wprintf(L" %lc %ls%ls%ls\n", TABLE_VLINE, esc_bold, totalheading, esc_end);

	wchar_t hrule[linewidth+2];
	for (int i = 0; i < linewidth; i++) hrule[i] = TABLE_HLINE_BOLD;
	hrule[linewidth] = '\n';
	hrule[linewidth+1] = '\0';
	int pos = dateheadinglen+1;
	for (int i = 0; i < NUM_CATEGORIES; i++) {
		hrule[pos] = TABLE_CROSS_BOLDHORI;
		pos += colwidths[i] + 1;
	}
	hrule[pos] = TABLE_CROSS_BOLDHORI;
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
			simple_category_totals[j] += celltotal;

			unsigned int len;
			char cellstr[colwidths[j]+1];
			memset(cellstr, ' ', colwidths[j]);
			if (celltotal < 0) {
				wprintf(L"%.*ls%ls-£%.02f%ls %lc", colwidths[j]-intstrlen((-1*celltotal)/100)-6, spaces, esc_negative, (-1*celltotal)/100.0f, esc_end, TABLE_VLINE);
			} else {
				wprintf(L"%.*ls%ls£%.02f%ls %lc", colwidths[j]-intstrlen(celltotal/100)-5, spaces, celltotal == 0 ? esc_zero : esc_positive, celltotal/100.0f, esc_end, TABLE_VLINE);
			}
			rowtotal += j == TCAT_MOVEMONEY ? 0 : celltotal;
		}
		if (rowtotal < 0) {
			wprintf(L"%.*ls%ls-£%.02f%ls\n", (int)(wcslen(totalheading)-intstrlen((-1*rowtotal)/100)-4), spaces, esc_negative, (-1*rowtotal)/100.0f, esc_end);
		} else {
			wprintf(L"%.*ls%ls£%.02f%ls\n", (int)(wcslen(totalheading)-intstrlen(rowtotal/100)-3), spaces, rowtotal == 0 ? esc_zero : esc_positive, rowtotal/100.0f, esc_end);
		}
		grandtotal += rowtotal;
	}

	wprintf(hrule);

	wprintf(L"%lstotal%ls%.*ls%lc", esc_bold, esc_end, dateheadinglen-4, spaces, TABLE_VLINE);
	for (int i = 0; i < NUM_CATEGORIES; i++) {
		if (simple_category_totals[i] < 0) {
			wprintf(L"%.*ls%ls-£%.02f%ls %lc", colwidths[i]-intstrlen((-1*simple_category_totals[i])/100)-6, spaces, esc_negative, (-1*simple_category_totals[i])/100.0f, esc_end, TABLE_VLINE);
		} else {
			wprintf(L"%.*ls%ls£%.02f%ls %lc", colwidths[i]-intstrlen(simple_category_totals[i]/100)-5, spaces, simple_category_totals[i] == 0 ? esc_zero : esc_positive, simple_category_totals[i]/100.0f, esc_end, TABLE_VLINE);
		}
	}

	if (grandtotal < 0) {
		wprintf(L"%.*ls%ls-£%.02f%ls\n", (int)(wcslen(totalheading)-intstrlen((-1*grandtotal)/100)-4), spaces, esc_negative, (-1*grandtotal)/100.0f, esc_end);
	} else {
		wprintf(L"%.*ls%ls£%.02f%ls\n", (int)(wcslen(totalheading)-intstrlen(grandtotal/100)-3), spaces, grandtotal == 0 ? esc_zero : esc_positive, grandtotal/100.0f, esc_end);
	}

	wprintf(hrule);

	wprintf(L"%lsaverage%ls%.*ls%lc", esc_bold, esc_end, dateheadinglen-6, spaces, TABLE_VLINE);
	for (int i = 0; i < NUM_CATEGORIES; i++) {
		int average = (simple_category_totals[i]/num_date_divisions);
		if (simple_category_totals[i] < 0) {
			wprintf(L"%.*ls%ls-£%.02f%ls %lc", colwidths[i]-intstrlen((-1*average)/100)-6, spaces, esc_negative, (-1*average)/100.0f, esc_end, TABLE_VLINE);
		} else {
			wprintf(L"%.*ls%ls£%.02f%ls %lc", colwidths[i]-intstrlen(average/100)-5, spaces, simple_category_totals[i] == 0 ? esc_zero : esc_positive, average/100.0f, esc_end, TABLE_VLINE);
		}
	}

	int total_average = grandtotal/num_date_divisions;
	if (total_average < 0) {
		wprintf(L"%.*ls%ls-£%.02f%ls\n", (int)(wcslen(totalheading)-intstrlen((-1*total_average)/100)-4), spaces, esc_negative, (-1*total_average)/100.0f, esc_end);
	} else {
		wprintf(L"%.*ls%ls£%.02f%ls\n", (int)(wcslen(totalheading)-intstrlen(total_average/100)-3), spaces, total_average == 0 ? esc_zero : esc_positive, total_average/100.0f, esc_end);
	}

	free(category_totals);
	free(division_dates);
	free(spaces);

	return EXIT_SUCCESS;
}
