/* ICCS227: Project 1: icsh
 * Name: Devya Shah
 * StudentID: 6480253
 */

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 
 #define MAX_CMD_BUFFER 255
 
 void helper(char *buffer) {
     char *newbuf = malloc(strlen(buffer) - 5 + 1);
     if (!newbuf) {
        return;
     }
     strncpy(newbuf, &buffer[5], strlen(buffer) - 5);
     newbuf[strlen(buffer) - 5] = '\0';
     printf("%s\n", newbuf);
     free(newbuf);
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
         helper(buffer);
         strncpy(oldbuffer, buffer, MAX_CMD_BUFFER);
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
         if (result < 0 || result > 255){
            result = 0;
         }
         printf("Exit code: %d\n", result);
         printf("Bye\n");
         exit(result);
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
 
 
 