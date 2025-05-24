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
 #include <unistd.h>
 
 #define MAX_CMD_BUFFER 255

 void handle1(int signum) {
    printf("\n");
 }
 
 void handle2(int signum) {
    printf("\n");
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
         if (!newbuf) {
            return;
         }
         strncpy(newbuf, &buffer[5], strlen(buffer) - 5);
         newbuf[strlen(buffer) - 5] = '\0';
         printf("%s\n", newbuf);
         free(newbuf);
         strncpy(oldbuffer, buffer, MAX_CMD_BUFFER);
         return;
     }
     if (strstr(buffer, ">") != NULL){
        int i = 0;
        char delimiter[] = ">";
        char* tokens[64]; 
        char* token1;
        token1 = strtok(buffer, delimiter);
        while (token1 != NULL && i < 63) {
            tokens[i++] = token1;
            token1 = strtok(NULL, delimiter);
        }
        tokens[i] = NULL;
        int j = 0;
        //Citation: https://www.geeksforgeeks.org/dup-dup2-linux-system-call/
        //Citation: https://www.reddit.com/r/learnprogramming/comments/4it2s3/would_someone_please_explainhelp_me_understand/
        int file_desc = open(tokens[1], O_TRUNC | O_CREAT | O_WRONLY, 0644);
        dup2(file_desc, 1);
        actions(tokens[0], buffer); 
        close(file_desc);
     }
     
     if (strstr(buffer, ".") != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
        //Citation: https://www.w3schools.com/c/c_files.php
        FILE *f = fopen(buffer, "r");
        if (f == NULL) {
            printf("file error");
            return;
        }
        //Citation: https://stackoverflow.com/questions/9206091/going-through-a-text-file-line-by-line-in-c
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            printf("%s", line);
            actions(line, oldbuffer);
        }
     }

     if (strstr(buffer, "exit ") == buffer) {
         char *newbuf = malloc(strlen(buffer) + 1);
         if (!newbuf) {
             perror("malloc failed");
             return;
         }
         //Citation: https://stackoverflow.com/questions/5881328/how-do-i-truncate-a-java-integer-to-fit-exand-to-a-given-number-of-bytes
         strncpy(newbuf, &buffer[5], strlen(buffer) - 1);
         newbuf[strlen(buffer) - 1] = '\0';
         int result = atoi(newbuf);
         free(newbuf);
         if (result < 0 || result > 255){
            result = result & 0xFF;
         }
         printf("Exit code: %d\n", result);
         printf("Bye\n");
         exit(result);
     }
     

     //Citation: https://stackoverflow.com/questions/5237482/how-do-i-execute-an-external-program-within-c-code-in-linux-with-arguments 
     //Citation: https://chatgpt.com/c/68302f25-4d0c-8001-babd-a44db93f01ee
     //Citation for process creation: https://iies.in/blog/understanding-fork-exec-and-process-creation-in-linux/
     int status;
     pid_t pid = fork();
     char delimiter[] = " ";
     char* token;
     char* tokens[64]; 
     int i = 0;

     token = strtok(buffer, delimiter);

     while (token != NULL && i < 63) {
        tokens[i++] = token;
        token = strtok(NULL, delimiter);
     }
     tokens[i] = NULL;
     //Citation: https://github.com/wenshuailu/shell_with_history/blob/master/shell.c (Lines 460 onwards)
     if (pid < 0) return;
     if(pid == 0){ 
        sigint_set();
        sigtstp_set();
        //Added error handling: https://chatgpt.com/share/6830639d-e504-8001-b61f-1e5366b0533e 
        execvp(tokens[0], tokens);
        perror("execvp failed");
        exit(1);
     } else {
        //Citation for WUNTRACED: https://stackoverflow.com/questions/33508997/waitpid-wnohang-wuntraced-how-do-i-use-these
        waitpid(pid, NULL, WUNTRACED);
     }
 
     strncpy(oldbuffer, buffer, MAX_CMD_BUFFER);
 }
 
 int main(int argc, char *argv[]) {
     char buffer[MAX_CMD_BUFFER];
     char oldbuffer[MAX_CMD_BUFFER];
     oldbuffer[0] = '\0';
     
     sigint_set();
     sigtstp_set();
 
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
 
 
 