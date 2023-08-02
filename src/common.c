#include "common.h"

int transaction_list_create(transaction_list_t *list, char *name, char *bank, char *file, size_t size) {
	list->transactions = malloc(sizeof(transaction_t)*size);
	list->capacity = size;
	list->size = 0;
	list->name = malloc(sizeof(char)*(strlen(name)+1));
	strcpy(list->name, name);
	list->bank = malloc(sizeof(char)*(strlen(bank)+1));
	strcpy(list->bank, bank);
	list->file = malloc(sizeof(char)*(strlen(file)+1));
	strcpy(list->file, file);
	return EXIT_SUCCESS;
}

int transaction_list_resize(transaction_list_t *list, size_t size) {
	list->transactions = realloc(list->transactions, sizeof(transaction_t)*size);
	list->capacity = size;
	return EXIT_SUCCESS;
}

int transaction_list_destroy(transaction_list_t *list) {
	for (int i = 0; i < list->size; i++) free(list->transactions[i].description);
	free(list->transactions);
	free(list->name);
	free(list->bank);
	free(list->file);
	return EXIT_SUCCESS;
}

int exponent(int radix, unsigned int exp) {
	return exp == 0 ? 1 : radix * exponent(radix, exp - 1);
}

unsigned int intstrlen(unsigned int num) {
	return 1 + (num < 10 ? 0 : intstrlen(num/10));
}

int parse_string_price(char *str) {
	short sign = str[0] == '-' ? -1 : 1;
	if (str[0] == '-' || str[0] == '+') str++;

	int length = 0, separator_index = -1;
	while (1) {
		if (str[length] == '.') separator_index = length;
		else if (str[length] == '\0') {
			if (separator_index == -1) separator_index = length;
			break;
		}
		length++;
	}

	int value = 0;

	// after decimal point
	for (int i = 1; i < length-separator_index; i++) {
		value += (str[length-i] - 0x30) * exponent(10, i-1);
	}

	// before decimal point
	for (int i = 1; i <= separator_index; i++) {
		value += (str[separator_index-i] - 0x30) * exponent(10, i+1);
	}

	return value * sign;
}

// not exposed in header file
int remove_escapes_recursive(char *readstr, int num_escapes) {
	int escaped = 0;
	char *writestr = readstr - num_escapes;
	while (*readstr != '\0') {
		if (escaped) return *readstr == '\\' ? remove_escapes_recursive(readstr+1, num_escapes+1) : EXIT_FAILURE;
		else {
			if (*readstr == '\\') escaped = 1;
			*writestr = *readstr;
		}
		readstr++;
		writestr++;
	}
	*writestr = *readstr;
	return EXIT_SUCCESS;
}

int remove_escapes(char *str) {
	return remove_escapes_recursive(str, 0);
}
