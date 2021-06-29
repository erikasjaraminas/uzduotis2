#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <file_operations.h>
#include <parser.h>

#define READ_ELEMENTS 5

struct Info
{
	char **value;
	char key_name[48];
	char folder_name[24];
	int count;
};

struct Info keys_values[READ_ELEMENTS];

char dir_to_watch[256];
FILE *log_file = NULL;

void free_up_memory() {

	for (int x = 0; x < READ_ELEMENTS; x++) {
		for (int y = 0; y < keys_values[x].count; y++) {
			if (keys_values[x].value[y] != NULL) {
				free(keys_values[x].value[y]);
			}
		}
		if (keys_values[x].value != NULL) {
			free(keys_values[x].value);
		}
	}
}

void exit_program(int status) {

	free_up_memory();

	exit(status);
}

void assign_key_values() {

	strcpy(keys_values[0].key_name, "audio_types");
	strcpy(keys_values[1].key_name, "video_types");
	strcpy(keys_values[2].key_name, "photo_types");
	strcpy(keys_values[3].key_name, "document_types");
	strcpy(keys_values[4].key_name, "types_to_watch");

	strcpy(keys_values[0].folder_name, "%s/Music");
	strcpy(keys_values[1].folder_name, "%s/Videos");
	strcpy(keys_values[2].folder_name, "%s/Pictures");
	strcpy(keys_values[3].folder_name, "%s/Documents");
}

int read_values() {

	config_option_t co;
	if ((co = read_config_file("/etc/watchlist.conf")) == NULL) {
		return 1;
	}
	
	int c = 0;
	char *token;
	while(1) {
		if (!strcmp(co->key, "dir_to_watch")) {
			strcpy(dir_to_watch, co->value);
		}
		for (int i = 0; i < READ_ELEMENTS; i++) {
			if (!strcmp(co->key, keys_values[i].key_name)) {
				keys_values[i].value[0] = (char *) malloc(5 * sizeof(char));
				if (keys_values[i].value[0] == NULL) {
					printf("Error creating variables\n");
					return 1;
				}
				token = strtok(co->value, ",");
				strcpy(keys_values[i].value[0], token);
				token = strtok(NULL, ",");
				c = 1;
				while (token != NULL) {
					keys_values[i].value = realloc(keys_values[i].value, (c + 1) * sizeof(char *));
					keys_values[i].value[c] = strdup(token);
					token = strtok(NULL, ",");
					c++;
				}
				keys_values[i].count = c;
			}
		}
		if (co->prev != NULL) {
			co = co->prev;
		} else {
			break;
		}
	}

	return 0;
}

int create_daemon_process() {

	pid_t process_id = 0;
	pid_t sid = 0;
	// Create child process
	process_id = fork();
	// Indication of fork() failure
	if (process_id < 0) {
		printf("fork failed!\n");
		return process_id;
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
		return sid;
	}
	// Change the current working directory to root.
	chdir("/");
	// Close stdin. stdout and stderr
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	return 0;
}

void move_files(int key, char *filename) {

	char *homedir = getenv("HOME");
	char newpath[256];

	for (int b = 0; b < keys_values[key].count; b++) {
		if (checkFile(filename, keys_values[key].value[b])) {
			sprintf(newpath, keys_values[key].folder_name, homedir);
			file_printnflush(log_file, "Moving file '%s' to '%s'. ", filename, newpath);
			int ret = move(dir_to_watch, newpath, filename);
			if (ret) {
				file_printnflush(log_file, "Failed.\n");
			} else {
				file_printnflush(log_file, "Success!\n");
			}
		}
	}
}

int main() {

	for (int i = 0; i < READ_ELEMENTS; i++) {
		keys_values[i].value = (char **) malloc(1 * sizeof(char *));

		if (keys_values[i].value == NULL) {
			printf("Error creating variables\n");

			exit_program(1);
		}
	}

	struct dirent *pDirent;
	DIR *pDir = NULL;
	FILE *conf = NULL;
	// Open a log file in write mode.
	log_file = fopen("/var/log/daemon.log", "w");
	if (log_file == NULL) {
		perror("Error opening log file");

		// Program exits if the file pointer returns NULL.
		exit_program(1);
	}
	file_printnflush(log_file, "Starting log file...\n\n");

	assign_key_values();
	if (read_values()) {
		printf("Error reading configuration file.\n");
		exit_program(1);
	}
	if (create_daemon_process() < 0) {
		printf("Error creating daemon process.\n");
		exit_program(1);
	}

	while (1) {
		pDir = opendir(dir_to_watch);
		if (pDir == NULL) {
			file_printnflush(log_file, "Error opening '%s'\n", dir_to_watch);
			exit_program(1);
		}

		while ((pDirent = readdir(pDir)) != NULL) {
			if (!strcmp(pDirent->d_name, ".") || !strcmp(pDirent->d_name, "..")) {
				continue;
			}

			for (int a = 0; a < keys_values[4].count; a++) {
				if (!strcmp(keys_values[4].value[a], "audio")) {
					move_files(0, pDirent->d_name);
				}
				if (!strcmp(keys_values[4].value[a], "video")) {
					move_files(1, pDirent->d_name);
				}
				if (!strcmp(keys_values[4].value[a], "photo")) {
					move_files(2, pDirent->d_name);
				}
				if (!strcmp(keys_values[4].value[a], "document")) {
					move_files(3, pDirent->d_name);
				}
			}
		}

		closedir(pDir);

		sleep(5);
	}

	free_up_memory();

	fclose(log_file);

	return (0);
}