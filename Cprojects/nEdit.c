/* COMPILE & RUN WITH:
gcc -O3 -o nEdit ./nEdit.c
./nEdit ./Other/TestFile.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define BOLD "\033[1m"
#define RED   "\033[38;2;230;65;50m"
#define GREEN "\033[38;2;50;230;75m"
#define WHITE "\033[38;2;255;255;255m"
#define LGRAY "\033[38;2;210;210;210m"
#define GRAY  "\033[38;2;140;140;140m"
#define DGRAY "\033[38;2;70;70;70m"
#define BLACK "\033[38;2;0;0;0m"
#define BG_WHITE "\033[48;2;255;255;255m"
#define BG_LGRAY "\033[48;2;210;210;210m"
#define BG_GRAY  "\033[48;2;140;140;140m"
#define BG_DGRAY "\033[48;2;70;70;70m"
#define BG_BLACK "\033[48;2;0;0;0m"
#define RESET    "\033[0m"

#define END_OF_LINE "\033[K"

#define CURSOR_HOME "\033[H"

#define MAX_LINE_LENGTH (2<<16) //64 KiB
#define MAX_INPUT_BUF_SIZE (2<<16) //64 KiB
#define MAX_LINE_COLUMNS (2<<10) //1 KiB

struct Pallete { char *console, *info_bar, *editor; };

struct Globals {
	char consoleBuf[MAX_LINE_COLUMNS];
	char infoBarBuf[MAX_LINE_COLUMNS];
	char **fileBuf;
	int fileLinesCount;
	struct Pallete pall;
	struct winsize win;
	int editLineStart;
	int state;
};

char **fileToLines(FILE *file, int *lineCount) {
	char **lines = NULL;
	char lineBuf[MAX_LINE_LENGTH];
	*lineCount = 0;
	while (fgets(lineBuf, sizeof(lineBuf), file)) {
		lines = realloc(lines, sizeof(char *) * (*lineCount + 1));
		lines[*lineCount] = malloc(strlen(lineBuf) + 1);
		strcpy(lines[*lineCount], lineBuf);
		(*lineCount)++;
	}

	return lines;
}

void render(const struct Globals set) {
	system("clear");
	printf("%s%s>> %s%s%s\n", BOLD, set.pall.console, set.consoleBuf, END_OF_LINE, RESET);
	printf("%s%s%s%s\n", set.pall.info_bar, set.infoBarBuf, END_OF_LINE, RESET);

	int maxLines = (set.fileLinesCount < set.win.ws_row) ? set.fileLinesCount : set.win.ws_row;
	for (int i = set.editLineStart-1; i < maxLines; i++) {
		printf("%s%4d > %s%s", set.pall.editor, i+1, set.fileBuf[i], END_OF_LINE);
	} printf(RESET);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf(RED " Error! Incorrect usage.\n Correct usage: nEdit <file>\n" RESET);
		return 1;
	}

	FILE *file = fopen(argv[1], "r+");
	if (file == NULL) {
		printf(RED " Error! Cant open %s\n Does the file exist?\n" RESET, argv[1]);
		return 1;
	}

	struct winsize win; ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
	struct Pallete pall = {
		"\033[38;2;235;245;240m\033[48;2;50;52;65m", //console
		"\033[38;2;195;205;200m\033[48;2;41;44;55m", //info bar
		"\033[38;2;236;214;213m\033[48;2;18;19;23m"  //editor
	};
	int fileLinesCount = 0; char **fileBuf = fileToLines(file, &fileLinesCount);
	struct Globals set = {"Command line", "Pete loves cinnamon rolls. Who the fuck is Pete though?", fileBuf, fileLinesCount, pall, win, 1, 0};

	while (set.state == 0) {
		render(set);
		char inputBuf[MAX_INPUT_BUF_SIZE];
		if (fgets(inputBuf, sizeof(inputBuf), stdin) == NULL) break;
		inputBuf[MAX_INPUT_BUF_SIZE-1] = '\0'; inputBuf[strcspn(inputBuf, "\n")] = '\0';
		strcpy(set.consoleBuf, inputBuf);
	}

	fclose(file);
	return 0;
}
