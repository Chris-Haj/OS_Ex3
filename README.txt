Mini Shell Simulation
Authored by Christopher Haj
207824772

==Description==
This program is used to simulate a mini shell that is built through two files, named ex2a.c and ex2b.c.
The C file ex2a.c is used to simulate some commands of the linux shell and passes out the same output as the actual shell, whereas the ex2b.c file is built
on-top of the first file to add another functionality of the shell to use a command that was already passed in through the use of !x, where x can be any number from 1 upto
the number of commands used through the run of the program.


Program DATABASES:
A text file by the name file.txt is used to store all commands entered by user (excluding !x commands).
Another text file named nohup.txt which stores the out of any command that starts with the words nohup

==Functions==
1) loop: contains the main loop which asks user for input to process.
2) checkInput: this function checks what kind of input was entered and deals with it according to what kind it is (done, history, cd, other...),.
   if it is (other) then the input is sent into the wordCounter and creates an array of pointers of the return size of wordCounter+1, and is sent into executeCommand(...)
3) counter: this function checks how many words each command contains, commands are separated by pipes '|'
   this function counts and returns the amount of words contained in the input.
4) executeCommand: this function places each word of the input in order into the array received from checkInput and sends the array into the execvp(...) built-in function
   to execute the command.
5) readHistory: this function reads and outputs the contents of the file.txt in a numbered fashion.
6) cmdFromHistory: this function is only used through an input of type !x which, executes the command in line x of file.txt if x is between [1,NumberOfLines]
7) cmdSplitter: this function is called to from the counter(...) function and it receives the original input, with the amount of commands, and the amount
    of words in each command. It creates a 3D array of chars of the same size as the amount of commands, then it fills up each index with a single command
    which is stored as single words in each index of the 2D array then proceeds to send this array to one of three commands, executeOne/Two/ThreeCmds
    after finishing their execution it calls to freeCommands to free any dynamically allocated memory.
8) cmdFromHistory: this functions checks if any of the commands in the original input is a history command, if yes then it stores the number of line
    the command in an array which holds the number of line of a command in the fitting index depending on if it is the first, second or third command.
9) fromHistoryLineToCmd: this functions is called to from cmdFromHistory then it proceeds to split the input, with each command on its own, if a command is in normal word format
    then it is kept as it is, else if it's in number format (!x) then it's searched for in the history file then saved in a char array, after turning all commands
    into normal format, they are merged again and sent to counter, to process and execute its' commands.
==Program Files==
ex3.c

==How to compile?==
compile: gcc ex3.c -o ex3

==How to run?==
./ex3


==Input==
1) "history"
2) "cd"
3) "done"
4) Input is required to be some basic commands of the linux shell, some examples are (ls, echo, cal, mkdir, touch), pipes can also be implemented (ex: ls | sort)
    and simulated which makes the output of the left command input to the right command. (Program can currently simulate up to 2 pipes)
5) Another kind of input that is allowed is !x to use a command that was written before in the current run.
6) If a command was entered with an ampersand at the end, the command will run in the background instead of foreground.
7) If nohup was entered followed by a command then the command will continue to process after the program finishes execution.

==Restricted inputs==
1) Input of only spaces.
2) Input of nothing.
3) Input where the first or last indexes are a space.
4) The command cd (will be available in a later program)
5) !x where x is a number greater current amount of lines in file.txt


==Output==
There can be multiple kinds of output depending on which commands are passed in.
1) Through the input "history", the contents of file.txt are printed out.
2) If cd was entered, then it simply outputs an error "command not supported yet".
3) If done was entered, the program prints out how many commands were used and the total amount of words in all inputs were (excluding !x commands)
4) If a shell command was entered, such as (ls, echo, cal), the same output as the linux shell will be output.
   Examples for (4):
   a- ls will print the contents of the current working directory.
   b- echo ... will write to the terminal all words entered after it ("echo hello" would print "hello").
   c- cal will print a calendar of the current month.
5) If pipes were used then output of command will be input of next command
    Example:
            ls|sort will print what ls normally prints but sorted.
