/* ICCS227: Project 1: icsh
 * Name: Devya Shah
 * StudentID: 6480253
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_CMD_BUFFER 255

pid_t foreground_pid = -1; 
struct job_t {              
    pid_t pid;              
    int jid;                
    int state;              
    char cmdline[255];
};
struct job_t job_list[MAXJOBS]; /* The job list */

void handle1(int signum) {
    printf("\n");
 }
 
 void handle2(int signum) {
    printf("\n");
 }

void sigtstp_set(void) {
    struct sigaction sa;
    sa.sa_handler = handle1;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTSTP, &sa, NULL);
}

void sigint_set(void) {
    struct sigaction sa;
    sa.sa_handler = handle2;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
}

void actions(char *buffer, char *oldbuffer) {
    buffer[strcspn(buffer, "\n")] = '\0';
    if (strcmp(buffer, "!!") == 0) {
        if (strlen(oldbuffer) == 0) {
            printf("No previous command to repeat.\n");
            return;
        }
        printf("\n%s\n", oldbuffer);
        actions(oldbuffer, oldbuffer);
        return;
    }

    if (strstr(buffer, "echo ") == buffer) {
        char *newbuf = malloc(strlen(buffer) - 5 + 1);
        if (!newbuf) return;
        strncpy(newbuf, &buffer[5], strlen(buffer) - 5);
        newbuf[strlen(buffer) - 5] = '\0';
        printf("%s\n", newbuf);
        free(newbuf);
        strncpy(oldbuffer, buffer, MAX_CMD_BUFFER);
        return;
    }

    if (strstr(buffer, ">") != NULL) {
        int i = 0;
        char delimiter[] = ">";
        char *tokens[2];
        char *token1 = strtok(buffer, delimiter);
        while (token1 != NULL && i < 2) {
            tokens[i++] = token1;
            token1 = strtok(NULL, delimiter);
        }
        tokens[i] = NULL;

        int saved_stdout = dup(1);
        int file_desc = open(tokens[1], O_APPEND | O_CREAT | O_WRONLY, 0666);
        dup2(file_desc, 1);
        actions(tokens[0], buffer);
        fflush(stdout);
        dup2(saved_stdout, 1);
        close(saved_stdout);
        close(file_desc);
        return;
    }

    if (strstr(buffer, "<") != NULL) {
        int i = 0;
        char delimiter[] = "<";
        char *tokens[2];
        char *token1 = strtok(buffer, delimiter);
        while (token1 != NULL && i < 2) {
            tokens[i++] = token1;
            token1 = strtok(NULL, delimiter);
        }
        char *cmd = tokens[0];
        char *filename = tokens[1];
        int saved_stdin = dup(0);
        int file_desc = open(filename, O_RDONLY);
        dup2(file_desc, 0);
        close(file_desc);
        actions(cmd, buffer);
        dup2(saved_stdin, 0);
        close(saved_stdin);
        return;
    }

    if (strstr(buffer, "exit ") == buffer) {
        char *newbuf = malloc(strlen(buffer) + 1);
        if (!newbuf) {
            perror("malloc failed");
            return;
        }
        strncpy(newbuf, &buffer[5], strlen(buffer) - 1);
        newbuf[strlen(buffer) - 1] = '\0';
        int result = atoi(newbuf);
        free(newbuf);
        if (result < 0 || result > 255) {
            result = result & 0xFF;
        }
        printf("Exit code: %d\n", result);
        printf("Bye\n");
        exit(result);
    }

    int status;
    pid_t pid = fork();
    char delimiter[] = " ";
    char *token;
    char *tokens[64];
    int i = 0;

    token = strtok(buffer, delimiter);
    while (token != NULL && i < 63) {
        tokens[i++] = token;
        token = strtok(NULL, delimiter);
    }
    tokens[i] = NULL;

    if (pid < 0) return;
    if (pid == 0) {
        execvp(tokens[0], tokens);
        perror("execvp failed");
        exit(1);
    } else {
        foreground_pid = pid;
        waitpid(pid, &status, WUNTRACED);
        foreground_pid = -1;
    }

    strncpy(oldbuffer, buffer, MAX_CMD_BUFFER);
}

int main(int argc, char *argv[]) {
    char buffer[MAX_CMD_BUFFER];
    char oldbuffer[MAX_CMD_BUFFER];
    oldbuffer[0] = '\0';

    sigint_set();
    sigtstp_set();
    
    if (argc == 2) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            perror("Failed to open script file");
            return 1;
        }
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            printf("%s", line);
            actions(line, oldbuffer);
        }
        fclose(f);
        return 0;
    }

    while (1) {
        printf("icsh $ ");
        fflush(stdout);

        if (fgets(buffer, MAX_CMD_BUFFER, stdin) == NULL) {
            printf("\nBye\n");
            break;
        }

        actions(buffer, oldbuffer);
    }

    return 0;
}

 
 
 