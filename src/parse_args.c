// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include "main.h"

#define MAX_ARGS 10
#define DEFAULT_BASEDIR "/tmp"

// Use posix-specific commands to expand a path argument.
char *parse_posix_filepath(char *arg) {
#ifdef USE_POSIX_PATH_EXPANSION
    int out = 0;
    wordexp_t p;   
    // Use wordexp to expand the path shell-style.
    // This allows the use of environment variables,
    // and the '~' syntax for the home directory.
    // The flags forbid command expansions (WRDE_NOCMD),
    // forbid undefined shell variables (WRDE_UNDEF),
    // and print any errors to stderr (WRDE_SHOWERR)
    if(out = wordexp(arg, &p, WRDE_NOCMD | WRDE_UNDEF | WRDE_SHOWERR)) {
        if(out == WRDE_BADCHAR) {
            fprintf(stderr, "Path argument contained an illegal character: %s\n", arg);
        } else if(out == WRDE_BADVAL) {
            fprintf(stderr, "Path argument contained an undefined variable: %s\n", arg);
        } else if(out == WRDE_CMDSUB) {
            fprintf(stderr, "Path argument contained a command substitution: %s\n", arg);
        } else if(out == WRDE_NOSPACE) {
            fprintf(stderr, "Ran out of memory while expanding %s\n", arg);
        } else if(out == WRDE_SYNTAX) {
            fprintf(stderr, "Shell syntax error expanding %s\n", arg);
        } else {
            fprintf(stderr, "Unknown error expanding %s\n", arg);
        }
        exit(1);
    }
    if(p.we_wordc != 1) {
        fprintf(stderr, "Argument %s expanded to more than one word.\n", arg);
        exit(1);
    }
    // The recovered value is malloced, and thus never freed,
    // but since it's needed for the lifetime of the program...
    return p.we_wordv[0];
#else
    // Make doubly sure we don't use this on accident.
    // If we do, just print an error.
    fprintf(stderr, "Posix path expansion is forbidden in this build!\n");
    exit(1);
#endif
}

// Check that a filepath arg is syntactically legal.
char *parse_basic_filepath(char *arg) {
    // Without shell magic, there are some commonly-used paths
    // we cannot accept. Particularly, the '~' notation
    // for home folders.
    if(strchr(arg, '~') != NULL) {
        fprintf(stderr, "Cannot expand home directories in paths: %s\n", arg);
        exit(1);
    }
    // TODO I'm sure I'm forgetting other things to check...
    return arg; 
    
}

// Convert an argument string into a file path we can use.
char *parse_filepath(char *arg) {
    // There's a 'good' way to do this using linux-specific libraries,
    // but it's a) posix-linux specific and b) difficult to model.
    #ifdef USE_POSIX_PATH_EXPANSION
    arg = parse_posix_filepath(arg);
    #else
    arg = parse_basic_filepath(arg);
    #endif

    return arg;
}

// Make sure that a directory exists,
// and is accessible.
void check_dir(const char *arg) {
    //NOTE: Weldr does NOT take responsibility for creating basedir.
    //It's really annoying.
    struct stat s;
    if(stat(arg, &s)) {
        fprintf(stderr, "Failed to stat the path %s\n", arg);
        perror("Stat Error");
        exit(1);
    }
    if(!S_ISDIR(s.st_mode)) {
        fprintf(stderr, "Path %s is not a directory\n", arg);
        exit(1);
    }
    if((s.st_mode & (S_IWUSR | S_IWGRP | S_IWOTH)) == 0) {
        fprintf(stderr, "Path %s is not writable\n", arg);
        exit(1);
    }

}

// Parse command-line interface for a welded binary.
int parse_args(int argc, char** argv, int num_instances, const char **basedir, const char const **instance_names, const char ***instance_args, const char const ***default_instance_args) {
    int i;
    int j;
    
    // Assign default values to outputs.
    *basedir = DEFAULT_BASEDIR;
    for(i = 0; i < num_instances; i++) {
        instance_args[i] = default_instance_args[i];
    }
    
    // Start at argv[1]; We know argv[0] is the binary name.
    for(i = 1; i < argc; i++) {
        if(!strcmp(argv[i], "-h") || !(strcmp(argv[i], "--help"))) {
            //Print the help message and quit.
            printf("Welded binary.\nThis is a combination of several programs,\n"
                   "designed to run inside a single process.\n\n"
                   "Arguments may be passed to an instance through the following CLI:\n\n"
                   "\t--inst-args arg1,arg2,arg3,...argN\n\n"
                   "To allow communication with the individual programs,\n"
                   "this binary redirects the standard streams for each instance\n"
                   "to files.  For example: \n\n"
                   "\t%1$s/instA_stdin\n"
                   "\t%1$s/instA_stdout\n"
                   "\t%1$s/instA_stderr\n"
                   "\t%1$s/instB_stdin\n"
                   "\t%1$s/instB_stdout\n"
                   "\t%1$s/instB_stderr\n\n"
                   "stdout and stderr are standard files, while stdin is a fifo;\n"
                   "You can send a message to stdin by executing something like: \n\n"
                   "\techo 'msg' > %1$s/instA_stdin\n\n"
                   "This binary contains the following instances:\n", *basedir);
            // NOTE: Because we're quitting, we can reuse i.
            for(i = 0; i < num_instances; i++) {
                printf("\t%s\n", instance_names[i]);
            }
            exit(0);
        }
        if(!strcmp(argv[i], "--basedir")) {
            i++;
            char* arg;
            if(i >= argc) {
                fprintf(stderr, "Basedir option provided without an argument.\n");
                exit(1);
            }
            if(strlen(argv[i]) >= 2 && argv[i][0] == '-' && argv[i][1] == '-') {
                fprintf(stderr, "Expected a file path, but got another option %s\n", argv[i]);
                exit(1);
            }
            *basedir = parse_filepath(argv[i]);

        } else if(strstr(argv[i], "-args") != NULL) {
            // Get the bare instance name.
            char *inst_name = strtok(argv[i], "-"); 
            printf("Checking if instance is present: %s\n", inst_name);
            
            // Iterate through the instances until we find a match, or run out.
            int found = 0;
            for(j = 0; j < num_instances; j++) {
                // When we find a match, parse the args.
                if(!strcmp(instance_names[j], inst_name)) {
                    found = 1;
                    i++;
                    printf("Found args for %s: %s\n", inst_name, argv[i]);
                    if(i >= argc) {
                        fprintf(stderr, "ERROR! Argument flag provided for %s, but no arguments listed.\n", inst_name);
                        exit(1);
                    }
                    int idx = 0;
                    int num_args = 0;
                    while(argv[i][idx] != '\0') {
                        if(argv[i][idx] == ',') {
                            num_args++;
                            if(num_args > MAX_ARGS) {
                                fprintf(stderr, "ERROR! Passed too many arguments to instance %s\n", inst_name);
                                exit(1);
                            }
                        }
                        idx++;
                    }
                    //Account for argv[0]
                    num_args += 1;
                    printf("\tFound %d args:\n", num_args);

                    //Little trick; argv[0] is artificial, so use it to store argc.
                    instance_args[j] = malloc(sizeof(char*) * (num_args + 1));
                    instance_args[j][0] = malloc(sizeof(int));
                    *(int*)instance_args[j][0] = num_args;
                    char *arg = strtok(argv[i], ",");
                    for(idx = 1; idx <= num_args; idx++) {
                        printf("\t\t%s\n", arg);
                        instance_args[j][idx] = arg;
                        arg = strtok(NULL, ",");
                    }
                    break;
                }
            }
            if(!found) {
                fprintf(stderr, "ERROR!  Unknown instance: %s\n", inst_name);
                exit(1);
            }
        }
    }
    // Ensure the basedir is valid.
    check_dir(*basedir);
}
