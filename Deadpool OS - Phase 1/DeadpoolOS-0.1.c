#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>

#define BUFFER_SIZE 100
#define USER "Guest"
#define OS "Deadpool"
#define VERSION "0.1"
#define COMMAND_SIZE 100
#define NB_OF_COMMANDS 12
#define NB_OF_OPTIONS 2
#define AUTHORS "Jehad Oumer, Heba Rachid, Sharif Fahes"
#define YEAR "2020"

char *deadpoolCommands[NB_OF_COMMANDS] = {"listdr", "printcdr", "createdr", "remove", "return", "create", "move", "clear", "compare", "read", "permission", "wordC"};
char *sysCommands[NB_OF_COMMANDS] = {"ls", "pwd", "mkdir", "rm", "echo", "touch", "mv", "clear", "diff", "cat", "chmod", "wc"};
char *deadpoolOptions[NB_OF_OPTIONS] = {"-all", "-F"};
char *sysOptions[NB_OF_OPTIONS] = {"-l", "-rf"};

void onePipe(char **, char **);
void twoPipe(char **, char **, char **);
int identifyCommand(char *);
int identifyOption(char *);

int main()
{

    printf(" ____                 _                   _    ___  ____\n");
    printf("|  _ \\  ___  __ _  __| |_ __   ___   ___ | |  / _ \\/ ___|\n");
    printf("| | | |/ _ \\/ _` |/ _` | '_ \\ / _ \\ / _ \\| | | | | \\___ \\\n");
    printf("| |_| |  __/ (_| | (_| | |_) | (_) | (_) | | | |_| |___) |\n");
    printf("|____/ \\___|\\__,_|\\__,_| .__/ \\___/ \\___/|_|  \\___/|____/\n");
    printf("                       |_|                  \n");

    printf("Welcome to the Beta version of Deadpool OS. This is the first version of the system, more updates will follow\nFeatures: supports tens of commands!, 2 one piped commands, and two 2 piped commands, and a fun easter egg\n\nfor more information about the commands supported use \"help\"\n");
    int *end = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    while (1)
    {
        int fd[2];
        pipe(fd);

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("Error forking child\n");
        }
        else if (pid == 0)
        {

            fflush(stdin);
            char buffer[BUFFER_SIZE];
            close(fd[0]);
            printf("\033[0;32m");
            printf("%s@%s ~: ", USER, OS);
            printf("\033[0m");
            fgets(buffer, sizeof(buffer), stdin);
            strtok(buffer, "\n");

            write(fd[1], buffer, strlen(buffer) + 1);

            close(fd[1]);
            exit(0);
        }
        else
        {
            wait(NULL);
            close(fd[1]);

            char buffer[BUFFER_SIZE];
            read(fd[0], buffer, BUFFER_SIZE);
            char *command[COMMAND_SIZE];
            char *token = strtok(buffer, " ");
            int i = 0;
            while (token != NULL)
            {

                command[i++] = token;
                token = strtok(NULL, " ");
            }
            command[i++] = NULL;
            ///recongize and excute alias command
            //printf("command: %s buffer:%s %d \n", command[0], buffer, (strcmp(command[0], "ld")));

            pid_t pid2 = fork();

            if (pid2 < 0)
            {
                perror("Error forking child2\n");
            }
            else if (pid2 == 0)
            {

                if (strcmp(command[0], "help") == 0)
                {
                    printf("\n%s OS Version %s offers the following commands:\n\t1) listdr : list the content of the directory, equivalent to ls and it uses the same set of options.\n\t2) printcdr : prints the current working directory, equivalent to pwd and it uses the same set of options.\n\t3) createdr : creates a new directory, equivalent to mkdir and it uses the same set of options.\n\t4) remove: removes file or directory, equivalent to rm and it uses the same set of options.(to remove directory use remove -rF\n\t5) return: returns a string to the shell, equivalent to echo and it uses the same set of options.\n\t6) create: creates a new file, equivalent to touch and it uses the same set of options.\n\t7) move: moves a file or directory to another directory, equivalent to mv and it uses the same set of options.\n\t8) clear: clears the content on the shell, equivalent to clear and it uses the same set of options.\n\t9) compare: compares the difference between two files, equivalent to diff and it uses the same set of options.\n\t10) help: shows the list of commands allowed in the OS and their description\n\t11) premission : modifies the access premissions of a file, equivalent to chmod and it uses the same set of options\n\t12) findAvenger: search inside avengers.txt file and locate a certain avenger by name or letter, example: findAvenger Hulk, findAvenger a ...\n\t13) end:terminate Deadpool OS\n\t14) countF: counts the number countF: count the number of files and directories in the directory\n\t15) details: get info about the OS.\n\t 16)listSorted: list the files and directories in a sorted manner\n\tSupported Options: -l and -rF", OS, VERSION);
                }
                if (strcmp(command[0], "details") == 0)
                {
                    printf("\n%s OS Version %s\nCSC326 Project - Phase 1\n\nLebenese American University\n%s, %s\n", OS, VERSION, AUTHORS, YEAR);
                }
                else if (strcmp(command[0], "countF") == 0)
                {
                    char *firstpart[5] = {"ls", NULL};
                    char *secondpart[10] = {"wc", "-l", NULL};
                    onePipe(firstpart, secondpart);
                }
                else if (strcmp(command[0], "findAvenger") == 0)
                {
                    char *firstpart[5] = {"cat", "avengers.txt", NULL};
                    char *secondpart[10] = {"grep", command[1], NULL};
                    onePipe(firstpart, secondpart);
                }
                else if (strcmp(command[0], "count") == 0)
                {
                    char *firstpart[5] = {"ls", NULL};
                    char *secondpart[10] = {"grep", command[1], NULL};
                    char *thirdpart[10] = {"wc", "-l", NULL};
                    twoPipe(firstpart, secondpart, thirdpart);
                }
                else if (strcmp(command[0], "listSorted") == 0)
                {
                    char *firstpart[5] = {"ls", NULL};
                    char *secondpart[10] = {"sort", NULL};
                    char *thirdpart[10] = {"less", NULL};
                    twoPipe(firstpart, secondpart, thirdpart);
                }
                else if (strcmp(command[0], "end") == 0)
                {
                    end[0] = 1;
                }
                else
                {
                    int value = identifyCommand(command[0]);

                    if (value == -1)
                        printf("%s command not found\n", command[0]);
                    else
                    {
                        command[0] = sysCommands[value];
                        if (i > 2)
                        {

                            int k = 1;
                            while (k < (i - 1) && command[k][0] == '-')
                            {
                                int v = identifyOption(command[k]);
                                if (v == -1)
                                {
                                    printf("%s invalid command\n", command[k]);
                                    exit(0);
                                }
                                else
                                    command[k] = sysOptions[v];
                                k++;
                            }
                        }
                        execvp(command[0], command);
                        // close(fd[0]);
                    }
                }
            }
            else
            {
                wait(NULL);

                //close(fd[0]);
            }
            if (end[0] == 1)
                break;
        }
        close(fd[0]);
        close(fd[1]);
    }

    return 0;
}

int identifyCommand(char *command)
{
    for (int i = 0; i < NB_OF_COMMANDS; i++)
    {
        if (strcmp(command, deadpoolCommands[i]) == 0)
        {
            return i;
        }
    }

    return -1;
}

int identifyOption(char *option)
{
    for (int i = 0; i < NB_OF_OPTIONS; i++)
        if (strcmp(option, deadpoolOptions[i]) == 0)
            return i;
    return -1;
}
void onePipe(char **command, char **command1)
{

    int fd[2];
    pipe(fd);
    pid_t child = fork();
    if (child == 0)
    {
        close(1);
        dup(fd[1]);
        close(fd[1]);
        close(fd[0]);
        execvp(command[0], command);
    }
    else
    {
        wait(NULL);
        close(0);
        dup(fd[0]);
        close(fd[0]);
        close(fd[1]);
        execvp(command1[0], command1);
    }
}

void twoPipe(char **command, char **command2, char **command3)
{
    int fd[2];
    int fd1[2];
    pipe(fd1);
    pipe(fd);
    pid_t child = fork();
    if (child == 0)
    {
        close(1);
        dup(fd[1]);

        close(fd1[1]);
        close(fd1[0]);
        close(fd[1]);
        close(fd[0]);
        execvp(command[0], command);
    }
    else
    {
        pid_t child1 = fork();
        if (child1 == 0)
        {
            close(0);
            dup(fd[0]);

            close(1);
            dup(fd1[1]);

            close(fd[1]);
            close(fd[0]);
            close(fd1[0]);
            close(fd1[1]);

            execvp(command2[0], command2);
        }
        else
        {

            close(0);
            dup(fd1[0]);

            close(fd[0]);
            close(fd[1]);
            close(fd1[0]);
            close(fd1[1]);
            execvp(command3[0], command3);
        }
        wait(NULL);
        close(fd[0]);
        close(fd[1]);
        close(fd1[0]);
        close(fd1[1]);
    }
}
