#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <wait.h>
#include <semaphore.h>
#include <signal.h>

#define SERV_PORT 8881
#define COMMAND_SIZE 100
#define BUFFER_SIZE 100
#define NB_OF_COMMANDS 16
#define NB_OF_OPTIONS 3
//#define USER "guest\0"
#define OS "Deadpool"
#define VERSION "0.3"
#define AUTHORS "Jehad Oumer, Heba Rachid, Sharif Fahes"
#define YEAR "2020"
#define BACKLOG 10
#define MAX 10

char *deadpoolCommands[NB_OF_COMMANDS] = {"listdr", "printcdr", "createdr", "remove", "return", "create", "move", "clear", "compare", "read", "permission", "wordC", "copy", "ipinfo", "psinfo", "find"};
char *sysCommands[NB_OF_COMMANDS] = {"ls", "pwd", "mkdir", "rm", "echo", "touch", "mv", "clear", "diff", "cat", "chmod", "wc", "cp", "ifconfig", "ps", "grep"};
char *deadpoolOptions[NB_OF_OPTIONS] = {"-all", "-F", "-details"};
char *sysOptions[NB_OF_OPTIONS] = {"-l", "-rf", "aux"};
int *flag;
int reqNum = 0;
int j = 0;
typedef struct
{
    char *com[BUFFER_SIZE];
    int id;
    int t;
    int client;
    int round;
} request;
int *clientSocket;
int in = 0;
int out = 0;
sem_t *turn;
typedef struct
{
    request queue[20];
    int runnedReq;
    int end;
    int curr;
    int client;

} threadQ;

threadQ threadQueue[BACKLOG];

request queue[50];
void schedule_thread(int);

void onePipe(char **, char **);
void twoPipe(char **, char **, char **);
int identifyCommand(char *);
int identifyOption(char *);
void *handle_client(void *);
int runCommand(char **command, int start, int end);
void runPipe(char **commands, int *pipeInd, int pipesNb);
void call(int[], int, char **, int);
char osLogo[] = " ____                 _                   _    ___  ____\n|  _ \\  ___  __ _  __| |_ __   ___   ___ | |  / _ \\/ ___|\n| | | |/ _ \\/ _` |/ _` | '_ \\ / _ \\ / _ \\| | | | | \\___ \\\n| |_| |  __/ (_| | (_| | |_) | (_) | (_) | | | |_| |___) |\n|____/ \\___|\\__,_|\\__,_| .__/ \\___/ \\___/|_|  \\___/|____/\n                       |_|                  \n";

char welcomeMessage[] = "***Server Version of the OS, Now it supports multiple clients!!!***\nWelcome to the Beta version of Deadpool OS. This is the third version of the system. More updates will follow.\n\nfor help use \"help\"\n";

typedef struct ClientS
{
    int id;
    int clientSocket;
    char *clientIP;
} clientS;

int main()
{
    //printf("%s %s server\nWaiting for connection....\n", OS, VERSION);
    int serverSocket = 0;
    clientSocket = malloc(sizeof(int) * BACKLOG);

    //sem to control thread scheduling
    turn = (sem_t *)malloc(sizeof(sem_t) * MAX);
    for (int i = 0; i < 3; i++)
        sem_init(&turn[i], 0, 0);
    flag = (int *)malloc(sizeof(int) * BACKLOG);
    flag[0] = 1;
    sem_post(&turn[0]);

    struct sockaddr_in address;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        printf("error server socket");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Server socket %d Initalized\n", SERV_PORT);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(SERV_PORT);

    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    int k = listen(serverSocket, BACKLOG);
    if (k < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Listening at port %d for incoming connection....\n", SERV_PORT);
    }

    int ClientCounter = 0;
    int i = 0;

    while (1)
    {
        int addrlen = sizeof(address);
        clientS client;

        clientSocket[i] = accept(serverSocket, (struct sockaddr *)&address, (socklen_t *)&addrlen);

        client.clientSocket = clientSocket[i++];

        if (client.clientSocket < 0)
        {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
        else
        {
            ClientCounter++;
            client.id = i;
            client.clientIP = inet_ntoa(address.sin_addr);
            printf("Connected with Client #%d %s.\n", client.id, client.clientIP);
            if (k >= 0)
                printf("Listening at port 8880 for incoming connection....\n");
        }
        if (fork() == 0)
        {
            pthread_t th;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            while (flag[i - 1] == 0)
                ;
            pthread_create(&th, &attr, handle_client, &client);
            threadQ thread;
            thread.client = clientSocket[i - 1];
            thread.runnedReq = 0;
            thread.end = 0;
            thread.curr = 0;
            threadQueue[in++] = thread;
        }

        printf("I am here %d\n", in);
    }

    close(serverSocket);
    return 0;
}
void schedule_thread(int id)
{
    //not working!!!!!!!!!!!!!!!!!!
    int max = 0;
    int ind = 0;
    for (int i = 0; i < 2; i++)
    {
        // if(threadQueue[i].end-threadQueue[i].curr>max){
        //     ind=i;max=threadQueue[i].end-threadQueue[i].curr;
        // }
        if (id - 1 != i)
        {
            ind = i;
            break;
        }
    }
    printf("ind:%d\n", ind);

    if (id - 1 != ind)
        threadQueue[in++] = threadQueue[out++];
    flag[ind] = 1;
    //sem_post(&turn[ind]);
}

void *handle_client(void *c)
{

    clientS *client = (clientS *)c;
    int s = (int)client->clientSocket;
    char *clientIP = (char *)client->clientIP;
    int clientID = (int)client->id;
    char clientIDString[2];
    sprintf(clientIDString, "%d", clientID);
    char USER[7] = "guest";

    strcat(USER, clientIDString);

    char t[10];
    int n = 0;
    while (flag[clientID - 1] == 0)
        ;
    flag[clientID - 1] = 0;

    n = strlen(osLogo);
    write(s, &n, sizeof(int));
    send(s, osLogo, n, 0);
    n = strlen(welcomeMessage);
    write(s, &n, sizeof(int));
    send(s, welcomeMessage, n, 0);
    while (1)
    {
        printf("waiting %d\n", clientID - 1);
        // sem_wait(&turn[clientID-1]);

        printf("inside thread %d\n", clientID);
        int fd[2];
        pipe(fd);
        char buffer[BUFFER_SIZE] = {0};
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("Error forking child\n");
        }
        else if (pid == 0)
        {
            close(fd[0]);
            n = strlen(USER);
            write(s, &n, sizeof(int));
            send(s, USER, n, 0);
            n = strlen(OS);
            write(s, &n, sizeof(int));
            send(s, OS, n, 0);
            read(s, &n, sizeof(int));
            recv(s, buffer, n, 0);
            printf("Client #%d %s, Recieved: %s\n", clientID, clientIP, buffer);

            write(fd[1], buffer, sizeof(buffer));

            close(fd[1]);
            exit(EXIT_SUCCESS);
        }
        else
        {
            wait(NULL);
            close(fd[1]);

            char buffer[BUFFER_SIZE];
            read(fd[0], buffer, BUFFER_SIZE);
            if (strcmp(buffer, "disconnect") == 0)
            {

                printf("Client #%d %s disconnected\n", clientID, clientIP);
                close(s);
                //
                break;
            }
            //printf("Recieved: %s\n", buffer);
            char *command[COMMAND_SIZE] = {0};
            char bufferCopied[200];
            strcpy(bufferCopied, buffer);
            char *token = strtok(buffer, " | ");

            int pipeInd[10];
            int pipeNb = 0;
            for (int i = 0; i < 10; i++)
                pipeInd[i] = -1;
            int i = 0, j = 0;
            int ind = 0;
            while (token != NULL)
            {
                if (i > 0 && bufferCopied[ind] == '|')
                {
                    command[i] = NULL;
                    pipeInd[j++] = i++;
                    pipeNb++;
                }
                command[i++] = token;
                ind += strlen(command[i - 1]) + 1;
                token = strtok(NULL, " | ");
            }

            command[i] = NULL;
            pipeInd[j] = i;

            request r;
            if (strcmp(command[0], "program") == 0)
                r.t = 6; //not working!!!!!!!!!!!!!!!!!!!!!!!
            else
                r.t = 1;

            r.id = reqNum++;
            r.client = s;
            threadQueue[out].queue[threadQueue[out].end++] = r;
            printf("%d", threadQueue[out].queue[threadQueue[out].end - 1].t);
            call(pipeInd, pipeNb, command, i);
        }
        threadQueue[out].runnedReq++;
        threadQueue[clientID - 1].round += 1;
        schedule_thread(clientID);
    }
}
void call(int pipeInd[], int pipeNb, char *command[], int i)
{

    printf("call entered\n");
    char bufferFile[4000] = {0};
    int fd2[2];
    pipe(fd2);

    int s = threadQueue[out].client;
    // int s=clientSocket[0];
    printf("%d %d\n", out, s);
    while (threadQueue[out].end > (threadQueue[out].curr))
    {
        printf("%d-%d", threadQueue[out].end, threadQueue[out].curr);
        if (threadQueue[out].queue[threadQueue[out].curr].t != 1)
        {
            //not working!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            printf("here\n");
            sleep(2);
            threadQueue[out].queue[threadQueue[out].curr].t -= 2;
            printf("time:%d\n", threadQueue[out].queue[threadQueue[out].curr].t);
            //if program not done insert it to end of queue
            if (queue[threadQueue[out].curr].t > 0)
                queue[threadQueue[out].end++] = threadQueue[out].queue[threadQueue[out].curr];
            else
            {
                char *msg = "program done\n";
                int x = strlen(msg);

                write(s, &x, sizeof(int));
                write(s, msg, x);
            }
        }
        else
        {
            pid_t pid2 = fork();

            if (pid2 < 0)
            {
                perror("Error forking child2\n");
            }
            else if (pid2 == 0)
            {
                close(fd2[0]);
                dup2(fd2[1], 1);
                dup2(fd2[1], 2);

                if (pipeNb > 0)
                {
                    runPipe(command, pipeInd, pipeNb);
                }
                else if (strcmp(command[0], "help") == 0)
                {
                    //sleep(10);
                    printf("\n%s OS Version %s offers the following commands:\n\t1) listdr : list the content of the directory, equivalent to ls and it uses the same set of options.\n\t2) printcdr : prints the current working directory, equivalent to pwd and it uses the same set of options.\n\t3) createdr : creates a new directory, equivalent to mkdir and it uses the same set of options.\n\t4) remove: removes file or directory, =rm, Usage: remove file, remove -F directory\n\t5) return: returns a string to the shell, =echo, Usage: return text.\n\t6) create: creates a new file, =touch, Usage: create filename\n\t7) move: moves a file or directory to another directory, =mv , Usage: move file dir.\n\t8) clear: clears the content on the shell, =clear\n\t9) compare: compares the difference between two files, equivalent to diff, Usage: compare file1 file2\n\t10) help: shows the list of commands allowed in the OS and their description\n\t11) premission : modifies the access premissions of a file, equivalent to chmod and it uses the same set of options\n\t12) findAvenger: search inside avengers.txt file and locate a certain avenger by name or letter, example: findAvenger Hulk, findAvenger a ...\n\t13) disconnect: kills connection with the connected client\n\t14) countF: counts the number of files and directories in the directory\n\t15)count ...:count the number of files with specified name, =ls | grep key | wc -l, Usage: count key\n\t16) details: get info about the OS.\n\t17)listSorted: list the files and directories in a sorted manner, =ls | sort | less\n\t18)copy : creates a copy of a file or directory, =cp,  Usage: copy file1 file2\n\t19)ipinfo : prints network interfaces information, =ifconfig\n\t20)read:display content of file =cat\n\t21)wordC: count the frequency of word = wc\n\t22)psinfo: print information about current running processes, =ps.\n\n\tSupported Options: -all listSorted and -F for remove, -details for psinfo\n", OS, VERSION);

                    close(fd2[1]);
                    exit(0);
                }
                if (strcmp(command[0], "details") == 0)
                {

                    printf("\n%s OS Version %s\nCSC326 Project - Phase 3\n\nLebenese American University\n%s, %s\n", OS, VERSION, AUTHORS, YEAR);

                    close(fd2[1]);
                    exit(0);
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
                else if (command[0][0] == '.' && command[0][1] == '/')
                {
                    execlp(command[0], command[0], NULL);
                }
                else
                {
                    int value = identifyCommand(command[0]);

                    if (value == -1)
                    {
                        printf("%s command not found\n", command[0]);

                        close(fd2[1]);
                        exit(0);
                    }
                    else
                    {
                        int value = runCommand(command, 0, i - 1);
                        if (value == -1)
                        {
                            printf("%s command not found\n", command[0]);

                            close(fd2[1]);
                            exit(0);
                        }
                        else if (value == -2)
                        {
                            printf("invalid option for command %s\n", command[0]);

                            close(fd2[1]);
                            exit(0);
                        }
                        else
                        {
                            execvp(command[0], command);
                        }
                    }
                }
            }
            else
            {
                wait(NULL);
                //close(1);
                int n = 0;
                close(fd2[1]);
                printf("here\n");
                read(fd2[0], bufferFile, sizeof(bufferFile));
                n = sizeof(bufferFile);
                write(s, &n, sizeof(int));
                send(s, bufferFile, n, 0);
                //recv(s, t, strlen(t), 0);
                close(fd2[0]);
            }
        }
        threadQueue[out].curr++;
    }
}

int runCommand(char **command, int start, int end)
{

    int value = identifyCommand(command[start]);

    int x = 0;
    if (value == -1)
    {

        return -1;
    }
    else
    {
        command[start] = sysCommands[value];

        int k = start + 1;

        while (k <= end)
        {

            // if(command[k][0] == '\''){k++; continue;}
            if (command[k][0] == '-')
            {
                int v = identifyOption(command[k]);
                if (v == -1)
                {

                    return -2;
                }
                else
                    command[k] = sysOptions[v];
            }
            k++;
            x = k;
        }
    }

    return x;
}

void runPipe(char **commands, int *pipeInd, int pipesNb)
{
    int i = 0;
    int fds[2 * pipesNb];
    int *temp = (int *)mmap(NULL, sizeof(int) * 2, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    temp[0] = 0;

    while (i < pipesNb)
    {
        if (pipe(2 * i + fds) < 0)
            perror("Failed");
        i++;
    }

    i = 0;
    while (pipeInd[i] != -1)
    {

        pid_t pid = fork(); // fork child to execute command

        if (pid == 0)
        {
            if (i != 0)
            {
                if (dup2(fds[2 * (i - 1)], 0) < 0)
                    perror("Failed");
            }
            if (pipeInd[i + 1] != -1)
            {
                if (dup2(fds[2 * i + 1], 1) < 0)
                    perror("Failed");
            }

            for (int j = 0; j < 2 * pipesNb; j++)
                close(fds[j]);

            char *command[50];
            runCommand(commands, temp[0], pipeInd[i] - 1);
            for (int j = temp[0]; j <= pipeInd[i]; j++)
                command[j - temp[0]] = commands[j];
            temp[0] = pipeInd[i] + 1;
            execvp(command[0], command);
        }
        else if (pid < 0)
            perror("error");
        i++;
    }
    for (int j = 0; j < 2 * pipesNb; j++)
    {
        close(fds[j]);
    }

    for (i = 0; i < pipesNb + 1; i++)
        wait(NULL);
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
