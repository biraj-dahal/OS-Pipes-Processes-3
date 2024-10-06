#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "You need to pass an argument for grep search term\n");
        exit(1);
    }

    char *search_term = argv[1];
    int P1_to_P2[2], P2_to_P3[2];

    char *cat_args[] = {"cat", "scores", NULL};
    char *grep_args[] = {"grep", search_term, NULL};
    char *sort_args[] = {"sort", NULL};

    // Create pipe for P1 P2 communication
    if (pipe(P1_to_P2) == -1) {
        perror("pipe");
        exit(1);
    }

    // Fork P1
    pid_t pid_P1 = fork();
    if (pid_P1 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid_P1 == 0) { // P1 - executes "cat scores"
        close(P1_to_P2[0]); 
        dup2(P1_to_P2[1], STDOUT_FILENO); 
        close(P1_to_P2[1]);

        execvp("cat", cat_args);
        perror("execvp cat failed");
        exit(1);
    }

    // Create pipe for P2 -> P3 communication
    if (pipe(P2_to_P3) == -1) {
        perror("pipe");
        exit(1);
    }

    // Fork P2
    pid_t pid_P2 = fork();
    if (pid_P2 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid_P2 == 0) { 
        close(P1_to_P2[1]); 
        dup2(P1_to_P2[0], STDIN_FILENO); 
        close(P1_to_P2[0]);

        close(P2_to_P3[0]); 
        dup2(P2_to_P3[1], STDOUT_FILENO); 
        close(P2_to_P3[1]);

        execvp("grep", grep_args);
        perror("execvp grep failed");
        exit(1);
    }

    // Close unused pipe ends in the parent
    close(P1_to_P2[0]);
    close(P1_to_P2[1]);
    close(P2_to_P3[1]);

    // Fork P3
    pid_t pid_P3 = fork();
    if (pid_P3 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid_P3 == 0) {
        dup2(P2_to_P3[0], STDIN_FILENO); 
        close(P2_to_P3[0]);

        execvp("sort", sort_args);
        perror("execvp sort failed");
        exit(1);
    }

    close(P2_to_P3[0]);

    // Wait for P1, P2, and P3 to complete
    waitpid(pid_P1, NULL, 0);
    waitpid(pid_P2, NULL, 0);
    waitpid(pid_P3, NULL, 0);

    return 0;
}
/*
Need to ask professor, cant stop the child processes while making the parent work.

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


int main(int argc, char **argv)
{
  if(argc != 2){
    printf("Needs the argument as search term for grep");
    return 1;
  }

  char *cat_args[] = {"cat", "scores", NULL};
  char *grep_args[] = {"grep", argv[1], NULL};
  char *sort_args[] = {"sort", NULL};

  int pipefd1[2];
  int pipefd2[2];

  // make a pipe for P1 and P2 communication
  if (pipe(pipefd1)==-1){
    printf("Pipe for pipefd1 failed");
    return 1;
  }

  int pid1 = fork();
  if(pid1 < 0){
    printf("Fork From P1 to P2 failed");
    return 1;
  }

  if (pid1 == 0)
    {
      // close unused pipes
      close(pipefd1[1]);

      // make a pipe for P2 P3 Communication
      if (pipe(pipefd2)==-1){
        printf("Pipe for pipefd2 failed");
        return 1;
      }

        int pid2 = fork();
          if(pid2 < 0){
            printf("Fork From P2 to P3 failed");
            return 1;
          }
      if(pid2 == 0){
        close(pipefd2[1]);

        dup2(pipefd2[0], STDIN_FILENO);

        execvp("sort", sort_args);
        exit(1);
      }
      else{
        close(pipefd1[1]);
        close(pipefd2[0]);
        
        // replace standard input with input part of pipe
        dup2(pipefd1[0], STDIN_FILENO);
        dup2(pipefd2[1], STDOUT_FILENO);

        // close unused half of pipe
        close(pipefd1[0]);
        close(pipefd2[1]);

        // execute grep
        execvp("grep", grep_args);
        exit(1);
      }


    }
  else
    {
      close(pipefd1[0]);
      close(pipefd2[0]);
      close(pipefd2[1]);

      // replace standard output with output part of pipe
      dup2(pipefd1[1], STDOUT_FILENO);
      // close unused unput half of pipe
      close(pipefd1[1]);

      // execute cat
      execvp("cat", cat_args);

      exit(1);
    }
}
*/