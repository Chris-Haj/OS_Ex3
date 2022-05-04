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
#define NOHUP "nohup"
#define CD_LENGTH strlen(CD)
#define EXIT_LENGTH strlen(EXIT)
#define HISTORY_LENGTH strlen(HISTORY)
#define PATH_LENGTH 100

void loop();
void checkInput(FILE *file, char *input, size_t i, int fromHistory);
void counter(const char *line, size_t i, int background);
void readHistory(FILE *file);
int cmdFromHistory(char *line, const int linesNum[], int size, int max, const int pipeIndexes[], const int beginningOfCmd[]);
void cmdSplitter(const char *line, int pipeAmount, int *wordsAmount, const int *pipeIndexes, int background);
void executeOneCmd(char **cmd[], int *words, int background);
void executeTwoCmds(char **cmd[], int *words, int background);
void executeThreeCmds(char **cmd[], int *words, int background);
void freeCommands(char **cmd[], const int *words, int commandsAmnt);
int fromHistoryLineToCmd(char *line, size_t i);
void handler(int sig){
    int stat=1;
    while(stat!=-1&&stat!=0)
        stat = waitpid(-1,&sig,WNOHANG);
}

int numberOfCommands = 1;
int totalNumberOfWords = 0;
int running = 1;

int main() {
    loop();
    return 0;
}

int fromHistoryLineToCmd(char *line, size_t i) {
    /*
     * this function is used to check if there are !x commands which are stored in the history file,
     * a loop is used to go over the array to split the input into commands depending on how many pipes exist
     * ,while separating the commands, if the command is of type !x then the number x is saved into historyLines array
     * into the fitting index depending on if it is the first/second/third command, if the command is in word format then its index
     * in historyLines is equal -1.
     */
    int historyLines[] = {-1, -1, -1};
    int beginningOfCmd[] = {0, 0, 0};
    int indexes[] = {-1, -1};
    int max = -1, cur = 0;
    while (line[i] != '\n') {
        if (line[i] == '!') {
            i++;
            historyLines[cur] = atoi(&line[i]);
            if (historyLines[cur] < 1) {
                fprintf(stderr, "File starts from line number 1!\n");
                return 1;
            }

            max = max > historyLines[cur] ? max : historyLines[cur];
        } else if (line[i] == '|') {
            indexes[cur] = (int) (i + 1);
            cur++;
            int c = (int) (i + 1);
            while (line[c++] != ' ');
            beginningOfCmd[cur] = c;
        }
        i++;
    }
    //If all commands were in normal word format then return 0 and do nothing to the input and continue normal execution
    if (historyLines[0] == -1 && historyLines[1] == -1 && historyLines[2] == -1)
        return 0;
    //If we found more than 2 pipes than return 1 meaning input is not valid for execution.
    else if (cur > 2)
        return 1;
    else {
        if (cmdFromHistory(line, historyLines, cur + 1, max, indexes, beginningOfCmd) == 0)
            return 0;
        else
            return 1;
    }
}

/*
 * Function used to search for the specific command in the line number entered next to !
 * and execute it if the number entered is less or equal to than the total number of lines in the file
 */
int cmdFromHistory(char *line, const int linesNum[], const int size, const int max, const int pipeIndexes[], const int beginningOfCmd[]) {
    FILE *file = fopen(FILENAME, "r");
    char command[size][LENGTH];
    for (int j = 0; j < size; j++) //empty all strings
        strcpy(command[j], "\0");
    char curLine[LENGTH];
    /*
     * Any command that was originally in the initial input in words format it which is indicated at by a -1 in the linesNum array index
     * is copied into the matching index into the command array*/
    for (int c = 0; c < size; c++) {
        if (linesNum[c] == -1) {
            if (pipeIndexes[c] != -1 && c < 2) {
                strncpy(command[c], &line[beginningOfCmd[c]], pipeIndexes[c] - beginningOfCmd[c] - 1);
                command[c][pipeIndexes[c] - beginningOfCmd[c] - 1] = '\0';
            } else {
                strcpy(command[c], &line[beginningOfCmd[c]]);
            }
        }
    }
    int cur = 1;
    //pass over the lines from the history file and copying the lines we need into the commands array
    while (fgets(curLine, LENGTH, file)) {
        if (cur == linesNum[0]) {
            strcpy(command[0], curLine);
            command[0][strlen(command[0]) - 1] = '\0';
        } else if (cur == linesNum[1]) {
            strcpy(command[1], curLine);
            command[1][strlen(command[1]) - 1] = '\0';
        } else if (cur == linesNum[2]) {
            strcpy(command[2], curLine);
            command[2][strlen(command[2]) - 1] = '\0';
        }
        cur++;
        if (cur > max)
            break;
    }
    fclose(file);
    //If cur does not reach max then that means one or more of the commands' number (!x) did not yet exist in the file
    if (cur < max) {
        fprintf(stderr, "One of the numbers entered does not exist in the file yet\n");
        return 1;
    }
    strcpy(curLine, "");
    printf("%s\n", command[0]);
    printf("%s\n", command[1]);
    //After reformatting the commands from !x to words they are combined again to be sent as a normal command to parse and execute
    if (size == 1) {
        sprintf(line, "%s\n", command[0]);
    } else if (size == 2) {
        sprintf(line, "%s|%s\n", command[0], command[1]);
    } else {
        sprintf(line, "%s|%s|%s\n", command[0], command[1], command[2]);
    }
    int counter = 0;
    /*In case one of the past commands had a pipe in it and the total amount of pipes reached is more than 2 then return 1
     * ,meaning command is not valid for execution.
     */
    for (int c = 0; line[c] != '\n'; c++)
        if (line[c] == '|')
            counter++;
    if (counter > 2)
        return 1;
    else
        return 0;
}

void cmdSplitter(const char *line, int pipeAmount, int *wordsAmount, const int *pipeIndexes, int background) {
    int i = 0;
    int start = 0, end, index = 0;
    int cmdAmount = pipeAmount + 1;
    char **commands[cmdAmount];
    //Each array is allocated memory according to how many words each command has while also making the last pointer of each array point to NULL
    for (; i < cmdAmount; i++) {
        commands[i] = (char **) calloc(wordsAmount[i] + 1, sizeof(char **));
        commands[i][wordsAmount[i]] = NULL;
    }
    i = 0;
    /*Each array inside the commands[] array is allocated memory to copy each word of the current command in a separate index and after
     * reaching the end of a command which we know by finding a | then the next command is copied into the next array's indexes*/
    for (int c = 0; c < cmdAmount; c++) {
        for (; line[i] != '\n' && line[i] != '|'; i++) {
            if (line[i] == ' ' || line[i + 1] == '\n' || line[i + 1] == '|') {
                end = i;
                if (start > end)
                    end = i = start;
                if (line[i + 1] == '\n' || (line[i + 1] == '|' && line[i] != ' '))
                    end = i + 1;
                int CurWordSize = (end - start);
                if (CurWordSize != 0) {
                    commands[c][index] = (char *) calloc(CurWordSize + 1, sizeof(char));
                    if (!commands[c][index]) {
                        fprintf(stderr, "Error allocating memory!\n");
                    }
                    strncpy(commands[c][index], &line[start], end - start);
                    index++;
                }
                start = end;
                while (line[start] == ' ')
                    start++;
                if (line[start] == '\n' || line[start] == '|') {
                    break;
                }
            }
        }
        if (c < pipeAmount)
            i = pipeIndexes[c] + 1;
        while (line[i] == ' ')
            i++;
        start = i;
        index = 0;
    }
    if (cmdAmount == 1)
        executeOneCmd(commands, wordsAmount, background);
    else if (cmdAmount == 2) {
        executeTwoCmds(commands, wordsAmount, background);
    } else
        executeThreeCmds(commands, wordsAmount, background);
    freeCommands(commands, wordsAmount, cmdAmount);
}

void freeCommands(char **cmd[], const int *words, int commandsAmnt) {
    for (int i = 0; i < commandsAmnt; i++) {
        for (int j = 0; j < words[i]; j++) {
            if (!cmd[i][j])
                free(cmd[i][j]);
        }
        if (!cmd[i])
            free(cmd[i]);
    }
}

void executeOneCmd(char **cmd[], int *words, int background) {
    int status;
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failure");
        freeCommands(cmd, words, 2);
        exit(1);
    } else if (pid == 0) {
        if (-1 == execvp(*(cmd[0]), cmd[0])) {
            perror("command not found");
            freeCommands(cmd, words, 1);
            exit(1);
        }
    }
    //If the initial input was received normally with nohup and no ampersand then the father is told to wait the child
    if (background == 0)
        wait(&status);
}

void executeTwoCmds(char **cmd[], int *words, int background) {
    int fd[2];
    int status;
    if (pipe(fd) == -1) {
        perror("pipe failure");
        freeCommands(cmd, words, 2);
        exit(1);
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failure");
        close(fd[0]); /*--*/ close(fd[1]);
        freeCommands(cmd, words, 2);
        exit(1);
    } else if (pid == 0) {
        if (dup2(fd[1], 1) == -1) {
            perror("dup failure");
            close(fd[0]); /*--*/ close(fd[1]);
            freeCommands(cmd, words, 2);
            exit(1);
        }
        if (close(fd[0]) == -1 || close(fd[1])) {
            perror("close failure");
            freeCommands(cmd, words, 2);
            exit(1);
        }
        if (execvp(*cmd[0], cmd[0]) == -1) {
            perror("command not found");
            freeCommands(cmd, words, 2);
            exit(1);
        }
        exit(0);
    } else {
        pid = fork();
        if (pid == -1) {
            perror("fork failure");
            close(fd[0]); /*--*/ close(fd[1]);
            freeCommands(cmd, words, 2);
            exit(1);
        } else if (pid == 0) {
            if (dup2(fd[0], 0) == -1) {
                perror("dup failure");
                close(fd[0]); /*--*/ close(fd[1]);
                freeCommands(cmd, words, 2);
                exit(1);
            }
            if (close(fd[0]) == -1 || close(fd[1])) {
                perror("close failure");
                freeCommands(cmd, words, 2);
                exit(1);
            }
            if (execvp(*cmd[1], cmd[1]) == -1) {
                perror("command not found");
                freeCommands(cmd, words, 2);
                exit(1);
            }
            exit(0);
        }
    }
    if (close(fd[0]) == -1 || close(fd[1])) {
        perror("close failure");
        freeCommands(cmd, words, 2);
        exit(1);
    }
    //If the initial input was received normally with nohup and no ampersand then the father is told to wait for the two children processes
    if(background==0){
        for(int i=0;i<2;i++)
            wait(&status);
    }
}

void executeThreeCmds(char **cmd[], int *words, int background) {
    int fd[4];
    int status;
    /*Array fd is used to open two pipes at index [0,1] and [2,3] which are used later for all three children created via fork
     * to pass in the input and output of each command from one to the next */
    if (pipe(fd) == -1 || pipe(fd + 2) == -1) {
        perror("pipe failure");
        exit(1);
    }
    /*fork is used 3 times to create 3 children processes, for each child to run one command while give its output as input
     * for the next child's command.
     * Each child also uses the dup2 command to open a connection via the pipe, which are then used as a way to receive their in/output to others.*/
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork() error");
        close(fd[0]); /*--*/ close(fd[1]); /*--*/ close(fd[2]);/*--*/close(fd[3]);
        freeCommands(cmd, words, 2);
        exit(1);
    } else if (pid == 0) {
        if (dup2(fd[1], STDOUT_FILENO) == -1) {
            close(fd[0]); /*--*/ close(fd[1]); /*--*/ close(fd[2]);/*--*/close(fd[3]);
            perror("dup failure");
            freeCommands(cmd, words, 3);
            exit(1);
        }
        if (close(fd[0]) == -1 || close(fd[1]) == -1 || close(fd[2]) == -1 || close(fd[3]) == -1) {
            perror("close failure");
            freeCommands(cmd, words, 3);
            exit(1);
        }
        if (execvp(*cmd[0], cmd[0]) == -1) {
            perror("command not found");
            exit(1);
        }
        exit(0);
    } else {
        pid = fork();
        if (pid == -1) {
            perror("fork() error");
            close(fd[0]); /*--*/ close(fd[1]); /*--*/ close(fd[2]);/*--*/close(fd[3]);
            exit(1);
        } else if (pid == 0) {
            if (dup2(fd[0], STDIN_FILENO) == -1 || dup2(fd[3], STDOUT_FILENO) == -1) {
                perror("dup failure");
                close(fd[0]); /*--*/ close(fd[1]); /*--*/ close(fd[2]);/*--*/close(fd[3]);
                exit(1);
            }
            if (close(fd[0]) == -1 || close(fd[1]) == -1 || close(fd[2]) == -1 || close(fd[3]) == -1) {
                perror("close failure");
                freeCommands(cmd, words, 3);
                exit(1);
            }
            if (execvp(*cmd[1], cmd[1]) == -1) {
                perror("command not found");
                exit(1);
            }
            exit(0);
        } else {
            pid = fork();
            if (pid == -1) {
                perror("fork() error");
                freeCommands(cmd, words, 3);
                close(fd[0]); /*--*/ close(fd[1]); /*--*/ close(fd[2]);/*--*/close(fd[3]);
                exit(1);
            } else if (pid == 0) {
                if (dup2(fd[2], STDIN_FILENO) == -1) {
                    close(fd[0]); /*--*/ close(fd[1]); /*--*/ close(fd[2]);/*--*/close(fd[3]);
                    perror("close failure");
                    freeCommands(cmd, words, 3);
                    exit(1);
                }
                if (close(fd[0]) == -1 || close(fd[1]) == -1 || close(fd[2]) == -1 || close(fd[3]) == -1) {
                    perror("close failure");
                    freeCommands(cmd, words, 3);
                    exit(1);
                }
                if (execvp(*cmd[2], cmd[2]) == -1)
                    perror("command not found");
                exit(0);
            }
        }
    }
    if (close(fd[0]) == -1 || close(fd[1]) == -1 || close(fd[2]) == -1 || close(fd[3]) == -1) {
        perror("close failure");
        freeCommands(cmd, words, 3);
        exit(1);
    }
    //If the initial input was received normally with nohup and no ampersand then the father is told to wait for all three children processes
    if(background==0){
        for(int i=0;i<3;i++)
            wait(&status);
    }
}

/*
 * Main loop function to keep asking user for input and call other functions according to what is passed into the stdin stream
 * while also checking if the input passed is valid
 * */
void loop() {
    signal(SIGCHLD,handler);
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
        fclose(file);
        if (fromHistoryLineToCmd(input, i) == 1) {
            fprintf(stderr, "Error in input!\n");
            continue;
        }
        fopen(FILENAME, "a+");
        checkInput(file, input, i, 0);
    }
}

/*
 * checkInput is used to check what kind of command/input was passed through.
 * If one of the commands (done/history/cd) were entered with no other words then the program either terminates/prints out all previously entered commands, or a cd error
 * If the input wasn't any of said commands, then the program will treat the input as a shell command and create an array of pointers of returned size from counter+1 and this array
 * is sent to the executeCommands function.
 * Update: if nohup was entered at the beginning of the input then the command continues to run after the program terminates,
 * and if an ampersand '&' was entered at the ending of a command then the command will run in the background.
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
        counter(input, i, 0);
        return;
    } else if (input[strlen(input) - 2] == '&') {
        fromHistory == 0 ? fprintf(file, "%s", input) : fprintf(file, "%s\n", input);
        //add the input received into the history file then remove the ampersand from input and any spaces before it
        size_t j = strlen(input)-3;
        while(input[j]==' ' && j>=0)
            j--;
        if(j==-1){
            fprintf(stderr,"Input with only an ampersand is not valid!\n");
            return;
        }
        input[j+1]='\n';
        input[j+2]='\0';
        counter(input, i, 1);
    }
    else if(strncmp(input,NOHUP,strlen(NOHUP))==0){
        fromHistory == 0 ? fprintf(file, "%s", input) : fprintf(file, "%s\n", input);
        //add the input received into the history file then remove the nohup from input and any spaces after it
        size_t j = strlen(NOHUP);
        while(input[j]==' ') {
            j++;
        }
        if(input[j]=='\n'){
            fprintf(stderr,"nohup with no command is not valid input!\n");
            return;
        }
        strcpy(input,&input[j]);
        counter(input,i,2);
    }
    else {
        counter(input, i, 0);
        fromHistory == 0 ? fprintf(file, "%s", input) : fprintf(file, "%s\n", input);
    }
}

/* the counter function is used to count how many words are in the input and add it to the total
 * number of words entered and incrementing total number of commands by 1.
 */
void counter(const char *line, size_t i, int background) {
    int wordsAmount[3] = {1, 1, 1};
    int pipeIdx[2] = {-1, -1};
    int curPipe = 0;
    int j = 0;
    /*
     * this loop is used to calculate how many words each command seperated by a pipe '|' and also counts how many pipes
     * are in the input while also saving the index of each pipe */
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
    for (int c = 0; c < curPipe + 1; c++){
        numberOfCommands++;
        totalNumberOfWords += wordsAmount[c];
    }
    cmdSplitter(line, curPipe, wordsAmount, pipeIdx, background);
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
        fprintf(stderr, "Error receiving file from function\n");
}