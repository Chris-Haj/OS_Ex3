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

void readHistory(FILE *file);

void cmdFromHistory(char *line);

void cmdSplitter(const char *line, int pipeAmount, int *wordsAmount, const int *pipeIndexes);

void executeOneCmd(char *cmd[]);

void executeTwoCmds(char *cmd[], char *cmd2[]);

void executeThreeCmds(char *cmd[], char *cmd2[], char *cmd3[]);

int numberOfCommands = 1;
int numberOfPipes = 0;
int totalNumberOfWords = 0;
int running = 1;

int main() {
    loop();
    return 0;

}

void cmdSplitter(const char *line, int pipeAmount, int *wordsAmount, const int *pipeIndexes) {
    int i = 0;
    int start = 0, end , index = 0;
    int cmdAmount = pipeAmount + 1;
    char **commands[cmdAmount];
    for (; i < cmdAmount; i++) {
        commands[i] = (char **) calloc(wordsAmount[i] + 1, sizeof(char **));
        commands[i][wordsAmount[i]] = NULL;
    }
    i = 0;
    for (int c = 0; c < cmdAmount; c++) {
        for (; line[i] != '\n' && line[i] != '|'; i++) {
            if (line[i] == ' ' || line[i + 1] == '\n' || line[i+1] == '|') {
                end = i;
                if (start > end)
                    end = i = start;
                if (line[i + 1] == '\n'||(line[i+1]=='|'&&line[i]!=' '))
                    end=i+1;
                int CurWordSize = (end - start);
                if(CurWordSize!=0){
                    commands[c][index] = (char *) calloc(CurWordSize+1, sizeof(char));
                    if (!commands[c][index]) {
                        fprintf(stderr, "Error allocating memory!\n");
                    }
                    strncpy(commands[c][index], &line[start], end - start);
//                    printf("%s letter amount is %d, mem is %d index is %d array is %d\n",commands[c][index],end-start,CurWordSize+1,index,c);
                    index++;
                }
                start = end ;
                while (line[start] == ' ') start++;

                if (line[start] == '\n' || line[start] == '|') {
                    break;
                }
            }
        }
        if (c < pipeAmount)
            i = pipeIndexes[c]+1;
        while(line[i]== ' ')
            i++;
        start=i;
        index = 0;
    }
    if (cmdAmount == 1)
        executeOneCmd(commands[0]);
    else if (cmdAmount == 2) {
        executeTwoCmds(commands[0], commands[1]);
    }
    else
        executeThreeCmds(commands[0], commands[1], commands[2]);
    for (i = 0; i < cmdAmount; i++) {
        for (int j = 0; j < wordsAmount[i]; j++) {
            free(commands[i][j]);
        }
        free(commands[i]);
    }
}

void executeOneCmd(char *cmd[]) {
    int status;
    pid_t pid = fork();
    if (pid == 0) {
        if (-1 == execvp(*cmd, cmd))
            perror("command unknown");
    }
    wait(&status);
}

void executeTwoCmds(char *cmd[], char *cmd2[]) {
    int fd[2];
    pipe(fd);
    pid_t pid = fork();
    int status;
    if (pid == 0) {
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        execvp(*cmd, cmd);
        exit(0);

    } else {
        pid = fork();
        if (pid == 0) {
            close(fd[1]);
            dup2(fd[0], 0);
            execvp(*cmd2, cmd2);
            close(fd[1]);
            exit(0);
        }
    }
    close(fd[0]);
    close(fd[1]);
    wait(&status);
    wait(&status);
}

void executeThreeCmds(char *cmd[], char *cmd2[], char *cmd3[]) {
    int fd[4];
    int status;
    pipe(fd);
    pipe(fd + 2);
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork() error");
        close(fd[0]);
        close(fd[1]);
        close(fd[2]);
        close(fd[3]);
        exit(1);
    } else if (pid == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        close(fd[2]);
        close(fd[3]);
        execvp(*cmd, cmd);
        exit(0);
    } else {
        pid = fork();
        if (pid == -1) {
            perror("fork() error");
            close(fd[0]);
            close(fd[1]);
            close(fd[2]);
            close(fd[3]);
            exit(1);
        } else if (pid == 0) {
            dup2(fd[0], 0);
            dup2(fd[3], 1);
            close(fd[0]);
            close(fd[1]);
            close(fd[2]);
            close(fd[3]);
            execvp(*cmd2, cmd2);
            exit(0);
        } else {
            pid = fork();
            if (pid == -1) {
                perror("fork() error");
                close(fd[0]);
                close(fd[1]);
                close(fd[2]);
                close(fd[3]);
                exit(1);
            } else if (pid == 0) {
                dup2(fd[2], 0);
                close(fd[0]);
                close(fd[1]);
                close(fd[2]);
                close(fd[3]);
                execvp(*cmd3, cmd3);
            }
        }
    }
    close(fd[0]);
    close(fd[1]);
    close(fd[2]);
    close(fd[3]);
    for (int i = 0; i < 3; i++)
        wait(&status);
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
    int wordsAmount[3] = {1, 1, 1};
    int pipeIdx[2] = {-1, -1};
    int curPipe = 0;
    int j = 0;
    while (line[i] != '\n') {
        if (line[i] == '|') {
            pipeIdx[curPipe++] = (int) i;
            j++;
            while (line[i + 1] == ' ')
                i++;
        } else if (line[i] == ' ') {
            while (line[i + 1] == ' ')
                i++;
            if (line[i + 1] != '|' && line[i + 1] != '\n')
                wordsAmount[j]++;
        }
        i++;
    }
//    printf("%d %d %d",wordsAmount[0],wordsAmount[1],wordsAmount[2]);
    numberOfPipes += curPipe;
    numberOfCommands++;
    for (int c = 0; c < curPipe + 1; c++)
        totalNumberOfWords += wordsAmount[c];
    cmdSplitter(line, curPipe, wordsAmount, pipeIdx);
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

/*
* after setting up the argv array the fork function is called and the array is sent to the execvp function in the child program, which processes the array as
* if the input was entered in a linux shell
*/


/*
 * receive char array
 * create array of the same size as the amount of words
 * put each word in an index
*/
