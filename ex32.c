// elad baal-tzdaka 312531973
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_BUFF_SIZE 10000
#define MAX_LINE_SIZE 1000
#define NO_C_FILE 0
#define COMPILATION_ERROR 10
#define TIMEOUT 20
#define WRONG 50
#define SIMILAR 75
#define EXCELLENT 100
// errors
#define DIRECTORY_ERROR "Not a valid directory\n"
#define OUTPUT_ERROR "Output file not exist\n"
#define INPUT_ERROR "Input file not exist\n"


int findCFile(char *filename, char *mainPath, char *cFilePath);

int getGrade(char *path, char *outPutPath, int inputFd);

int compileFile(char *path);

int runMoreThen5Sec(int inputFd);

int comp(char *outputPath);

void writeResults(char *name, int grade, int fd);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return -1;
    }
    int configuration = open(argv[1], O_RDONLY);
    if (configuration == -1) {
        perror("Error in: open");
        exit(-1);
    }
    char buffer[MAX_BUFF_SIZE];
    if (read(configuration, buffer, MAX_BUFF_SIZE - 1) == -1) {
        perror("Error in: read");
        exit(-1);
    }
    //first line, path to a directory
    char *token = strtok(buffer, "\n");
    char dirPath[MAX_LINE_SIZE] = "";
    strcpy(dirPath, token);

    //second line, input path
    token = strtok(NULL, "\n");
    char inputPath[MAX_LINE_SIZE] = "";
    strcpy(inputPath, token);

    //third line, output path
    token = strtok(NULL, "\n");
    char outputPath[MAX_LINE_SIZE] = "";
    strcpy(outputPath, token);

    struct stat stat_dir;
    if ((stat(dirPath, &stat_dir) != -1 && !S_ISDIR(stat_dir.st_mode))|| (access(dirPath, F_OK) == -1)) {
        write(2, DIRECTORY_ERROR, sizeof(DIRECTORY_ERROR));
        return -1;
    }
    if ((access(inputPath, F_OK) == -1)) {
        close(configuration);
        perror(INPUT_ERROR);
        return -1;
    }
    if ((access(outputPath, F_OK) == -1)) {
        close(configuration);
        perror(INPUT_ERROR);
        return -1;
    }


    // i\o redirection
    int errorsFd = open("errors.txt", O_WRONLY | O_CREAT, 0666);
    if (errorsFd == -1) {
        perror("Error in: open");
        exit(-1);
    }
    if (dup2(errorsFd, 2) == -1) {
        perror("Error in: dup2");
        exit(-1);
    }

    int inputFd = open(inputPath, O_CREAT | O_RDONLY, 0666);
    if (inputFd == -1) {
        perror("Error in: open");
        close(errorsFd);
        exit(-1);
    }
    if (dup2(inputFd, 0) == -1) {
        perror("Error in: dup2");
        close(inputFd);
        close(errorsFd);
        exit(-1);
    }

    int resultsFd = open("results.csv", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (resultsFd == -1) {
        perror("Error in: open");
        close(errorsFd);
        close(inputFd);
        exit(-1);
    }

    // open directory
    DIR *folder = opendir(dirPath);
    if (folder == NULL) {
        perror("Error in: open");
        close(errorsFd);
        close(inputFd);
        close(resultsFd);
        exit(-1);
    }
    //scan dir
    struct dirent *dit;
    while ((dit = readdir(folder)) != NULL) {
        // files inside main directory
        char *fileName = dit->d_name;
        if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0) {
            continue;
        }
        if (dit->d_type == DT_DIR) {
            char cFilePath[MAX_LINE_SIZE] = "";
            if (findCFile(fileName, dirPath, cFilePath) == -1) {
                continue;
            }
            int grade = getGrade(cFilePath, outputPath, inputFd);
            if (grade == -1) {
                continue;
            }
            writeResults(fileName, grade, resultsFd);
        }
    }
    closedir(folder);
    close(inputFd);
    close(errorsFd);
    close(resultsFd);
    remove("temp.out");
    remove("output.txt");

}

void writeResults(char *name, int grade, int fd) {
    char line[MAX_LINE_SIZE] = "";
    strcat(line, name);
    strcat(line, ",");
    switch (grade) {
        case NO_C_FILE:
            strcat(line, "0,NO_C_FILE");
            break;
        case COMPILATION_ERROR:
            strcat(line, "10,COMPILATION_ERROR");
            break;
        case TIMEOUT:
            strcat(line, "20,TIMEOUT");
            break;
        case WRONG:
            strcat(line, "50,WRONG");
            break;
        case SIMILAR:
            strcat(line, "75,SIMILAR");
            break;
        case EXCELLENT:
            strcat(line, "100,EXCELLENT");
            break;
        default:
            break;
    }
    strcat(line, "\n");
    write(fd, line, strlen(line));


}

int getGrade(char *path, char *outputPath, int inputFd) {
    if (strcmp(path, "") == 0) {
        return 0;
    }
    int result = compileFile(path);
    if (result == 0) {
        return 10;
    }
    result = runMoreThen5Sec(inputFd);
    if (result == 1) {
        return 20;
    }
    result = comp(outputPath);
    if (result == 1) {
        return 100;
    }
    if (result == 2) {
        return 50;
    }
    if (result == 3) {
        return 75;
    }
    return -1;
}

int comp(char *outputPath) {
    char *args[] = {"./comp.out", "output.txt", outputPath, NULL};
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error in: fork");
        return -1;
    }
    int status;

    // child try to run
    if (pid == 0) {
        execvp("./comp.out", args);
        //if execvp failed
        perror("Error in: execvp");
        return -1;
    }

    // father wait for child
    waitpid(pid, &status, 0);
    //if child terminated normally
    if (WIFEXITED(status)) {
        // return the exit status of the child
        return WEXITSTATUS(status);
    }
    return -1;
}

//if return  1 - the exe file run more then 5 sec
//if return  0 - the exe file run <= 5 sec
int runMoreThen5Sec(int inputFd) {
    // new output
    int outputFd = open("output.txt", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (outputFd == -1) {
        perror("Error in: open");
        return -1;
    }
    if (dup2(outputFd, 1) == -1) {
        perror("Error in: dup2");
        return -1;
    }

    if (lseek(inputFd, 0, SEEK_SET) == -1) {
        perror("Error in: lseek");
        return -1;
    }

    char *args[] = {"./temp.out", NULL};
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error in: fork");
        return -1;
    }
    int status;

    // child try to run
    if (pid == 0) {
        alarm(5);
        execvp("./temp.out", args);
        perror("Error in: execvp");
        return -1;
    }

    // father wait for child
    waitpid(pid, &status, 0);
    //if child process ends with signal not handled
    if (WIFSIGNALED(status)) {
        // if the signal received was sigalrm
        if (WTERMSIG(status) == SIGALRM) {
            close(outputFd);
            return 1;
        }
    }
    close(outputFd);
    return 0;
}

//if return -1 - system failed
//if return  0 - compilation failed
//if return  1 - success
int compileFile(char *path) {
    char *args[] = {"gcc", path, "-o", "temp.out", NULL};
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error in: fork");
        return -1;
    }
    int status;

    // child try to compile
    if (pid == 0) {
        execvp("gcc", args);
    }

    // father wait for child
    waitpid(pid, &status, 0);
    //if child terminated normally
    if (WIFEXITED(status)) {
        // return the exit status of the child
        if (WEXITSTATUS(status)) {
            //printf("Exit status of the child was %d\n",exit_status);
            return 0;
        } else {
            return 1;
        }
    }
    return -1;
}

int findCFile(char *fileName, char *mainPath, char *cFilePath) {
    // create user path
    char userPath[MAX_LINE_SIZE] = "";
    strcat(userPath, mainPath);
    strcat(userPath, "/");
    strcat(userPath, fileName);

    DIR *user = opendir(userPath);
    if (user == NULL) {
        return -1;
    }
    struct dirent *userIt;
    while ((userIt = readdir(user)) != NULL) {
        char *suffix = strrchr(userIt->d_name, '.');
        if (suffix == NULL) {
            continue;
        }
        if (strcmp(suffix, ".c") == 0) {
            strcat(userPath, "/");
            strcat(userPath, userIt->d_name);
            strcpy(cFilePath, userPath);
            closedir(user);
            return 1;
        }
    }
    closedir(user);
    return 1;
}