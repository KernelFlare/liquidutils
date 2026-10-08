#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#define PLACEHOLDER "[Hyperlink blocked]"

static void builtin_cd(char **args) {
    if (!args[1]) { fprintf(stderr, "cd: missing arg\n"); return; }
    if (chdir(args[1]) != 0) perror("cd");
}

static void builtin_pwd(char **args) {
    (void)args;
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd))) printf("%s\n", cwd);
}

static void builtin_ls(char **args) {
    DIR *dir = opendir(args[1] ? args[1] : ".");
    if (!dir) { perror("ls"); return; }
    struct dirent *entry;
    while ((entry = readdir(dir))) printf("%s ", entry->d_name);
    printf("\n");
    closedir(dir);
}

static void builtin_mkdir(char **args) {
    if (!args[1]) { fprintf(stderr, "mkdir: missing arg\n"); return; }
    if (mkdir(args[1], 0755) != 0 && errno != EEXIST) perror("mkdir");
}

static void builtin_rmdir(char **args) {
    if (!args[1]) { fprintf(stderr, "rmdir: missing arg\n"); return; }
    if (rmdir(args[1]) != 0) perror("rmdir");
}

static void builtin_rm(char **args) {
    if (!args[1]) { fprintf(stderr, "rm: missing arg\n"); return; }
    if (remove(args[1]) != 0) perror("rm");
}

static void builtin_date(char **args) {
    (void)args;
    time_t now = time(NULL);
    if (now == (time_t)-1) {
        perror("date");
        return;
    }
    struct tm *tm_info = localtime(&now);
    if (!tm_info) {
        perror("date");
        return;
    }
    char buf[64];
    if (strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Z %Y", tm_info) == 0) {
        fprintf(stderr, "date: strftime failed\n");
        return;
    }
    printf("%s\n", buf);
}

static void builtin_clear(char **args) {
    (void)args;
    printf("\033[2J\033[1;1H");
}

static void builtin_cat(char **args) {
    if (!args[1]) { fprintf(stderr, "cat: missing arg\n"); return; }
    FILE *f = fopen(args[1], "r");
    if (!f) { perror("cat"); return; }
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        fwrite(buf, 1, n, stdout);
    }
    printf("\n");
    fclose(f);
}

static void builtin_echo(char **args) {
    for (int i = 1; args[i]; i++) {
        printf("%s%s", args[i], args[i+1] ? " " : "");
    }
    printf("\n");
}

static void builtin_help(char **args) {
    (void)args;
    printf("Liquidutils dropshell — builtins:\n");
    printf("  cd <dir>    change directory\n");
    printf("  pwd         print working directory\n");
    printf("  ls [dir]    list directory\n");
    printf("  cat         read file\n");
    printf("  echo        copy input\n");
    printf("  date        show date and time\n");
    printf("  mkdir <d>   make directory\n");
    printf("  rmdir <d>   remove directory\n");
    printf("  rm <file>   remove file\n");
    printf("  clear       clear screen\n");
    printf("  help        this help\n");
    printf("  exit        leave shell\n");
    printf("Anything else runs as external command via fork/exec.\n");
}

static void builtin_exec(char **args) {
    if (!args || !args[0]) return;
    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        perror("exec");
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
    }
}

struct builtin { const char *name; void (*func)(char **); };
static struct builtin builtins[] = {
    {"cd",    builtin_cd},
    {"pwd",   builtin_pwd},
    {"echo",  builtin_echo},
    {"date",  builtin_date},
    {"cat",   builtin_cat},
    {"ls",    builtin_ls},
    {"mkdir", builtin_mkdir},
    {"rmdir", builtin_rmdir},
    {"rm",    builtin_rm},
    {"clear", builtin_clear},
    {"help",  builtin_help},
    {NULL, NULL}
};

static char **shell_split(char *line) {
    int cap = 16, len = 0;
    char **tokens = malloc(cap * sizeof(char *));
    if (!tokens) return NULL;
    char *tok = strtok(line, " \t\r\n");
    while (tok) {
        if (len >= cap) {
            cap *= 2;
            tokens = realloc(tokens, cap * sizeof(char *));
            if (!tokens) return NULL;
        }
        tokens[len++] = tok;
        tok = strtok(NULL, " \t\r\n");
    }
    tokens[len] = NULL;
    return tokens;
}

void builtin_shell(void) {
    const char *ver = "26.10.08" ;
    char *line = NULL;
    size_t buflen = 0;
    struct utsname buffer;
    if (uname(&buffer) != 0) {
        perror("bname: uname failed");
    }
    printf("Liquidutils DropSHell %s. Type 'help' for commands, 'exit' to leave.\n", ver);
    for (;;) {
        const char *user;
        user = getenv("USER");
        if (!user) user = getenv("LOGNAME");
        if (!user) user = PLACEHOLDER;
        printf("%s/%s/%s $ ", buffer.sysname, buffer.machine, user);
        fflush(stdout);
        ssize_t n = getline(&line, &buflen, stdin);
        if (n < 0) break;
        char **tokens = shell_split(line);
        if (!tokens || !tokens[0]) { free(tokens); continue; }
        if (strcmp(tokens[0], "exit") == 0) { free(tokens); break; }
        int found = 0;
        for (int i = 0; builtins[i].name; i++) {
            if (strcmp(tokens[0], builtins[i].name) == 0) {
                builtins[i].func(tokens);
                found = 1;
                break;
            }
        }
        if (!found) builtin_exec(tokens);
        free(tokens);
    }
    free(line);
}

int main(void){
    builtin_shell();
    return 0;
}
