/* Author(s): Jorge Tadeo & Taylor Allen
 *
 * This is lab10.c the csc60mshell
 * This program serves as a skeleton for doing labs 9, 10, 11.
 * Student is required to use this program to build a mini shell
 * using the specification as documented in direction.
 * Date: Fall 2018
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define MAXLINE 80
#define MAXARGS 20
#define MAX_PATH_LENGTH 5000
#define TRUE 1

/* function prototypes */
int parseline(char *cmdline, char **argv);
void process_input(int argc, char **argv);
void handle_redir(int count, char *argv[]);

/* ----------------------------------------------------------------- */
/*                  The main program starts here                     */
/* ----------------------------------------------------------------- */

int main(void){
  char cmdline[MAXLINE];
  char *argv[MAXARGS];
  char path[MAX_PATH_LENGTH];
  char dir[MAX_PATH_LENGTH];
  int argc;
  int status;
  pid_t pid;
  int loop;

  /* Loop forever to wait and process commands */
  do{
    /* Print your shell name: csc60mshell (m for mini shell) */
    printf("csc60mshell > ");

    /* Read the command line */
    fgets(cmdline, MAXLINE, stdin);

    /* Call parseline to build argc/argv */
    argc = parseline(cmdline, argv);

    /* Print out argc and argv list */
    for(loop = 0; loop < argc ; loop++)
      printf("Argc %i = %s\n",loop,argv[loop]);

    /* if user passes no args return shell prompt */
    if(argc == 0){
      continue;
    }

    /* Check for exit, pwd, or cd */
      else if(strcmp(argv[0], "exit") ==  0){
        return EXIT_SUCCESS;
    } else if (strcmp(argv[0], "pwd") == 0){                   // if argv[0] equals pwd print the current working dir
        (getcwd(path, MAX_PATH_LENGTH)) ? printf("%s\n", path) : fprintf(stderr, "Cannot Print The Current Working Directory.\n");
      continue;
    } else if (strcmp(argv[0], "cd") == 0 && argv[1] == NULL){ // if argv[0] equals cd without a second arg change dir to HOME
        chdir(getenv("HOME"));
        continue;
    } else if (strcmp(argv[0], "cd") == 0){                    // if cd has a second arg change dir to the path specified
        if(argc == 1){
          chdir(getenv("HOME"));
          continue;
        } else{
            if (chdir(argv[1]) == -1){
              int dirErr = chdir(argv[1]);
              (dirErr < 0) ? perror("Error") : chdir(argv[1]);
              continue;
            }
        }
    } else{ /* Else, fork off a process */
        pid = fork();
        switch(pid){
          case -1:
            perror("Shell Program fork error");
                exit(EXIT_FAILURE);
          case 0:
          /* I am child process. I will execute the command, */
          /* and call: execvp */
          process_input(argc, argv);
          break;
          default:
          /* I am parent process */
          if (wait(&status) == -1)
            perror("Parent Process error");
          else
            fprintf(stderr, "Child returned status: %d\n");
          break;
        }   /* end of the switch */
    }	/* end of the if-else-if */
  }	while (TRUE); /* end of the while */
}     /* end of main */

/* ----------------------------------------------------------------- */
/*                  parseline                                        */
/* ----------------------------------------------------------------- */
/* parse input line into argc/argv format */

int parseline(char *cmdline, char **argv){
  int count = 0;
  char *separator = " \n\t"; /* Includes space, Enter, Tab */

  /* strtok searches for the characters listed in separator */
  argv[count] = strtok(cmdline, separator);

  while ((argv[count] != NULL) && (count+1 < MAXARGS))
    argv[++count] = strtok((char *) 0, separator);

  return count;  //This becomes "argc" back in main.
}

/* ----------------------------------------------------------------- */
/*                  process_input                                    */
/* ----------------------------------------------------------------- */
void process_input(int argc, char **argv) {
  int out;

  handle_redir(argc,argv);
  out = execvp(argv[0],argv);

  if (out == -1) {
      fprintf(stderr, "Error on the exec call\n");
      _exit(EXIT_FAILURE);
  }
}

/* ----------------------------------------------------------------- */
/*                  handle_redir                                     */
/* ----------------------------------------------------------------- */
void handle_redir(int count, char *argv[]){
  /* int out_redir = dup(1); */
  /* int in_redir  = dup(0); */
  int out_redir = 0;
  int in_redir  = 0;
  int loop;

  for(loop = 0; loop < count ; loop++){
    if(strcmp(argv[loop], ">") ==  0){
      if(out_redir != 0){
        printf("Error: Cannot output to more than one file.\n");
        _exit(EXIT_FAILURE);
      }
      else if(loop == 0){
        fprintf(stderr, "Error: No command entered.\n");
        _exit(EXIT_FAILURE);
      }
      out_redir = loop;
    }
    else if(strcmp(argv[loop], "<") ==  0){
      if(in_redir != 0){
        fprintf(stderr, "Error: Cannot input to more than one file.\n");
        _exit(EXIT_FAILURE);
      }
      else if(loop == 0){
        fprintf(stderr, "Error: No command entered.\n");
        _exit(EXIT_FAILURE);
      }
      in_redir = loop;
    }
  }
  if(out_redir != 0){
    if(argv[out_redir+1] == NULL){
      fprintf(stderr, "Error: No command entered.\n");
      _exit(EXIT_FAILURE);
    }
    int out_fd = open(argv[out_redir+1], /* O_RDWR */ O_WRONLY | O_CREAT | O_TRUNC, 0755);
    /* if(out_fd == -1) perror("Error opening file"); */
    if(out_fd == -1) perror("Redir Error");
    dup2(out_fd,1);
    close(out_fd);
    argv[out_redir] = NULL;
  }
  if(in_redir != 0){
    if(argv[in_redir+1] == NULL){
      fprintf(stderr, "Error: No file\n");
        _exit(EXIT_FAILURE);
    }
    int in_fd = open(argv[in_redir+1],O_RDONLY);

    dup2(in_fd,0);
    if(in_fd == -1) perror("Redir Error");
    close(in_fd);
    argv[in_redir] = NULL;
  }
}
/* ----------------------------------------------------------------- */
