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
#include <errno.h>
#include <sys/ioctl.h> 

#define MAX_CMD_BUFFER 255
#define CMDLINE_LEN 255

//Citations: https://github.com/yichuns/Shell-Lab/blob/master/tsh.c 
pid_t foreground_pid = -1; 
int fg = 0;
struct job_t {
    pid_t pid;
    int jid;
    int state;
    int wifstop;
    char cmdline[CMDLINE_LEN];
    struct job_t *next;  
};

struct job_t *start = NULL;
int jid1 = 1;
int excode = 0;

//Citations: https://www.programiz.com/dsa/linked-list-operations
void add_job(pid_t pid, int state, const char *cmdline, int wifstop) {
    struct job_t *jobb = malloc(sizeof(struct job_t));
    if (!jobb) {
        printf("cant");
        return;
    }
    jobb->wifstop = wifstop;
    jobb->pid = pid;
    jobb->jid = jid1++;
    jobb->state = state;
    int len = strlen(cmdline);
    if (len >= CMDLINE_LEN) {
        len = CMDLINE_LEN - 1;
    }
    strncpy(jobb->cmdline, cmdline, len);
    jobb->cmdline[len] = '\0'; 
    jobb->next = start;
    start = jobb;
    if(wifstop == 0){
        printf("[%d+]        running         %s \n", jobb->jid, jobb->cmdline);
    }
}

void list_jobs() {
    struct job_t *curr = start;
    while (curr) {
        if(curr->state == 1){
            if (curr->next == NULL) {
                if(curr->wifstop == 0){
                    printf("[%d]  +     running         %s \n", curr->jid, curr->cmdline);
                    curr = curr->next;
                } else {
                    printf("[%d]  +     suspended       %s \n", curr->jid, curr->cmdline);
                    curr = curr->next;
                }
            }
            else if (curr->next->next == NULL) {
                if(curr->wifstop == 0){
                    printf("[%d]  -     running         %s \n", curr->jid, curr->cmdline);
                    curr = curr->next;
                } else {
                    printf("[%d]  -     suspended       %s \n", curr->jid, curr->cmdline);
                    curr = curr->next;
                }
            }
            else {
                if(curr->wifstop == 0){
                    printf("[%d]        running         %s \n", curr->jid, curr->cmdline);
                    curr = curr->next;
                } else {
                    printf("[%d]        suspended       %s \n", curr->jid, curr->cmdline);
                    curr = curr->next;
                }
            }
        } else {
            return;
        }
    }
}

void child_handler(int signum) {
    int status;
    //Citation: https://gist.github.com/udaya1223/10730660
    pid_t p;
    while((p = waitpid(-1, &status, WNOHANG)) > 0){
        struct job_t *curr = start;
        struct job_t *prev = NULL;
        while (curr) {
            if (curr->pid == p) {
                printf("\n[%d]        Done           %s\n", curr->jid, curr->cmdline);
                printf("If this command interrupted when you were typing into the shell, please copy and paste your buffer again or press enter if your buffer was done\n");
                if (prev) {
                    prev->next = curr->next;
                } else {
                    start = curr->next;
                }
                free(curr);
                break;
            }
            prev = curr;
            curr = curr->next;
        }
        if(fg == 0){
            write(STDOUT_FILENO, "icsh $ ", 7);
        } else {
            //Citation: https://chatgpt.com/share/683ad879-ee8c-800f-9d5f-edb911326c62
            char enter = '\n';
            ioctl(STDIN_FILENO, TIOCSTI, &enter);
        }
        fflush(stdout);
    }
}

void remove_job(pid_t p){
    //Citation: https://gist.github.com/udaya1223/10730660
    struct job_t *curr = start;
    struct job_t *prev = NULL;
    while (curr) {
        if (curr->pid == p) {
            if (prev) {
                prev->next = curr->next;
            } else {
                start = curr->next;
            }
            free(curr);
            break;
        }
        prev = curr;
        curr = curr->next;
    }
}

void sigchld_set(){
    struct sigaction sa_chld;
    sa_chld.sa_handler = child_handler;
    sa_chld.sa_flags = SA_RESTART;
    sigemptyset(&sa_chld.sa_mask);
    sigaction(SIGCHLD, &sa_chld, NULL);
}

void handle1(int signum) {
    write(STDOUT_FILENO, "\nicsh $ ", 8);
}

void handle2(int signum) {
    write(STDOUT_FILENO, "\nicsh $ ", 8);
}

// Citation: https://stackoverflow.com/a/40116030/17123296
// Citation: https://pubs.opengroup.org/onlinepubs/007904875/functions/sigaction.html 
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
    char tempbuffer[CMDLINE_LEN];
    strncpy(tempbuffer, buffer, CMDLINE_LEN - 1);
    tempbuffer[CMDLINE_LEN - 1] = '\0';

    if (strcmp(buffer, "!!") == 0) {
        char tempbuffer2[MAX_CMD_BUFFER];
        strncpy(tempbuffer2, oldbuffer, MAX_CMD_BUFFER);
        if (tempbuffer2[0] == '\0') {
            printf("No previous command.\n");
            excode = 1;
            return;
        }
        printf("%s\n", tempbuffer2);
        if((strstr(tempbuffer2, ">") != NULL) || (strstr(tempbuffer2, "<") != NULL)){
            redir(tempbuffer2);
            return;
        }
        actions(tempbuffer2, oldbuffer);
        return;
    }
    if(buffer[0] != '\0'){
        strncpy(oldbuffer, buffer, MAX_CMD_BUFFER);
    }

    if (strstr(buffer, "echo ") == buffer) {
        strncpy(oldbuffer, buffer, MAX_CMD_BUFFER);
        if (strcmp(buffer, "echo $?") == 0) {
            printf("%d\n", excode);
            return;
        }
        char *newbuf = malloc(strlen(buffer) - 5 + 1);
        if (!newbuf) return;
        strncpy(newbuf, &buffer[5], strlen(buffer) - 5);
        newbuf[strlen(buffer) - 5] = '\0';
        printf("%s\n", newbuf);
        free(newbuf);
        excode = 0;
        return;
    }

    if (strstr(buffer, "jobs") == buffer) {
        strncpy(oldbuffer, buffer, MAX_CMD_BUFFER);
        list_jobs();
        excode = 0;
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

    if (strstr(buffer, "fg %") == buffer) {
        strncpy(oldbuffer, buffer, MAX_CMD_BUFFER);
        int job = atoi(buffer + 4);
        struct job_t *curr = start;
        while (curr) {
            if (curr->jid == job) {
                //Citation: https://chatgpt.com/share/68367ad1-b9cc-800f-9cd8-36747b025a83 
                //Citation: https://stackoverflow.com/questions/5341220/how-do-i-get-tcsetpgrp-to-work-in-c
                tcsetpgrp(STDIN_FILENO, curr->pid);
                //Citation: https://stackoverflow.com/questions/5785988/how-to-bring-a-child-process-running-in-the-background-to-the-foreground
                kill(-curr->pid, SIGCONT); 
                int status;
                waitpid(-curr->pid, &status, WUNTRACED);
                tcsetpgrp(STDIN_FILENO, getpgrp());
                if (curr->jid <= jid1 && WIFSTOPPED(status)) {
                    printf("\n[%d]        suspended       %s \n", curr->jid, curr->cmdline);
                    curr->state = 1;
                    curr->wifstop = 0;
                } else {
                    printf("[%d]          Completed       %s \n", curr->jid, curr->cmdline);
                    remove_job(curr->pid);
                    break;
                }
            }
            curr = curr->next;
        }
        excode = 0;
        return;
    }

    if (strstr(buffer, "bg %") == buffer) {
        strncpy(oldbuffer, buffer, MAX_CMD_BUFFER);
        int job = atoi(buffer + 4);
        struct job_t *curr = start;
        while (curr) {
            if (curr->jid == job) {
                curr->wifstop = 0;
                printf("[%d+]        running         %s \n", curr->jid, curr->cmdline);
                kill(-curr->pid, SIGCONT);
                curr->state = 1;
                break;
            }
            curr = curr->next;
        }
        excode = 0;
        return;
    }

    if (strcmp(buffer, "help") == 0) {
        printf("Welcome to Dave's icsh!\n\n"
            "Controls and Commands:\n\n"
            "1. Run commands normally by typing them and pressing Enter.\n"
            "2. Use & at the end of a command to run it in the background.\n"
            "3. Use jobs to list all background and suspended jobs.\n"
            "4. Use fg <job_id> to bring a background or suspended job to the foreground.\n"
            "5. Use bg <job_id> to resume a suspended job in the background.\n"
            "6. Press Ctrl+C to terminate the current foreground job.\n"
            "7. Press Ctrl+Z to suspend the current foreground job.\n"
            "8. Use !! to repeat the last command.\n"
            "9. Use < and > to redirect input and output of commands.\n"
            "10. Use exit to quit the shell.\n\n"
            "Enjoy!\n");
        excode = 0;
        return;
    }

    //Citation: https://github.com/yichuns/Shell-Lab/blob/master/tsh.c
    char jobbuffer[CMDLINE_LEN];
    strncpy(jobbuffer, buffer, CMDLINE_LEN - 1);
    jobbuffer[CMDLINE_LEN - 1] = '\0';

    int status;
    pid_t pid = fork();
    char delimiter[] = " ";
    char *token;
    char *tokens[64];
    int i = 0;
    int background_pid = 0;

    token = strtok(buffer, delimiter);
    while (token != NULL && i < 63) {
        if (strcmp(token, "&") == 0) {
            background_pid = 1;  
        } else {
            tokens[i++] = token;  
        }
        token = strtok(NULL, delimiter);
    }
    tokens[i] = NULL;

    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        //Citation: https://stackoverflow.com/questions/5341220/how-do-i-get-tcsetpgrp-to-work-in-c
        //Citation: https://emersion.fr/blog/2019/job-control/
        pid_t cpid = getpid();
        setpgid(0, 0);
        if (!background_pid) {
            tcsetpgrp(STDIN_FILENO, cpid);
        }
        execvp(tokens[0], tokens);
        fprintf(stderr, "Bad command\n");
        exit(1);
    } else {
        //Citation: https://stackoverflow.com/questions/5341220/how-do-i-get-tcsetpgrp-to-work-in-c
        //Citation: https://emersion.fr/blog/2019/job-control/
        setpgid(pid, pid);  
        if (!background_pid) {
            tcsetpgrp(STDIN_FILENO, pid);
            foreground_pid = pid;
            fg = 1;  
            waitpid(-pid, &status, WUNTRACED);
            fg = 0;  
            foreground_pid = -1;
            tcsetpgrp(STDIN_FILENO, getpgrp());
            if (WIFSTOPPED(status)) {
                add_job(pid, 1, jobbuffer, 1);
                printf("\n[%d]        suspended       %s \n", jid1 - 1, jobbuffer);
            } else if (WIFSIGNALED(status)) {
                printf("\n");
            } else if (WIFEXITED(status)) {
                excode = WEXITSTATUS(status);
            } else {
                excode = 1;
            }
        } else {
            add_job(pid, 1, jobbuffer, 0);
        }
    }
}
void redir(char* buffer){
    if (strstr(buffer, ">") != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
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
        excode = 0;
    }

    if (strstr(buffer, "<") != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
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
        if (file_desc < 0) {
            printf("No file\n");
        }
        else {
            dup2(file_desc, 0);
            close(file_desc);
            actions(cmd, buffer);
            dup2(saved_stdin, 0);
            close(saved_stdin);
            excode = 0;
        }
    }
}

int main(int argc, char *argv[]) {
    char buffer[MAX_CMD_BUFFER];
    char oldbuffer[MAX_CMD_BUFFER];
    oldbuffer[0] = '\0';

    sigchld_set();
    sigint_set();
    sigtstp_set();
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    
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

        if (strstr(buffer, ">") != NULL) {
            strncpy(oldbuffer, buffer, MAX_CMD_BUFFER);
            buffer[strcspn(buffer, "\n")] = '\0';
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
            excode = 0;
            continue;
        }

        if (strstr(buffer, "<") != NULL) {
            strncpy(oldbuffer, buffer, MAX_CMD_BUFFER);
            buffer[strcspn(buffer, "\n")] = '\0';
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
            if (file_desc < 0) {
                printf("No file\n");
            }
            else {
                dup2(file_desc, 0);
                close(file_desc);
                actions(cmd, buffer);
                dup2(saved_stdin, 0);
                close(saved_stdin);
                excode = 0;
            }
            continue;
        }

        actions(buffer, oldbuffer);
    }
    while (start != NULL) {
        remove_job(start->pid);
    }
    return excode;
}

 
 
 