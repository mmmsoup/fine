#ifndef DATE_H
#define DATE_H

#include "gperf/month_abbr.h"

#define LEAP_YEAR(y) ((y % 4 == 0) - (y % 400 == 0))

typedef struct {
	short day;
	short month;
	short year;
	int days_since_epoch;
} date_t;

extern const char *const weekday_names[7];
extern const char *const month_names[13];

typedef enum {
	DATEFMT_DDMMYYYY,	// '01/01/1970'
	DATEFMT_MMDDYYYY,	// '01/01/1970' but silly way round
	DATEFMT_YYYYMMDD,	// '1970-01-01' ISO 8601 my beloved <3
	DATEFMT_MMMYYYY,	// 'Jan 1970'
	DATEFMT_YYYY,		// '1970'
} date_formats;
extern const int date_format_lengths[];
char *datestr(date_t, int);

// days since 1970-01-01
int days_since_epoch(int, int, int);

// returns 0-6 representing weekdays starting with Sunday
extern int weekday(int, int, int);

extern const char *weekday_name(int);

// returns index of earlier date, i.e 0 or 1
extern int earlier_date(date_t, date_t);

// used to get dates for weekly date divisions
date_t prev_sun(date_t);
date_t prev_mon(date_t);
date_t next_week(date_t);

int date_division_index_week_sun(date_t, date_t);
int date_division_index_week_mon(date_t, date_t);
int date_division_index_month(date_t, date_t);
int date_division_index_year(date_t, date_t);

#endif
