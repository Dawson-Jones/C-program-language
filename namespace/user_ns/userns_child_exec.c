// allow UID and GID mappings to be specified 
// when creating a user namespace.

#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];    /* Space for child's stack */


struct child_args {
    char **argv;
    int pipe_fd[2]; // synchronize parent and child
};

static int verbose;

static void usage(char *pname)
{
    fprintf(stderr, "Usage: %s [options] cmd [arg...]\n\n", pname);
    fprintf(stderr, "Create a child process that executes a shell command in a new user namespace,\n"
                    "and possibly also other new namespace(s).\n\n");
    
    fprintf(stderr, "Options can be:\n\n");
#define fpe(str) fprintf(stderr, "      %s", str);
    fpe("-i             New IPC namespace\n");
    fpe("-m             New mount namespace\n");
    fpe("-n             New network namespace\n");
    fpe("-p             New PID namespace\n");
    fpe("-u             New UTS namespace\n");
    fpe("-U             New user namespace\n");
    fpe("-M uid_map     Specify UID map for user namespace\n");
    fpe("-G gid_map     Specify GID map for user namespcae\n");
    fpe("               if -M or -G is specified, -U ifs required\n");
    fpe("-v             Display verbose messages\n");
    fpe("\n");
    fpe("Map strings for -M and -G consist of records of the form:\n");
    fpe("\n");
    fpe("   ID-inside-ns    ID-outside-ns   len\n");
    fpe("\n");
    fpe("A map string can multiple records, separated commas;\n");
    fpe("the commas are replaced by newlines before writing to map files\n");

    exit(EXIT_FAILURE);
}


static void update_map(char *mapping, char *map_file)
{
    int fd, i;
    size_t map_len;

    map_len = strlen(mapping);
    for (i = 0; i < map_len; ++i) {
        if (mapping[i] == ',')
            mapping[i] = '\n';
    }

    fd = open(map_file, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "open %s: %s\n", map_file, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (write(fd, mapping, map_len) != map_len) {
        fprintf(stderr, "write %s: %s\n", map_file, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    close(fd);
}

static int child_func(void *arg)
{
    struct child_args *args = (struct child_args *) arg;
    char ch;

    close(args->pipe_fd[1]);
    if (read(args->pipe_fd[0], &ch, 1) != 0) {
        fprintf(stderr, "Failure in child: read from pipe returned != 0\n");
        exit(EXIT_FAILURE);
    }
    if (verbose) {
        printf("message received from parent: %x\n", ch);
    }

    execvp(args->argv[0], args->argv);
    errExit("execvp");
}


int main(int argc, char *argv[]) {
    int flags, opt;
    pid_t child_pid;
    struct child_args args;
    char *uid_map, *gid_map;
    char map_path[PATH_MAX];

    flags = 0;
    verbose = 0;
    uid_map = NULL, gid_map = NULL;

    while ((opt = getopt(argc, argv, "+imnpuUM:G:v")) != -1) {
        switch (opt) {
        case 'i': flags |= CLONE_NEWIPC;    break;
        case 'm': flags |= CLONE_NEWNS;     break;
        case 'n': flags |= CLONE_NEWNET;    break;
        case 'p': flags |= CLONE_NEWPID;    break;
        case 'u': flags |= CLONE_NEWUTS;    break;
        case 'U': flags |= CLONE_NEWUSER;   break;
        case 'M': uid_map = optarg;         break;
        case 'G': gid_map = optarg;         break;
        case 'v': verbose = 1;              break;
        default: usage(argv[0]);            break;
        }
    }
    if ((uid_map || gid_map) && !(flags & CLONE_NEWUSER)) {
        usage(argv[0]);
    }

    args.argv = &argv[optind];
    if (pipe(args.pipe_fd) == -1)
        errExit("pipe");
    
    child_pid = clone(child_func, child_stack + STACK_SIZE,
                    flags | SIGCHLD, &args);
    if (child_pid == -1)
        errExit("clone");
    
    if (verbose)
        printf("%s: PID of child created by clone() is %ld\n",
            argv[0], (long) child_pid);
    
    if (uid_map != NULL) {
        snprintf(map_path, PATH_MAX, "/proc/%ld/uid_map", (long) child_pid);
        update_map(uid_map, map_path);
    }
    if (gid_map != NULL) {
        // https://unix.stackexchange.com/questions/692177/echo-to-gid-map-fails-but-uid-map-success/692194#692194?newreg=6392de4a4bac40bda61383677511bbf9
        // Writing "deny" to the /proc/[pid]/setgroups file 
        // before writing to /proc/[pid]/gid_map 
        // will permanently disable setgroups(2) 
        // in a user namespace and allow writing to /proc/[pid]/gid_map 
        // without having the CAP_SETGID capability in the parent user namespace.
        snprintf(map_path, PATH_MAX, "/proc/%ld/setgroups", (long) child_pid);
        int fd = open(map_path, O_RDWR);    // error check
        write(fd, "deny", strlen("deny"));  // ignore
        close(fd);
        snprintf(map_path, PATH_MAX, "/proc/%ld/gid_map", (long) child_pid);
        update_map(gid_map, map_path);
    }

    close(args.pipe_fd[1]);
    if (waitpid(child_pid, NULL, 0) == -1)
        errExit("waitpid");
    
    if (verbose)
        printf("%s: terminating\n", argv[0]);

    return 0;
}
