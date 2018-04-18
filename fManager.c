#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <fcntl.h>

#define BUFFERSIZE    4096
#define COPYMODE      0644

void list();
void dostat(char *);
void show_file_info(char *, struct stat *);
void nameSort(char [][100], char *);
void sizeSort(char [][100], char *);
void timeSort(char [][100], char *);
int getSize(char *);
int getTime(char *);
void copy();

int ac;
char av[5][100];
int arraysize;                                                                      // tracks sorted_dir size

int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("Error: command not provided.\n");
    exit(-1);
  }

  ac = argc;
  int i;
  for (i=0; i < argc; i++) {
    strcpy(av[i], argv[i]);
  }

  if (strcmp(argv[1], "ls") == 0) {
    list();
  } else if (strcmp(argv[1], "cp") == 0) {
    copy();
  } else {
      printf("Error: command not found.\n");
      exit(-1);
  }
}

/* ls command
** Accepts -l, -l -s, -l -t, switches.
** -l, lists in alphabetical order by file name, and displays the size and time-last-modified.
** -l -s, lists in ascending order by file size, and displays the time-last-modified and name.
** -l, lists in latest-first order by time-last-modified, and displays the name and size.
** All commands with all switches, accept a full path, partial path, or no path.
*/
void list() {
  char dirname[50];
  char sorted_dir[30][100];
  DIR             *dir_ptr;
  struct dirent   *direntp;

  if (ac == 2) {                                                                    // ./fManager ls
    strcpy(dirname, ".");
  } else if (ac == 3) {
    if (strcmp(av[2], "-l") == 0) {                                                 // ./fManager ls -l
      strcpy(dirname, ".");
    } else { strcpy(dirname, av[2]); }
  } else if (ac == 4) {
      if (strncmp(av[3], "-", 1) == 0) {
        strcpy(dirname, ".");
      } else { strcpy(dirname, av[3]); }
  } else if (ac == 5) {
    strcpy(dirname, av[4]);
  } else {
    printf("Error: Too many arguments.\n");
    exit(-1);
  }

  if ((dir_ptr = opendir(dirname)) == NULL) {
    fprintf(stderr, "Error: cannot open %s\n", dirname);
  } else {
    chdir(dirname);                                                                 // change directory to path/

    while ((direntp = readdir(dir_ptr)) != NULL) {
      if (strncmp(".", direntp->d_name, 1) == 0) {                                  // omit dot files
        continue;
      }

      if ((strcmp(av[3], "-s") != 0) && (strcmp(av[3], "-t") != 0)) {
        nameSort(sorted_dir, direntp->d_name);
      } else if (strcmp(av[3], "-s") == 0) {
        sizeSort(sorted_dir, direntp->d_name);
      } else if (strcmp(av[3], "-t") == 0) {
        timeSort(sorted_dir, direntp->d_name);
      }
    }

    for(int j = 0; j < arraysize; j++) {
      if (strcmp(av[2], "-l") == 0) {
        dostat(sorted_dir[j]);                                                      // list file info
      } else {
        printf("%-12.12s   ", sorted_dir[j]);                                       // list file names only
      }
    }
    printf("\n");
    closedir(dir_ptr);
  }
}

/* stat()
** Creates the stat structure containing all file info.
*/
void dostat(char *filename) {
  struct stat     info;

  if (stat(filename, &info) == -1) {
    perror(filename);
  } else {
    show_file_info(filename, &info);
  }
}

/*
** Print file info.
*/
void show_file_info(char *filename, struct stat *info_p) {
  printf("%8ld ", (long)info_p->st_size);
  printf("%.12s ", 4+ctime(&info_p->st_mtime));
  printf("%s\n", filename);
}

/* alphabetical order
** Creates an array in alphabetical order.
*/
void nameSort(char dir_array[][100], char *name) {
  int i;

  for (i=arraysize-1; (i >= 0  && (strcmp(dir_array[i], name) > 0)); i--) {
    strcpy(dir_array[i+1], dir_array[i]);
  }

  strcpy(dir_array[i+1], name);
  arraysize++;

}

/* ascending order
** Creates an array in ascending numerical order of file size.
*/
void sizeSort(char dir_array[][100], char *name) {
  int incoming = getSize(name);
  int i;

  for (i=arraysize-1; (i >= 0  && getSize(dir_array[i]) > incoming); i--) {
    strcpy(dir_array[i+1], dir_array[i]);
  }

  strcpy(dir_array[i+1], name);
  arraysize++;
}

/* ascending order
** Creates an array in ascending numerical order of the file's last-modified time.
*/
void timeSort(char dir_array[][100], char *name) {
  int incoming = getTime(name);
  int i;

  for (i=arraysize-1; (i >= 0  && getTime(dir_array[i]) > incoming); i--) {
    strcpy(dir_array[i+1], dir_array[i]);
  }

  strcpy(dir_array[i+1], name);
  arraysize++;
}

/*
** Returns filesize of a file that is passed as a parameter.
*/
int getSize(char *filename) {
  struct stat     info;
  int filesize;

  if (stat(filename, &info) == -1) {
    perror(filename);
  } else {
    filesize = info.st_size;
  }

  return filesize;
}

/*
** Returns last-modified time of a file that is passed as a parameter.
*/
int getTime(char *filename) {
  struct stat     info;
  int filetime;

  if (stat(filename, &info) == -1) {
    perror(filename);
  } else {
    filetime = info.st_mtime;
  }

  return filetime;

}

/* basic Linux copy function
** Copies given file to a new name and in a chosen location.
** Acknowledment of "Understanding Unix/Linux Programming" by Bruce Molay (2003).
*/
void copy() {
  int in_fd, out_fd, n_chars;
  char buf[BUFFERSIZE];

  if (ac != 4) {
    fprintf(stderr, "usage: %s source destination\n", *av);
    exit(1);
  }

  if ((in_fd = open(av[2], O_RDONLY)) == -1) {
    printf("Error: Cannot open ");
    perror(av[2]);
  }
  if ((out_fd = creat(av[3], COPYMODE)) == -1) {
    printf("Error: Cannot create ");
    perror(av[3]);
  }

  while ((n_chars = read(in_fd, buf, BUFFERSIZE)) > 0) {
    if (write(out_fd, buf, n_chars) != n_chars) {
      printf("Error: Write error to ");
      perror(av[3]);
    }
  }

  if (n_chars == -1) {
    printf("Error: Read error to ");
    perror(av[2]);
  }

  if (close(in_fd) == -1 || close(out_fd) == -1) {
    printf("Error closing files\n");
  }
}
