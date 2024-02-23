#include "gperf.h"

const char *get_ttype_pretty_string(ttype_t ttype) {
	switch (ttype) {
		case TTYPE_ACCT_CREDIT:
			return "+acct";
			break;
		case TTYPE_ACCT_DEBIT:
			return "-acct";
			break;
		case TTYPE_CARD_CREDIT:
			return "+card";
			break;
		case TTYPE_CARD_DEBIT:
			return "-card";
			break;
		case TTYPE_CASH_CREDIT:
			return "+cash";
			break;
		case TTYPE_CASH_DEBIT:
			return "-cash";
			break;
		case TTYPE_CHEQUE_CREDIT:
			return "+cheq";
			break;
		case TTYPE_CHEQUE_DEBIT:
			return "-cheq";
			break;
		default:
			return "";
	}
}
