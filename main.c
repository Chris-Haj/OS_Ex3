#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define LENGTH 514
#define FILENAME "file.txt"
#define HISTORY "history"
#define EXIT "done"
#define CD "cd"
#define CD_LENGTH strlen(CD)
#define EXIT_LENGTH strlen(EXIT)
#define HISTORY_LENGTH strlen(HISTORY)


#define PATH_LENGTH 100

void loop();

void checkInput(FILE *file, char *input, size_t i, int fromHistory);

void counter(const char *line, size_t i);

void executeCommands(char *argv[], char *line, size_t size);

void readHistory(FILE *file);

void cmdFromHistory(char *line);

int numberOfCommands = 1;
int numberOfPipes=0;
int totalNumberOfWords = 0;
int running = 1;

int main() {
//    loop();

    char *test[]={"basd","-l",NULL};
    int commands=1;
    int fd[2];
    int num=0;
    pipe(fd);
    pid_t pid=fork();
    if(pid==0){
        close(fd[0]);
        int random= execvp(*test,test);
        dup2(fd[1],1);
        printf("%d",random);
        close(fd[1]);
        exit(0);
    }else{
        if(commands >1){
            pid = fork();
            if(pid == 0) {
                close(fd[1]);
                dup2(fd[0], 0);
                int failed;
                scanf("%d", &failed);
                if (failed == -1) {
                    printf("cmd not found!\n");
                    exit(1);
                }
                close(fd[0]);
            }



        }
        else{
            close(fd[1]);
            dup2(fd[0],0);
            int failed;
            scanf("%d",&failed);
            if(failed == -1)
                printf("cmd not found!\n");
            close(fd[0]);
        }
    }

    return 0;
}

//Main loop function to keep asking user for input and call other functions according to what is passed into the stdin stream
void loop() {
    char location[PATH_LENGTH];
    const char *const PATH = getcwd(location, PATH_LENGTH);
    FILE *file = fopen(FILENAME, "a+");
    if (file == NULL)
        fprintf(stderr, "Error trying to open file!\n");
    char input[LENGTH] = "";
    while (running) {
        printf("%s> ", PATH);
        fgets(input, LENGTH, stdin);
        fflush(stdin);
        size_t i = 0;
        while (input[i] == ' ') i++;
        if (input[i] == '\0' || input[i] == '\n') {
            fprintf(stderr, "Please enter a command!\n"
                            "Empty input or input of only spaces is not allowed!\n");
            continue;
        }
        int lastChar = (int) strlen(input) - 2;
        if (!(i == 0 && lastChar >= 0 && input[lastChar] != ' ')) {
            fprintf(stderr, "Spaces before or after a command is not allowed!\n");
            continue;
        }
        if (input[0] == '!') {
            int IsANumber = 1;
            for (int j = 1; input[j] != '\n'; j++) {
                if (!('0' <= input[j] && input[j] <= '9'))
                    IsANumber = 0;
            }
            if (IsANumber == 0) {
                fprintf(stderr, "Please enter only numbers after the ! to execute a past command\n");
            } else {
                fclose(file);
                cmdFromHistory(&input[1]);
                file = fopen(FILENAME, "a+");
            }
            continue;
        }
        checkInput(file, input, i, 0);
    }
}

/*
 * checkInput is used to check what kind of command/input was passed through.
 * If one of the commands (done/history/cd) were entered with no other words then the program either terminates/prints out all previously entered commands, or a cd error
 * If the input wasn't any of said commands, then the program will treat the input as a shell command and create an array of pointers of returned size from counter+1 and this array
 * is sent to the executeCommands function.
*/
void checkInput(FILE *file, char *input, size_t i, int fromHistory) {
    if (strncmp(input, EXIT, EXIT_LENGTH) == 0 && input[EXIT_LENGTH] == '\n') {
        fclose(file);
        printf("Num of commands: %d\n", numberOfCommands);
        printf("Total number of words in all commands: %d!\n", totalNumberOfWords);
        running = 0;
        return;
    } else if (strncmp(input, HISTORY, HISTORY_LENGTH) == 0 && input[HISTORY_LENGTH] == '\n') {
        fromHistory == 0 ? fprintf(file, "%s", input) : fprintf(file, "%s\n", input);
        readHistory(file);
        numberOfCommands++, totalNumberOfWords++;
        return;
    } else if (strncmp(&input[i], CD, CD_LENGTH) == 0 && (input[CD_LENGTH] == ' ' || input[CD_LENGTH] == '\n')) {
        fprintf(stderr, "Command not supported yet!\n");
        counter(input, i);
        return;
    } else {
        counter(input, i);
    }
    fromHistory == 0 ? fprintf(file, "%s", input) : fprintf(file, "%s\n", input);
}

/*
 * the counter function is used to count how many words are in the input and add it to the total
 * number of words entered and incrementing total number of commands by 1.
 */

void counter(const char *line, size_t i) {
    int wordsAmount[3] = {1,1,1};
    int pipeIdx[2] = {-1,-1};
    int curPipe = 0;
    int j=0;
    while (line[i] != '\n') {
        if (line[i] == '|'){
            pipeIdx[curPipe++] = (int) i;
            j++;
            while (line[i + 1] == ' ')
                i++;
        }
        else if (line[i] == ' ') {
            while (line[i + 1] == ' ')
                i++;
            if(line[i+1]!='|' && line[i+1]!='\n')
                wordsAmount[j]++;
        }
        i++;
    }
    numberOfPipes+=curPipe;
    numberOfCommands++;
    for(int c = 0;c<curPipe+1;c++)
        totalNumberOfWords += wordsAmount[c];



}

/*
 * Function used to search for the specific command in the line number entered next to !
 * and execute it if the number entered is less or equal to than the total number of lines in the file
 */
void cmdFromHistory(char *line) {
    FILE *file = fopen(FILENAME, "r");
    char command[LENGTH];
    int lineNumber = (int) strtol(line, NULL, 10);
    int cur = 0;
    while (cur < lineNumber && fgets(command, LENGTH, file)) {
        cur++;
    }
    if (cur < lineNumber) {
        fprintf(stderr, "Number of line does not exist yet!\n");
        return;
    }
    printf("%s", command);
    checkInput(file, command, 0, 1);
    fclose(file);
}


//the readHistory function is simple function used to reopen the file in read mode and pass through all lines in the file while printing them to the terminal.
void readHistory(FILE *file) {
    rewind(file);
    if (file != NULL) {
        char currentLine[LENGTH];
        int counter = 1;
        while (fgets(currentLine, LENGTH, file))
            printf("%d: %s", counter++, currentLine);
    } else
        fprintf(stderr, "Error receiving file from function");
}

/*After receiving the array of pointers, this function goes through the input again,
 * while placing each word in a separate index
*/
void executeCommands(char *argv[], char *line, size_t size) {
    int start = 0, end = 0, index = 0;
    for (int i = 0; line[i] != '\n'; i++) {
        if (start > end)
            end = i = start;
        if (line[i] == ' ' || line[i + 1] == '\n') {
            end = i;
            if (line[i + 1] == '\n')
                end++;
            int CurWordSize = (end - start) + 1;
            argv[index] = (char *) calloc(CurWordSize, sizeof(char));
            if (!argv[index]) {
                fprintf(stderr, "Error allocating memory!\n");
            }
            strncpy(argv[index], &line[start], end - start);
            start = end + 1;
            while (line[start] == ' ') start++;
            if (line[start] == '\n')
                break;
            index++;
        }
    }
/*
* after setting up the argv array the fork function is called and the array is sent to the execvp function in the child program, which processes the array as
* if the input was entered in a linux shell
*/
    pid_t child = fork();
    if (child < 0) {
        perror("fork error");
    }
    if (child == 0) {
        if (execvp(argv[0], argv) == -1) {
            perror("execvp error");
        }
        exit(1);
    }
    wait(NULL);
    for (int i = 0; i < size; i++)
        free(argv[i]);
}

void sendToPipe(char *in[], char *out[]) {
    pid_t pid;
    int fd[2];

    pipe(fd);
    pid = fork();

    if (pid == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(in[0], in);
        exit(1);
    } else {
        pid = fork();

        if (pid == 0) {
            dup2(fd[0], STDIN_FILENO);
            close(fd[1]);
            close(fd[0]);
            execvp(out[0], out);
            exit(1);
        } else {
            int status;
            close(fd[0]);
            close(fd[1]);
            waitpid(pid, &status, 0);
        }
    }
}
