#include "csv.h"

int csv_open(csv_t *csv, char *path) {
	csv->file = fopen(path, "r");
	return csv->file == NULL ? errno : EXIT_SUCCESS;
}

int csv_close(csv_t *csv) {
	return fclose(csv->file) == 0 ? EXIT_SUCCESS : errno;
}

int csv_next_line(csv_t *csv, csv_line_t *line) {
	char *rawline = NULL;
	size_t n;
	ssize_t bytes_read = getline(&rawline, &n, csv->file);
	if (
#ifdef CSV_COMMENTS
		rawline[0] == '#' ||
#endif
		rawline[0] == '\n'
	) {
		free(rawline);
		line->ptr = malloc(sizeof(char));
		line->ptr = '\0';
		line->fields = NULL;
		line->num_fields = 0;
		return 1;
	}
	if (rawline[bytes_read-1] == '\n') {
		rawline[bytes_read-1] = '\0';
		bytes_read--;
	}
	short field_indices[bytes_read];
	field_indices[0] = 0;
	size_t num_fields = 1;
	int escapes = 0;
	int in_quotes = 0;

	if (bytes_read == -1) return 0;
	else if (line == NULL) {
		free(rawline);
		return 1;
	}

	line->fields = NULL;
	line->num_fields = 0;

	for (int i = 0; i < bytes_read; i++) {
		if (in_quotes) {
			if (rawline[i] == '\\') {
				escapes++;
			} else if (rawline[i] == '"') {
				if (escapes % 2 == 0) {
					in_quotes = 0;
				}
			} else escapes = 0;
		} else if (rawline[i] == ',') {
			rawline[i] = '\0';
			field_indices[num_fields] = i+1;
			num_fields++;
		} else if (rawline[i] == '"') in_quotes = 1;
	}
	field_indices[num_fields] = bytes_read+1; // in case we need to remove a final '"'

	line->num_fields = num_fields;
	line->fields = malloc(sizeof(char*)*num_fields);

	for (int j = 0; j < num_fields; j++) {
		if (rawline[field_indices[j]] == '"') {
			field_indices[j]++;
			rawline[field_indices[j+1]-2] = '\0';
			if (remove_escapes(rawline+field_indices[j]) != EXIT_SUCCESS) {
				fprintf(stderr, "invalid escape sequence\n");
				line->fields[j] = '\0';
				continue;
			}
		}
		line->fields[j] = rawline+field_indices[j];
	}

	line->ptr = rawline;

	return 1;
}

int csv_free_line(csv_line_t *line) {
	free(line->ptr);
	return EXIT_SUCCESS;
}

int csv_skip_lines(csv_t *csv, unsigned int num_lines) {
	size_t n;
	char *line = NULL;
	ssize_t bytes_read;
	for (int i = 0; i < num_lines; i++) {
		bytes_read = getline(&line, &n, csv->file);
	}
	return bytes_read != -1;
}
