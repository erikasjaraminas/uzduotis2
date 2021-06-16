#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <file_operations.h>
#include <parser.h>

char dir_to_watch[256];
char **audio_types;
char **video_types;
char **photo_types;
char **document_types;
char **types_to_watch;

int audio_types_count = 0;
int video_types_count = 0;
int photo_types_count = 0;
int document_types_count = 0;
int types_to_watch_count = 0;

void free_up_memory() {

	// audio_types
	for (int i = 0; i < audio_types_count; i++) {
		free(audio_types[i]);
	}
	free(audio_types);

	// video_types
	for (int i = 0; i < video_types_count; i++) {
		free(video_types[i]);
	}
	free(video_types);

	// photo_types
	for (int i = 0; i < photo_types_count; i++) {
		free(photo_types[i]);
	}
	free(photo_types);

	// document_types
	for (int i = 0; i < document_types_count; i++) {
		free(document_types[i]);
	}
	free(document_types);

	// types_to_watch
	for (int i = 0; i < types_to_watch_count; i++) {
		free(types_to_watch[i]);
	}
	free(types_to_watch);
}

void exit_program(int status) {

	free_up_memory();

	exit(status);
}

void read_values() {

	config_option_t co;
	if ((co = read_config_file("/etc/watchlist.conf")) == NULL) {
		perror("read_config_file()");

		exit_program(1);
	}
	
	int c = 0;
	char *token;
	while(1) {
		if (!strcmp(co->key, "dir_to_watch")) {
			strcpy(dir_to_watch, co->value);
		}
		if (!strcmp(co->key, "audio_types")) {
			audio_types[0] = (char *) malloc(5 * sizeof(char));
			token = strtok(co->value, ",");
			strcpy(audio_types[0], token);
			token = strtok(NULL, ",");
			c = 1;
			while (token != NULL) {
				audio_types = realloc(audio_types, (c + 1) * sizeof(char *));
				audio_types[c] = strdup(token);
				token = strtok(NULL, ",");
				c++;
			}
			audio_types_count = c;
		}
		if (!strcmp(co->key, "video_types")) {
			video_types[0] = (char *) malloc(5 * sizeof(char));
			token = strtok(co->value, ",");
			strcpy(video_types[0], token);
			token = strtok(NULL, ",");
			c = 1;
			while (token != NULL) {
				video_types = realloc(video_types, (c + 1) * sizeof(char *));
				video_types[c] = strdup(token);
				token = strtok(NULL, ",");
				c++;
			}
			video_types_count = c;
		}
		if (!strcmp(co->key, "photo_types")) {
			photo_types[0] = (char *) malloc(5 * sizeof(char));
			token = strtok(co->value, ",");
			strcpy(photo_types[0], token);
			token = strtok(NULL, ",");
			c = 1;
			while (token != NULL) {
				photo_types = realloc(photo_types, (c + 1) * sizeof(char *));
				photo_types[c] = strdup(token);
				token = strtok(NULL, ",");
				c++;
			}
			photo_types_count = c;
		}
		if (!strcmp(co->key, "document_types")) {
			document_types[0] = (char *) malloc(5 * sizeof(char));
			token = strtok(co->value, ",");
			strcpy(document_types[0], token);
			token = strtok(NULL, ",");
			c = 1;
			while (token != NULL) {
				document_types = realloc(document_types, (c + 1) * sizeof(char *));
				document_types[c] = strdup(token);
				token = strtok(NULL, ",");
				c++;
			}
			document_types_count = c;
		}
		if (!strcmp(co->key, "types_to_watch")) {
			types_to_watch[0] = (char *) malloc(5 * sizeof(char));
			token = strtok(co->value, ",");
			strcpy(types_to_watch[0], token);
			token = strtok(NULL, ",");
			c = 1;
			while (token != NULL) {
				types_to_watch = realloc(types_to_watch, (c + 1) * sizeof(char *));
				types_to_watch[c] = strdup(token);
				token = strtok(NULL, ",");
				c++;
			}
			types_to_watch_count = c;
		}
		if (co->prev != NULL) {
			co = co->prev;
		} else {
			break;
		}
	}
}

int main() {

	audio_types = (char **) malloc(1 * sizeof(char *));
	video_types = (char **) malloc(1 * sizeof(char *));
	photo_types = (char **) malloc(1 * sizeof(char *));
	document_types = (char **) malloc(1 * sizeof(char *));
	types_to_watch = (char **) malloc(1 * sizeof(char *));

	struct dirent *pDirent;
	DIR *pDir = NULL;

	FILE *conf = NULL;
	FILE *log = NULL;
	pid_t process_id = 0;
	pid_t sid = 0;
	// Create child process
	process_id = fork();
	// Indication of fork() failure
	if (process_id < 0) {
		printf("fork failed!\n");
		exit_program(1);
	}
	// PARENT PROCESS. Need to kill it.
	if (process_id > 0) {
		printf("process_id of child process %d \n", process_id);
		exit_program(0);
	}
	//unmask the file mode
	umask(0);
	//set new session
	sid = setsid();
	if(sid < 0) {
		exit_program(1);
	}
	// Change the current working directory to root.
	chdir("/");
	// Open a log file in write mode.
	log = fopen("/var/log/daemon.log", "w");
	if (log == NULL) {
		perror("Error opening log file");

		// Program exits if the file pointer returns NULL.
		exit_program(1);
	}

	read_values();
	// Close stdin. stdout and stderr
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	file_printnflush(log, "Starting log file...\n\n");

	char *homedir = getenv("HOME");
	char newpath[256];

	while (1) {
		pDir = opendir(dir_to_watch);
		if (pDir == NULL) {
			file_printnflush(log, "Error opening '%s'\n", dir_to_watch);
			exit_program(1);
		}

		for (int i = 0; i < audio_types_count; i++) {
			printf("%s\n", audio_types[i]);
		}

		while ((pDirent = readdir(pDir)) != NULL) {
			if (!strcmp(pDirent->d_name, ".") || !strcmp(pDirent->d_name, "..")) {
				continue;
			}

			for (int a = 0; a < types_to_watch_count; a++) {
				if (!strcmp(types_to_watch[a], "audio")) {
					for (int b = 0; b < audio_types_count; b++) {
						if (checkFile(pDirent->d_name, audio_types[b])) {
							sprintf(newpath, "%s/Music", homedir);
							file_printnflush(log, "Moving file '%s' to '%s'. ", pDirent->d_name, newpath);
							int ret = move(dir_to_watch, newpath, pDirent->d_name);
							if (ret) {
								file_printnflush(log, "Failed.\n");
							} else {
								file_printnflush(log, "Success!\n");
							}
						}
					}
				}
				if (!strcmp(types_to_watch[a], "video")) {
					for (int b = 0; b < video_types_count; b++) {
						if (checkFile(pDirent->d_name, video_types[b])) {
							sprintf(newpath, "%s/Videos", homedir);
							file_printnflush(log, "Moving file '%s' to '%s'. ", pDirent->d_name, newpath);
							int ret = move(dir_to_watch, newpath, pDirent->d_name);
							if (ret) {
								file_printnflush(log, "Failed.\n");
							} else {
								file_printnflush(log, "Success!\n");
							}
						}
					}
				}
				if (!strcmp(types_to_watch[a], "photo")) {
					for (int b = 0; b < photo_types_count; b++) {
						if (checkFile(pDirent->d_name, photo_types[b])) {
							sprintf(newpath, "%s/Pictures", homedir);
							file_printnflush(log, "Moving file '%s' to '%s'. ", pDirent->d_name, newpath);
							int ret = move(dir_to_watch, newpath, pDirent->d_name);
							if (ret) {
								file_printnflush(log, "Failed.\n");
							} else {
								file_printnflush(log, "Success!\n");
							}
						}
					}
				}
				if (!strcmp(types_to_watch[a], "document")) {
					for (int b = 0; b < document_types_count; b++) {
						if (checkFile(pDirent->d_name, document_types[b])) {
							sprintf(newpath, "%s/Documents", homedir);
							file_printnflush(log, "Moving file '%s' to '%s'. ", pDirent->d_name, newpath);
							int ret = move(dir_to_watch, newpath, pDirent->d_name);
							if (ret) {
								file_printnflush(log, "Failed.\n");
							} else {
								file_printnflush(log, "Success!\n");
							}
						}
					}
				}
			}
		}

		closedir(pDir);

		sleep(5);
	}

	free_up_memory();

	fclose(log);

	return (0);
}