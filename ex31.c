// elad baal-tzdaka 312531973
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int checkIfEquals(int file1, int file2);

int checkIfSimilar(int file1, int file2);

int main(int argc, char *argv[]) {
    //not enough parameter
    if (argc != 3) {
        exit(-1);
    }
    //open files
    int file1 = open(argv[1], O_RDONLY);
    if (file1 == -1) {
        perror("Error in: open");
        exit(-1);
    }
    int file2 = open(argv[2], O_RDONLY);
    if (file2 == -1) {
        close(file1);
        perror("Error in: open");
        exit(-1);

    }

    int result = checkIfEquals(file1, file2);
    if (result == 1) {
        close(file1);
        close(file2);
        return 1;
    }
    if (result == 0) {
        if(lseek(file1, -1, SEEK_CUR)==-1)
        {
            perror("Error in: lseek");
            close(file1);
            close(file2);
            exit(-1);
        }
        if(lseek(file2, -1, SEEK_CUR)==-1){
            perror("Error in: lseek");
            close(file1);
            close(file2);
            exit(-1);
        }
    }
    result = checkIfSimilar(file1, file2);
    if (result == 1) {
        close(file1);
        close(file2);
        return 3;
    }
    close(file1);
    close(file2);
    return 2;
}

int checkIfSimilar(int file1, int file2) {
    char buff1;
    char buff2;
    // read first char of file1
    int bytes_read1 = read(file1, &buff1, 1);
    if (bytes_read1 == -1) {
        perror("Error in: read");
        exit(-1);
    }
    // ignore spaces
    while ((buff1 > 126 || buff1 < 33) && (bytes_read1 != 0)) {
        bytes_read1 = read(file1, &buff1, 1);
        if (bytes_read1 == -1) {
            perror("Error in: read");
            exit(-1);
        }
    }
    buff1 = toupper(buff1);
    // read first char of file2
    int bytes_read2 = read(file2, &buff2, 1);
    if (bytes_read2 == -1) {
        perror("Error in: read");
        exit(-1);
    }
    while ((buff2 > 126 || buff2 < 33) && bytes_read2 != 0) {
        bytes_read2 = read(file2, &buff2, 1);
        if (bytes_read2 == -1) {
            perror("Error in: read");
            exit(-1);
        }
    }
    buff2 = toupper(buff2);

    while ((bytes_read1 != 0) && (bytes_read2 != 0) && (buff1 == buff2)) {
        // read char of file1
        bytes_read1 = read(file1, &buff1, 1);
        if (bytes_read1 == -1) {
            perror("Error in: read");
            exit(-1);
        }
        while ((buff1 > 126 || buff1 < 33) && bytes_read1 != 0) {
            bytes_read1 = read(file1, &buff1, 1);
            if (bytes_read1 == -1) {
                perror("Error in: read");
                exit(-1);
            }
        }
        buff1 = toupper(buff1);
        // read char of file2
        bytes_read2 = read(file2, &buff2, 1);
        if (bytes_read2 == -1) {
            perror("Error in: read");
            exit(-1);
        }
        while ((buff2 > 126 || buff2 < 33) && bytes_read2 != 0) {
            bytes_read2 = read(file2, &buff2, 1);
            if (bytes_read2 == -1) {
                perror("Error in: read");
                exit(-1);
            }
        }
        buff2 = toupper(buff2);
    }
    if ((bytes_read1 == 0) && (bytes_read2 == 0)) {
        return 1;
    }
    return 0;
}

//return 2 - length are different
//return 1 - equals
//return 0 not equals
int checkIfEquals(int file1, int file2) {
    int lengthFile1 = lseek(file1, 0, SEEK_END);
    if (lengthFile1 == -1) {
        perror("Error in: lseek");
        exit(-1);
    }
    int lengthFile2 = lseek(file2, 0, SEEK_END);
    if (lengthFile2 == -1) {
        perror("Error in: lseek");
        exit(-1);
    }
    //reset offset
    if (lseek(file1, 0, SEEK_SET) == -1) {
        perror("Error in: lseek");
        exit(-1);
    }
    if (lseek(file2, 0, SEEK_SET) == -1) {
        perror("Error in: lseek");
        exit(-1);
    }
    if (lengthFile1 != lengthFile2) {
        return 2;
    }

    int length = lengthFile1;
    char buff1[2] = "";
    char buff2[2] = "";
    int bytes_read1 = read(file1, buff1, 1);
    if (bytes_read1 == -1) {
        perror("Error in: read");
        exit(-1);
    }
    int bytes_read2 = read(file2, buff2, 1);
    if (bytes_read2 == -1) {
        perror("Error in: read");
        exit(-1);
    }
    while ((length > 0) && (strcmp(buff1, buff2) == 0)) {
         bytes_read1 = read(file1, buff1, 1);
        if (bytes_read1 == -1) {
            perror("Error in: read");
            exit(-1);
        }
        bytes_read2 = read(file2, buff2, 1);
        if (bytes_read2 == -1) {
            perror("Error in: read");
            exit(-1);
        }
        length--;
    }
    if (length == 0) {
        return 1;
    }
    return 0;
}
