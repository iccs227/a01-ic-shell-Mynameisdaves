/* ICCS227: Project 1: icsh
 * Name: Devya Shah
 * StudentID: 6480253
 */

 #include <stdio.h>
 #include <string.h>
 #include <unistd.h>
 #include <sys/wait.h>
 #include <stdlib.h>
 
 #define MAX_CMD_BUFFER 255
 
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
     int status;
     int pid;

     if(fork() == 0){ 
        status = system(buffer);
        printf("icsh $ ");
        exit(0);
     }else{
        waitpid (pid, NULL, 0);
        
     }
 
     strncpy(oldbuffer, buffer, MAX_CMD_BUFFER);
 }
 
 int main(int argc, char *argv[]) {
     char buffer[MAX_CMD_BUFFER];
     char oldbuffer[MAX_CMD_BUFFER];
     oldbuffer[0] = '\0';
 
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
 
 
 