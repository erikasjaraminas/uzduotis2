#include <stdio.h>
#include <string.h>
#include <stdarg.h>

int checkFile(char *filename, char *type) {

	char str[6];
	char *ret;
	ret = strrchr(filename, '.');
	sprintf(str, ".%s", type);

	if (ret == NULL) {
		return 0;
	}

	if (!strcmp(str, ret)) {
		return 1;
	}

	return 0;
}

int move(char *oldpath, char *newpath, char *filename) {

	char old[256];
	char new[256];

	sprintf(old, "%s/%s", oldpath, filename);
	sprintf(new, "%s/%s", newpath, filename);

	return rename(old, new);
}

int file_printnflush(FILE *stream, const char *format, ...) {

	int ret;
	va_list args;

	va_start(args, format);
	ret = vfprintf(stream, format, args);
	va_end(args);

	if (ret < 0) {
		return ret;
	}

	return fflush(stream);
}