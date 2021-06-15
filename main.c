#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <file_operations.h>
#include <parser.h>

int main() {

	char dir_to_watch[256];
	char **audio_types = malloc(1 * sizeof(char *));
	char **video_types = malloc(1 * sizeof(char *));
	char **photo_types = malloc(1 * sizeof(char *));
	char **document_types = malloc(1 * sizeof(char *));
	char **types_to_watch = malloc(1 * sizeof(char *));

	int audio_types_count = 0;
	int video_types_count = 0;
	int photo_types_count = 0;
	int docoment_types_count = 0;
	int types_to_watch_count = 0;

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
		exit(1);
	}
	// PARENT PROCESS. Need to kill it.
	if (process_id > 0) {
		printf("process_id of child process %d \n", process_id);
		exit(0);
	}
	//unmask the file mode
	umask(0);
	//set new session
	sid = setsid();
	if(sid < 0) {
		exit(1);
	}
	// Change the current working directory to root.
	chdir("/");
	// Open a log file in write mode.
	log = fopen("log.txt", "w");
	if (log == NULL) {
		perror("Error opening log file");

		// Program exits if the file pointer returns NULL.
		exit(1);
	}

	config_option_t co;
	if ((co = read_config_file("watchlist.conf")) == NULL) {
		perror("read_config_file()");
		return -1;
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
			docoment_types_count = c;
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
	// Close stdin. stdout and stderr
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	file_printnflush(log, "Starting log file...\n\n");

	while (1) {
		pDir = opendir(dir_to_watch);
		if (pDir == NULL) {
			file_printnflush(log, "Error opening '%s'\n", dir_to_watch);
			exit(1);
		}

		while ((pDirent = readdir(pDir)) != NULL) {
			if (!strcmp(pDirent->d_name, ".") || !strcmp(pDirent->d_name, "..")) {
				continue;
			}

			for (int a = 0; a < types_to_watch_count; a++) {
				if (!strcmp(types_to_watch[a], "audio")) {
					for (int b = 0; b < audio_types_count; b++) {
						if (checkFile(pDirent->d_name, audio_types[b])) {
							file_printnflush(log, "Moving file '%s' to '/home/erikas/Music'. ", pDirent->d_name);
							int ret = move(dir_to_watch, "/home/erikas/Music", pDirent->d_name);
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
							file_printnflush(log, "Moving file '%s' to '/home/erikas/Videos'. ", pDirent->d_name);
							int ret = move(dir_to_watch, "/home/erikas/Videos", pDirent->d_name);
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
							file_printnflush(log, "Moving file '%s' to '/home/erikas/Pictures'. ", pDirent->d_name);
							int ret = move(dir_to_watch, "/home/erikas/Pictures", pDirent->d_name);
							if (ret) {
								file_printnflush(log, "Failed.\n");
							} else {
								file_printnflush(log, "Success!\n");
							}
						}
					}
				}
				if (!strcmp(types_to_watch[a], "document")) {
					for (int b = 0; b < docoment_types_count; b++) {
						if (checkFile(pDirent->d_name, document_types[b])) {
							file_printnflush(log, "Moving file '%s' to '/home/erikas/Documents'. ", pDirent->d_name);
							int ret = move(dir_to_watch, "/home/erikas/Documents", pDirent->d_name);
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
	for (int i = 0; i < docoment_types_count; i++) {
		free(document_types[i]);
	}
	free(document_types);

	// types_to_watch
	for (int i = 0; i < types_to_watch_count; i++) {
		free(types_to_watch[i]);
	}
	free(types_to_watch);

	fclose(log);

	return (0);
}