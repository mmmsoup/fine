#include "date.h"

const char *const weekday_names[7] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};

const char *const month_names[13] = {
	"the forbidden zeroth month", // i knew using 1-indexed months was a bad idea
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
};

const int month_lengths[] = {
	0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,	// regular year
	0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31	// leap year
};

const int date_format_lengths[] = {
	10,	// DDMMYYYY
	10,	// MMDDYYYY
	10,	// YYYYMMDD
	8,	// MMMYYYY
	4	// YYYY
};

char *datestr(date_t date, int format) {
	static char str[11];
	str[10] = '\0';
	switch (format == DATEFMT_NONE ? DEFAULT_DATEFMT : format) {
		case DATEFMT_DDMMYYYY:
			str[0] = (date.day / 10) + 0x30;
			str[1] = (date.day % 10) + 0x30;
			str[2] = '/';
			str[3] = (date.month / 10) + 0x30;
			str[4] = (date.month % 10) + 0x30;
			str[5] = '/';
			str[6] = (date.year / 1000) + 0x30;
			str[7] = ((date.year % 1000) / 100) + 0x30;
			str[8] = ((date.year % 100) / 10) + 0x30;
			str[9] = (date.year % 10) + 0x30;
		case DATEFMT_MMDDYYYY:
			// ew
			str[0] = (date.month / 10) + 0x30;
			str[1] = (date.month % 10) + 0x30;
			str[2] = '/';
			str[3] = (date.day / 10) + 0x30;
			str[4] = (date.day % 10) + 0x30;
			str[5] = '/';
			str[6] = (date.year / 1000) + 0x30;
			str[7] = ((date.year % 1000) / 100) + 0x30;
			str[8] = ((date.year % 100) / 10) + 0x30;
			str[9] = (date.year % 10) + 0x30;
		case DATEFMT_YYYYMMDD:
			// ISO-8601 my beloved <3
			str[0] = (date.year / 1000) + 0x30;
			str[1] = ((date.year % 1000) / 100) + 0x30;
			str[2] = ((date.year % 100) / 10) + 0x30;
			str[3] = (date.year % 10) + 0x30;
			str[4] = '-';
			str[5] = (date.month / 10) + 0x30;
			str[6] = (date.month % 10) + 0x30;
			str[7] = '-';
			str[8] = (date.day / 10) + 0x30;
			str[9] = (date.day % 10) + 0x30;
			break;
		case DATEFMT_MMMYYYY:
			str[0] = month_names[date.month][0];
			str[1] = month_names[date.month][1];
			str[2] = month_names[date.month][2];
			str[3] = ' ';
			str[4] = (date.year / 1000) + 0x30;
			str[5] = ((date.year % 1000) / 100) + 0x30;
			str[6] = ((date.year % 100) / 10) + 0x30;
			str[7] = (date.year % 10) + 0x30;
			str[8] = '\0';
			break;
		case DATEFMT_YYYY:
			str[0] = (date.year / 1000) + 0x30;
			str[1] = ((date.year % 1000) / 100) + 0x30;
			str[2] = ((date.year % 100) / 10) + 0x30;
			str[3] = (date.year % 10) + 0x30;
			str[4] = '\0';
			break;
		default:
			str[0] = '\0';
	}
	return str;
}

int days_since_epoch(int day, int month, int year) {
	const int days_after_full_months[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

	int num_days = ((year - 1970) * 365) + days_after_full_months[month-1] + day - 1;

	// leap days in previous years
	num_days += ((year - 1969) / 4) - ((year - 1901) / 100) + ((year - 1601) / 400);

	// take into account leap day in current year
	num_days += (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)) && (month > 2);

	return num_days;
}

extern inline int weekday(int day, int month, int year) {
	return (day += month < 3 ? year-- : year - 2, 23*month/9 + day + 4 + year/4- year/100 + year/400)%7;
}

extern inline const char *weekday_name(int weekday) {
	return weekday_names[weekday];
}

extern inline int earlier_date(date_t d1, date_t d2) {
	return (d1.year*10000 + d1.month*100 + d1.day) > (d2.year*10000 + d2.month*100 + d2.day);
}

date_t prev_sun(date_t d1) {
	date_t d2;
	d2.day = d1.day - weekday(d1.day, d1.month, d1.year);
	d2.day += d2.day <= 0 ? month_lengths[(((LEAP_YEAR(d1.year) * 13) + d1.month - 2) % 12) + 1] : 0;
	d2.month = ((d1.month - (d2.day > d1.day) - 1) % 12) + 1;
	d2.year = d1.year - (d2.month > d1.month);
	return d2;
}
date_t prev_mon(date_t d1) {
	date_t d2;
	int d1_weekday = weekday(d1.day, d1.month, d1.year);
	d2.day = d1.day + (d1_weekday == 0 ? -6 : 1 - d1_weekday);
	d2.day += d2.day <= 0 ? month_lengths[(((LEAP_YEAR(d1.year) * 13) + d1.month - 2) % 12) + 1] : 0;
	d2.month = ((d1.month - (d2.day > d1.day) - 1) % 12) + 1;
	d2.year = d1.year - (d2.month > d1.month);
	return d2;
}
date_t next_week(date_t d1) {
	date_t d2;
	d2.day = ((d1.day + 6) % month_lengths[(LEAP_YEAR(d1.year) * 13) + d1.month]) + 1;
	d2.month = ((d1.month + (d2.day < d1.day) - 1) % 12) + 1;
	d2.year = d1.year + (d2.month < d1.month);
	return d2;
}

int date_division_index_week_sun(date_t date, date_t startdate) {
	return (date.days_since_epoch-startdate.days_since_epoch)/7;
}
int date_division_index_week_mon(date_t date, date_t startdate) {
	return (date.days_since_epoch-startdate.days_since_epoch)/7;
}
int date_division_index_month(date_t date, date_t startdate) {
	return (date.year-startdate.year)*12 + (date.month-startdate.month);
}
int date_division_index_year(date_t date, date_t startdate) {
	return date.year - startdate.year;
}
