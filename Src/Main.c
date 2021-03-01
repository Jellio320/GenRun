#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>

#define BUFFER_SIZE 256
#define UNUSED(x) ((void) (x))

const char helpStr[477] = "Generate a batch file to auto run program.\nUsage: GenRun [options] name ...\nOptions:\n -a, --args\t\t\tAdditional arguments to automaticaly pass to program when run.\n -e, --extension\t\tFile extension of program. If no argument is passed, then defaults to \".exe\".\n -h, --help\t\t\tDisplays this message.\n -n, --name \t\t\tName of program to call in batch.\n -o, --output <file>\t\tOutputs to <file>. Defaults to \"Run.bat\".\n -p, --path <directory>\t\tPrefixes program call with \"<directory>\\\".\n";

char name[BUFFER_SIZE], ext[BUFFER_SIZE], path[BUFFER_SIZE], fname[BUFFER_SIZE], *args = "\0";
FILE* file;

static struct option longOptions[7] = {
    { "name", required_argument, 0, 'n' },
    { "args", required_argument, 0, 'a' },
    { "extension", optional_argument, 0, 'e' },
    { "help", no_argument, 0, 'h' },
    { "output", required_argument, 0, 'o' },
    { "path", required_argument, 0, 'p' },
    { 0, 0, 0, 0 }
};

void handler(int num) {
    if (file->_flag) {
        close(file->_file);
    }
    switch (num) {
    case SIGINT:
        exit(num);

    case SIGTERM:
        abort();
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    memset(name, 0, 1);
    memset(ext, 0, 1);
    memset(path, 0, 1);
    memset(fname, 0, 1);

    if (argc > 1) {
        opterr = 1;
        int opt, optIndex, passed = 0;
        char* cp;
        while ((opt = getopt_long(argc, argv, ":n:a:e::ho:p:", longOptions, &optIndex)) != -1) {
            switch (opt) {
            case 'n':
                strncpy(name, optarg, BUFFER_SIZE - 1);
                passed++;
                break;

            case 'a':
                args = optarg;
                passed++;
                break;

            case 'e':
                if (optarg) {
                    strncpy(ext, optarg, BUFFER_SIZE - 1);
                } else {
                    strcpy(ext, ".exe");
                }
                passed++;
                break;

            default:
            case 'h':
                printf(helpStr);
                return 1;

            case 'o':
                strncpy(fname, optarg, BUFFER_SIZE - 1);
                passed++;
                break;

            case 'p':
                strncpy(path, optarg, BUFFER_SIZE - 1);
                for (cp = path; *cp != 0; cp++) {
                    if (*cp == '/') {
                        *cp = '\\';
                    }
                }
                passed++;
                break;

            case '?':
                fprintf(stderr, "Unknown option ignored: %c\n", optopt);
                break;

            case ':':
                fprintf(stderr, "%c is missing a paramater.\n", optopt);
                switch (optopt) {
                case 'n':
                    fprintf(stderr, "Must specify a name.\n");
                    break;
                }
                return 1;
                break;
            }
        }

        int i = optind;
        while (i < (argc - 1)) {
            i++;
        }

        if (*name == 0) {
            strncpy(name, *(argv + i), BUFFER_SIZE - 1);
        }

    } else {
        printf(helpStr);
        return 1;
    }
    if (*name == 0) {
        fprintf(stderr, "Must specify a name.\n");
        return 1;
    }
    if (*fname == 0) {
        strcpy(fname, "Run.bat");
    }

    file = fopen(fname, "w");
    if (file == NULL) {
        perror("Error: ");
        return -1;
    }

    fprintf(file, "@ECHO OFF\ncall %s%s%s%s %s%s%%*\nECHO. 1>&2\nECHO %s%s returned %%ERRORLEVEL%% 1>&2", path, ((*path == 0) ? "\0" : "\\"), name, ext, args, ((*args == 0) ? "\0" : " "), name, ext);
    if (fclose(file) == EOF && errno != 0) {
        perror("Error: ");
        return -1;
    }
    printf("Wrote to file %s\n", fname);
    return 0;
}
