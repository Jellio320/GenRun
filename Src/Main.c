#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>

#define BUFFER_SIZE 256

const char helpStr[602]
    = "Generate a batch or shell script file to auto run program.\nUsage: GenRun [options] name ...\nOptions:\n -a, --args\t\t\tAdditional arguments to automaticaly pass to program when run. Quotation marks around argument is recommended.\n -e, --extension=\t\tFile extension of program. If no argument is passed, then defaults to \".exe\" on Windows & \".out\" on Linux.\n -h, --help\t\t\tDisplays this message.\n -n, --name \t\t\tName of program to call in batch.\n -o, --output=<file>\t\tOutputs to <file>. Defaults to \"run.bat\" on Windows & \"run.sh\" on Linux.\n -p, --path <directory>\t\tPrefixes program call with \"<directory>\\\".\n";

char name[BUFFER_SIZE] = "\0", ext[BUFFER_SIZE] = "\0", path[BUFFER_SIZE] = "\0",
#ifdef __linux
     fname[BUFFER_SIZE] = "run.sh",
#else
     fname[BUFFER_SIZE] = "run.bat",
#endif
     *args = "\0";

FILE* file;

struct option longOptions[7] = {
    { "name", required_argument, 0, 'n' },
    { "args", required_argument, 0, 'a' },
    { "extension", optional_argument, 0, 'e' },
    { "help", no_argument, 0, 'h' },
    { "output", optional_argument, 0, 'o' },
    { "path", required_argument, 0, 'p' },
    { 0, 0, 0, 0 }
};

void sigHandler(int num) {
#ifdef __linux
    if (file->_flags) {
        close(file->_fileno);
    }
#else
    if (file->_flag) {
        close(file->_file);
    }
#endif
    switch (num) {
    case SIGINT:
        exit(num);

    case SIGTERM:
        abort();
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    if (argc > 1) {
        opterr = 1;
        int opt, optIndex;
        while ((opt = getopt_long(argc, argv, ":n:a:e::ho::p:", longOptions, &optIndex)) != -1) {
            switch (opt) {
            case 'n':
                strncpy(name, optarg, BUFFER_SIZE - 1);
                break;

            case 'a':
                args = optarg;
                break;

            case 'e':
                if (optarg) {
                    char* cp = ext;
                    if (*optarg != '.') {
                        *cp++ = '.';
                    }
                    strncpy(cp, optarg, BUFFER_SIZE - 2);
                } else {
#ifdef __linux
                    strcpy(ext, ".out");
#else
                    strcpy(ext, ".exe");
#endif
                }
                break;

            default:
            case 'h':
                printf(helpStr);
                return 1;

            case 'o':
                if (optarg) {
                    strncpy(fname, optarg, BUFFER_SIZE - 1);
                } else {
                    fprintf(stderr, "No output file specified, ignoring option.\n");
                }
                break;

            case 'p':
                strncpy(path, optarg, BUFFER_SIZE - 1);
#ifndef __linux
                for (char* cp = path; *cp != 0; cp++) {
                    if (*cp == '/') {
                        *cp = '\\';
                    }
                }
#endif
                break;

            case '?':
                fprintf(stderr, "Unknown option ignored: %s\n", *(argv + (optind - 1)));
                break;

            case ':':
                fprintf(stderr, "Option \"%s\" is missing paramaters. ", *(argv + (optind - 1)));
                switch (optopt) {
                case 'n':
                    fprintf(stderr, "No program name specified.\n");
                    break;

                case 'a':
                    fprintf(stderr, "No additional arguments specified.\n");
                    break;

                case 'p':
                    fprintf(stderr, "No directory specified.\n");
                    break;
                }

                return 1;
            }
        }

        int i = optind;
        if (i < argc) {
            while (i < (argc - 1)) {
                i++;
            }

            if (*name == 0) {
                strncpy(name, *(argv + i), BUFFER_SIZE - 1);
            }
        }
    } else {
        printf(helpStr);
        return 1;
    }
    if (*name == 0) {
        fprintf(stderr, "Must specify a name.\n");
        return 1;
    }

    file = fopen(fname, "w");
    if (file == NULL) {
        perror("Error: ");
        return -1;
    }
#ifdef __linux
    fprintf(file, "#!/bin/bash\n./%s%s%s%s %s%s$*\necho\necho %s%s returned $?", path, ((*path == 0) ? "\0" : "/"), name, ext, args, ((*args == 0) ? "\0" : " "), name, ext);
#else
    fprintf(file, "@ECHO OFF\ncall %s%s%s%s %s%s%%*\nECHO. 1>&2\nECHO %s%s returned %%ERRORLEVEL%% 1>&2", path, ((*path == 0) ? "\0" : "\\"), name, ext, args, ((*args == 0) ? "\0" : " "), name, ext);
#endif
    if (fclose(file) == EOF && errno != 0) {
        perror("Error: ");
        return -1;
    }
    printf("Wrote to file %s\n", fname);
    return 0;
}
