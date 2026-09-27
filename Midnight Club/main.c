#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define i_boolean short int
#define i_true 1
#define i_false 0

#define MAX_DIRECTORY_SIZE 2048
#define MAX_SELECTED_SIZE 2048

struct dirent *de;
DIR *directory = NULL;
struct termios original_settings;
struct termios modified_settings;

// ^[[D ^[[C ^[[B ^[[A

unsigned int selected = 0;
char selected_name[MAX_SELECTED_SIZE];
unsigned int objects_in_directory = 0;
char input;
char key_sequence[3];
void print_directory(void)
{
	char cwd[MAX_DIRECTORY_SIZE];
	unsigned int i = 0;

	getcwd(cwd, sizeof(cwd));
	printf("<| %s |>\n", cwd);

	printf("~ %u | $ %u\n\n",objects_in_directory,selected);
	objects_in_directory = 0;

	rewinddir(directory);
	while ( (de = readdir(directory)) != NULL) {
		if (strcmp(".", de->d_name) == 0) {
			continue;
		}

		if (objects_in_directory == selected) {
			strcpy(selected_name,de->d_name);
		}

		if (de->d_type == DT_DIR) {
			printf("/ ");
		} else if (de->d_type == DT_REG) {
			printf("- ");
		}

		printf("%s",de->d_name);
		if (objects_in_directory == selected) {
			printf(" <\n");
		} else {
			printf("\n");
		}

		objects_in_directory++;
	}
	fflush(stdout);
}

int main(void)
{
	directory = opendir(".");

	tcgetattr(STDIN_FILENO, &original_settings);

	modified_settings = original_settings;
	modified_settings.c_lflag &= (~ICANON & ~ECHO);
	modified_settings.c_cc[VMIN] = 3;
	modified_settings.c_cc[VTIME] = 0;
	printf("\033[2J\033[H");
	print_directory();
	tcsetattr(STDIN_FILENO, TCSANOW, &modified_settings);

	while (1) {
		read(STDIN_FILENO, key_sequence, 1);
		if (key_sequence[0] == 'q') {
			break;
		}

		if ( key_sequence[0] == 10 || key_sequence[0] == 13) {
			if (de->d_type == DT_DIR) {
				closedir(directory);
				chdir(selected_name);
				directory = opendir(".");
				selected = 0;
			} else if (de->d_type == DT_REG) {

			}
		}

		if ( key_sequence[0] == 27) {
			read(STDIN_FILENO, &key_sequence[1], 2);
			if (key_sequence[1] == 91) {
				if (key_sequence[2] == 65 && selected > 0) {
					selected--;
				} else if (key_sequence[2] == 66 && selected < objects_in_directory - 1) {
					selected++;
				}
			}
		}

		printf("\033[2J\033[H");
		fflush(stdout);
		print_directory();
	}
	chdir(selected_name);
	
	closedir(directory);
	tcsetattr(STDIN_FILENO, TCSANOW, &original_settings);
	return 0;
}
