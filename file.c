#include <dirent.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
int rows, cols;

#define MAX_FILES 1024
#define MAX_NAME 256

char files[MAX_FILES][MAX_NAME];
int file_count = 0;
int selected = 0;
char cwd[1024] = "/";

void load_dir(const char *path) {
  DIR *dir;
  struct dirent *entry;

  file_count = 0;
  selected = 0;

  dir = opendir(path);
  if (!dir)
    return;

  while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
    strncpy(files[file_count++], entry->d_name, MAX_NAME);
  }
  closedir(dir);
}

int is_dir(const char *name) {
  struct stat st;
  char fullpath[1024];

  snprintf(fullpath, sizeof(fullpath), "%s/%s", cwd, name);
  if (stat(fullpath, &st) == 0)
    return S_ISDIR(st.st_mode);

  return 0;
}

void go_back() {
  if (strcmp(cwd, "/") == 0)
    return;

  char *p = strrchr(cwd, '/');
  if (p && p != cwd)
    *p = '\0';
  else
    strcpy(cwd, "/");

  load_dir(cwd);
}
void open_file(const char *filename, const char *cwd) {
  char fullpath[1024];
  snprintf(fullpath, sizeof(fullpath), "%s/%s", cwd, filename);

  pid_t pid = fork();
  if (pid == 0) {

    execlp("xdg-open", "xdg-open", fullpath, (char *)NULL);
    exit(1);
  }
}

int main() {
  int ch;

  initscr();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);
  curs_set(0);

  load_dir(cwd);

  while (1) {
    clear();
    getmaxyx(stdscr, rows, cols);

    // Print header centered
    char header[1024];
    snprintf(header, sizeof(header),
             "Dir: %s  (enter=open  left arrow key=back  q=quit)", cwd);
    mvprintw(1, (cols - strlen(header)) / 2, "%s", header);

    // Print files centered
    int start_y = (rows - file_count) / 2;
    if (start_y < 3)
      start_y = 3;

    for (int i = 0; i < file_count; i++) {
      char line[512];
      snprintf(line, sizeof(line), "%s%s", files[i],
               is_dir(files[i]) ? "/" : "");
      int x = (cols - strlen(line)) / 2;
      int y = start_y + i;

      if (i == selected)
        attron(A_REVERSE);

      mvprintw(y, x, "%s", line);

      if (i == selected)
        attroff(A_REVERSE);
    }

    // Get user input
    ch = getch();

    // Handle keys
    if (ch == 'q') {
      break;
    } else if (ch == KEY_UP && selected > 0) {
      selected--;
    } else if (ch == KEY_DOWN && selected < file_count - 1) {
      selected++;
    } else if (ch == KEY_LEFT) {
      go_back();
    } else if (ch == '\n' && file_count > 0) {
      if (is_dir(files[selected])) {
        char newpath[1024];
        if (strcmp(files[selected], "..") == 0) {
          go_back();
        } else if (strcmp(files[selected], ".") != 0) {
          if (strcmp(cwd, "/") == 0)
            snprintf(newpath, sizeof(newpath), "/%s", files[selected]);
          else
            snprintf(newpath, sizeof(newpath), "%s/%s", cwd, files[selected]);

          strncpy(cwd, newpath, sizeof(cwd));
          load_dir(cwd);
        }
      } else {
        open_file(files[selected], cwd);
      }
    }
  }

  // Move endwin() after loop
  endwin();
  return 0;
}
