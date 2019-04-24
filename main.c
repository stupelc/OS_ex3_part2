//Chagit Stupel 209089960

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <memory.h>
#include <wait.h>
#include <stdbool.h>

#define SIZE 150
#define PRINT_ERROR fprintf (stderr,"Error in system call\n");

//grades of students
//no c file - gets 0
#define NO_C_FILE_GRADE 0
#define NO_C_FILE_STR "NO_C_FILE"
// compilation error - gets 20
#define COMPILATION_ERROR_GRADE 20
#define COMPILATION_ERROR_STR "COMPILATION_ERROR"
//timeOut problem - gets 40
#define TIMEOUT_GRADE 40
#define TIMEOUT_STR "TIMEOUT"
//bad outpuut grade - gets 60
#define BAD_OUTPUT_GRADE 60
#define BAD_OUTPUT_STR "BAD_OUTPUT"
// similar output - gets 80
#define SIMILAR_OUTPUT_GRADE 80
#define SIMILAR_OUTPUT_STR "SIMILAR_OUTPUT"
// identical output - gets 100
#define GREAT_JOB_GRADE 100
#define GREAT_JOB_STR "GREAT_JOB"

/**
 * struct of the configuration file
 */
typedef struct conf_file {
    char path[SIZE];
    char input[SIZE];
    char output[SIZE];
} conFile;

/**
 * struct of the student grade
 */
typedef struct studentGrade {
    char name[SIZE];
    char gradeString[SIZE];
    int grade;
} studentGrade;

void compareFilesAndUpadate(conFile *configuration, studentGrade *sg, char path[SIZE]);

void runProgram(conFile *configuration, studentGrade *sg, char path[SIZE]);

void setConfiguration(char *path, conFile *configuration);

void checkOutputNum(int num);

void checkNULL(char* num);

int moveDirectories(conFile *configuration, studentGrade *sg, char directory[SIZE]);

int isCompile(char path[], struct dirent *rDirc);

void saveStudent(struct studentGrade *sg);

int main(int argc, char *argv[]) {
    //creates the configuration file
    conFile configuration = {};

    //set the configuration of the file
    setConfiguration(argv[1], &configuration);

    DIR *mainDir = opendir(configuration.path);
    if (mainDir == NULL) {
        PRINT_ERROR
        exit(EXIT_FAILURE);
    }
    struct dirent *mainDirent = readdir(mainDir);
    if (mainDirent == NULL && errno != 0) {
        PRINT_ERROR
        exit(EXIT_FAILURE);
    }

    //start running on the directories
    while (mainDirent != NULL) {
        if (!strcmp(mainDirent->d_name, "..") || !strcmp(mainDirent->d_name, ".")) {
            mainDirent = readdir(mainDir);
            if (mainDirent == NULL && errno != 0) {
                PRINT_ERROR
                exit(EXIT_FAILURE);
            }
            continue;
        } else if (mainDirent->d_type == DT_DIR) {
            studentGrade sg = {0};
            strcpy(sg.name, mainDirent->d_name);
            char path[SIZE] = {0};
            strncpy(path, configuration.path, strlen(configuration.path));
            strcat(path, "/");
            strcat(path, mainDirent->d_name);

            int hasCFile = moveDirectories(&configuration, &sg, path);
            if (!hasCFile) {
                sg.grade = NO_C_FILE_GRADE;
                strcpy(sg.gradeString, NO_C_FILE_STR);
            }

            //save the student
            saveStudent(&sg);

        }
        mainDirent = readdir(mainDir);
        if (mainDirent == NULL && errno != 0) {
            PRINT_ERROR
            exit(EXIT_FAILURE);
        }
    }
    checkOutputNum(closedir(mainDir));
}


/**
 *
 * @param path the path of the file that we get from the user
 * @param configuration the configuration
 * set the configurations according to the file from the args
 */
void setConfiguration(char *path, conFile *configuration) {
    int fd = open(path, O_RDONLY);
    checkOutputNum(fd);

    //size of buffer is 3 num of configuration * each size 150
    char bufInput[150 * 3];
    int numRead = read(fd, bufInput, sizeof(bufInput));
    checkOutputNum(numRead);

    int numOfConfi = 1; //num of the arg in the configuration struct
    int i = 0; //index in bufferInfut
    int j = 0;//index in each buffer;
    while (numOfConfi <= 3) {
        if (bufInput[i] == '\n') {
            numOfConfi++;
            j = 0;
        } else {
            switch (numOfConfi) {
                case 1:
                    configuration->path[j] = bufInput[i];
                    break;
                case 2:
                    configuration->input[j] = bufInput[i];
                    break;
                case 3:
                    configuration->output[j] = bufInput[i];
                    break;
            }
            j++;
        }
        i++;
    }
    //close the file
    checkOutputNum(close(fd));
}

/**
 *
 * @param num the num of the return value
 */
void checkOutputNum(int num) {
    if (num == -1) {
        printf("Error Number % d\n", errno);
    }
}

/**
 * @param num
 * check if the num is NULL
 */
void checkNULL(char* num) {
    if (num == NULL) {
        exit(EXIT_FAILURE);
    }
}


/**
 *
 * @param configuration configuration file from the user
 * @param sg struct of the student
 * @param path path of the directory we are in
 */
void runProgram(conFile *configuration, studentGrade *sg, char path[SIZE]) {
    pid_t child = fork();
    checkOutputNum(child);
    if (child == 0) {
        char execFile[SIZE] = {0};//a. out file
        strncpy(execFile, path, strlen(path));
        strcat(execFile, "/a.out");

        char outputFile[SIZE] = {0}; //a file for the outputs
        strncpy(outputFile, path, strlen(path));
        strcat(outputFile, "/out.txt");

        //get the input file
        int in = open(configuration->input, O_RDONLY);
        checkOutputNum(in);

        //get the output file
        int out = open(outputFile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        checkOutputNum(out);

        if (dup2(in, STDIN_FILENO) == -1) {
            PRINT_ERROR
            close(in);
            close(out);
            exit(EXIT_FAILURE);
        }
        if (dup2(out, STDOUT_FILENO) == -1) {
            PRINT_ERROR
            close(out);
            exit(EXIT_FAILURE);
        }
        checkOutputNum(close(in));
        checkOutputNum(close(out));

        //run the program
        char *args[2] = {execFile, NULL};
        execvp(execFile, args);
        exit(EXIT_FAILURE);

    }
    int time = 0;
    int status;
    while (!waitpid(child, &status, WNOHANG) && time < 5) {
        sleep(1);
        time++;
    }
    if (time == 5) {
        strcpy(sg->gradeString, TIMEOUT_STR);
        sg->grade = TIMEOUT_GRADE;
    } else {
        compareFilesAndUpadate(configuration, sg, path);
    }

    char outputFile1[SIZE] = {0}; //a file for the outputs
    strncpy(outputFile1, path, strlen(path));
    strcat(outputFile1, "/out.txt");
    unlink(outputFile1);

    char outputFile2[SIZE] = {0}; //a file for the outputs
    strncpy(outputFile2, path, strlen(path));
    strcat(outputFile2, "/a.out");
    unlink(outputFile2);

}

/**
 *
 * @param configuration file
 * @param sg student grade struct
 * @param path the directory we are in to
 * comparing between the 2 outputs we have: our output and the user output
 * and update the student grade according to what we got
 */
void compareFilesAndUpadate(conFile *configuration, studentGrade *sg, char path[SIZE]) {
    //we will use the compare program we did in ex31
    int score; //the input from the program

    pid_t child = fork();
    checkOutputNum(child);

    //we are in the child process
    if (child == 0) {
        //get the path of out.txt from the user
        char outputFile[SIZE] = {0};
        strncpy(outputFile, path, strlen(path));
        strcat(outputFile, "/out.txt");

        //get the compare program path: comp.out
        char comProgram[SIZE] = {0};
        checkNULL(getcwd(comProgram, sizeof(comProgram)));
        strcat(comProgram, "/comp.out");

        char* args[] = {"comp.out", configuration->output, outputFile, NULL};
        execvp(comProgram ,args);
        exit(-1);
    }

    int status;
    waitpid(child, &status, 0);
    //if compilation error not success
    if (WEXITSTATUS(status) == 0) {
        return;
    } else {
        score = WEXITSTATUS(status);
    }

    //update the struct of the student according to what we got from the compare program
    if (score == 1) {//identical
        sg->grade = GREAT_JOB_GRADE;
        strcpy(sg->gradeString, GREAT_JOB_STR);
    } else if (score == 2) { //different
        sg->grade = BAD_OUTPUT_GRADE;
        strcpy(sg->gradeString, BAD_OUTPUT_STR);
    } else if (score == 3) { //simillar
        sg->grade = SIMILAR_OUTPUT_GRADE;
        strcpy(sg->gradeString, SIMILAR_OUTPUT_STR);
    }
}

/**
 *
 * @param configuration - the configuration from the user
 * @param sg - the struct of the student
 * @param directory - the path we are into
 * @return 1 - if there is a c file in the directories 0-otherwise
 * the function runs on all the files in the directory
 */
int moveDirectories(conFile *configuration, studentGrade *sg, char directory[SIZE]) {
    int hasCFile;
    if (!(hasCFile == 1)) {
        hasCFile = 0;
    }
    //open the path that we got in the file using systemCall "openDir"
    DIR *dirc = opendir(directory);
    if (dirc == NULL) {
        PRINT_ERROR
        exit(EXIT_FAILURE);
    }

    //pointer to the first file that we wanna to read
    struct dirent *moveDirc = readdir(dirc);
    if (moveDirc == NULL && errno != 0) {
        PRINT_ERROR
        exit(EXIT_FAILURE);
    }

    //loop on the files and search in directory
    while (moveDirc != NULL) {
        if (!strcmp(moveDirc->d_name, "..") || !strcmp(moveDirc->d_name, ".")) {
            moveDirc = readdir(dirc);
            if (moveDirc == NULL && errno != 0) {
                PRINT_ERROR
                exit(EXIT_FAILURE);
            }
            continue;
        } else if (strcmp(moveDirc->d_name + (strlen(moveDirc->d_name)) - 2, ".c") == 0) {
            hasCFile = 1;
            int isCompileFile = isCompile(directory, moveDirc);
            if (isCompileFile == 0) { //not compile
                sg->grade = COMPILATION_ERROR_GRADE;
                strcpy(sg->gradeString, COMPILATION_ERROR_STR);
            } else {
                runProgram(configuration, sg, directory);
            }

            //if the file we read is a directory
        } else if (moveDirc->d_type == DT_DIR) {
            char path[150] = {0};
            memset(path, 0, strlen(path));
            strncpy(path, directory, strlen(directory));
            strcat(path, "/");
            strcat(path, moveDirc->d_name);
            moveDirectories(configuration, sg, path);
            memset(path, 0, strlen(path));
            strncpy(path, directory, strlen(directory));
        }

        //read the next directory
        moveDirc = readdir(dirc);
    }
    return hasCFile;
}

/**
 *
 * @param path the path of the directory we are in
 * @param rDirc pointer that runs on all the files in the directory
 * @return if the c program is compile
 */
int isCompile(char path[], struct dirent *rDirc) {
    pid_t childProc = fork();
    checkOutputNum(childProc);

    char buffCFile[SIZE] = {0};
    //create a path to the c file i want to compile
    strncpy(buffCFile, path, strlen(path));
    strcat(buffCFile, "/");
    strcat(buffCFile, rDirc->d_name);

    if (childProc == 0) {
        char outputFile[SIZE] = {0};
        //create the path to where to output file will be
        strncpy(outputFile, path, strlen(path));
        strcat(outputFile, "/a.out");
        char *command[5] = {"gcc", "-o", outputFile, buffCFile, NULL};
        execvp("gcc", command);
        exit(EXIT_FAILURE);
    }

    int status;
    checkOutputNum(waitpid(childProc, &status, 0));
    //if compilation error not success
    if (WEXITSTATUS(status) != 0) {
        return 0;
    } else {
        return 1;
    }
}

/**
 *
 * @param sg the struct of the student
 * save the student with his grades in the file
 */
void saveStudent(struct studentGrade *sg) {
    int out = open("results.csv", O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    checkOutputNum(out);
    char line[SIZE]={0};
    char temp[sizeof(int)];
    strcat(line, sg->name);
    strcat(line, ",");
    sprintf(temp, "%d", sg->grade);
    strcat(line, temp);
    strcat(line, ",");
    strcat(line, sg->gradeString);
    strcat(line, "\n");

    printf("%s", line);
    int in = write(out, line, strlen(line));
    checkOutputNum(in);
}
