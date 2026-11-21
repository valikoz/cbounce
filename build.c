#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/errno.h>

#define STRV_GROW_FACTOR 2
#define STRV_INIT_MAXLEN 256

typedef struct {
    int maxlen;
    int len;
    int itemsize;
} meta_t;

#define strv_meta(strv) ((meta_t*)(strv) - 1)
#define strv_free(strv) free(strv_meta(strv))
void strv_init(char ***strvp, size_t n);
void strv_push(char ***strvp, char *item);

void cmd_append_cflags(char **cmd);
int cmd_exec_child(char **cmd);
int cmd_exec(char **cmd);

int cstrlen(char *s);

int main(int argc, char **argv)
{
    (void)argv;
    char **cmd = NULL;
    strv_init(&cmd, sizeof(char*));

    if (argc > 1) {             /* Rebuild ourselves. */
        #define BUILD_EXE "./build"
        /* Rebuild current program. */
        strv_push(&cmd, "cc");
        strv_push(&cmd, "-o");
        strv_push(&cmd, BUILD_EXE);
        strv_push(&cmd, "build.c");
        strv_push(&cmd, "-Wall");
        strv_push(&cmd, "-Wextra");
        strv_push(&cmd, "-g");
        strv_push(&cmd, "-std=c89");

        if (!cmd_exec_child(cmd))
            return(1);

        /* Re-run current program. */
        strv_meta(cmd)->len = 0;
        strv_push(&cmd, BUILD_EXE);
        if (!cmd_exec(cmd)) {
            fprintf(stderr, "Could not exec command %s. errno: %d", cmd[0], errno);
            return(1);
        }
    }

    /* Build the dll. */
    strv_meta(cmd)->len = 0;
    strv_push(&cmd, "cc");
    strv_push(&cmd, "-o");
    strv_push(&cmd, "./game.so");
    strv_push(&cmd, "game.c");
    strv_push(&cmd, "-fPIC");
    strv_push(&cmd, "-shared");
    cmd_append_cflags(cmd);

    if (!cmd_exec_child(cmd))
        return(1);

    /* Build main executable. */
    strv_meta(cmd)->len = 0;
    strv_push(&cmd, "cc");
    strv_push(&cmd, "-o");
    strv_push(&cmd, "./main");
    strv_push(&cmd, "main.c");
    strv_push(&cmd, "-Wl,-rpath=./");
    cmd_append_cflags(cmd);

    if (!cmd_exec_child(cmd))
        return(1);

    /* strv_free(cmd); */
    return 0;
}

void cmd_append_cflags(char **cmd)
{
    strv_push(&cmd, "-Wall");
    strv_push(&cmd, "-Wextra");
    strv_push(&cmd, "-g");
    strv_push(&cmd, "-I/usr/local/include");
    strv_push(&cmd, "-L/usr/local/lib");
    strv_push(&cmd, "-lGL");
    strv_push(&cmd, "-lm");
    strv_push(&cmd, "-lpthread");
    strv_push(&cmd, "-ldl");
    strv_push(&cmd, "-lrt");
    strv_push(&cmd, "-lX11");
    strv_push(&cmd, "-I./raylib/raylib-6.2_linux_amd64/include");
    strv_push(&cmd, "-L./raylib/raylib-6.2_linux_amd64/lib");
    strv_push(&cmd, "-Wl,-rpath=./raylib/raylib-6.2_linux_amd64/lib");
    strv_push(&cmd, "-lraylib");
}

int cmd_exec_child(char **cmd)
{
    pid_t cpid;

    cpid = fork();

    if (cpid == -1)
        fprintf(stderr, "Failed to fork child process.\n");

    if (cpid == 0)   /* The child process. */
        cmd_exec(cmd);
    /* The parent process. */
    while (wait(NULL) != -1 || errno != ECHILD);

    return 1;

}

int cmd_exec(char **cmd)
{
    int i;
    for (i = 0; i < strv_meta(cmd)->len; ++i)
        fprintf(stdout, "%s%s", cmd[i], i < strv_meta(cmd)->len - 1 ? " " : "\n");
    if (cmd[strv_meta(cmd)->len - 1] != NULL)
        strv_push(&cmd, NULL);
    if (execvp((char *)cmd[0], (char * const *)cmd)) {
        fprintf(stderr, "Could not exec child process for %s. errno: %d\n", cmd[0], errno);
        return 1;
    }
    return 1;
}

void strv_init(char ***strvp, size_t n)
{
    meta_t *meta = malloc(n * STRV_INIT_MAXLEN + sizeof(meta_t));
    if (meta == NULL)
        exit(1);
    meta->itemsize = n;
    meta->maxlen = STRV_INIT_MAXLEN;
    meta->len  = 0;
    *strvp = (char**)(meta + 1);
}

void strv_push(char ***strvp, char *item)
{
    if (*strvp == NULL)
        strv_init(strvp, sizeof(char*));
    meta_t *meta = strv_meta(*strvp);
    if (meta->len >= meta->maxlen) {
        meta->maxlen *= STRV_GROW_FACTOR;
        meta = realloc(meta, (size_t)meta->itemsize * meta->maxlen + sizeof(meta_t));
        if (meta == NULL)
            exit(1);
        *strvp = (char**)(meta + 1); /* realloc may invalidate the array. */
    }
    (*strvp)[meta->len++] = item;
}
