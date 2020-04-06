#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

void fatal(int code, const char *msg) __attribute__((noreturn));
void fatal(int code, const char *msg)
{
    perror(msg);
    exit(code);
}

static void fcntl_unset(int fd, int flag)
{
    int fdflags;
    
    fdflags = fcntl(fd, F_GETFD, 0);
    if (fdflags == -1) fatal(EX_OSERR, "fcntl F_GETFD");

    fdflags = fcntl(fd, F_SETFD, fdflags & ~flag);
    if (fdflags == -1) fatal(EX_OSERR, "fcntl F_SETFD");
}

static const int PIPE_WRITE_FD = 3;

int main (int argc __attribute__((unused)),
          char **argv __attribute__((unused)))
{
    pid_t pid;
    int child_pipe[2];
    int r;

    r = pipe(child_pipe);
    if (r) fatal(EX_OSERR, "pipe");

    pid = fork();
    if (pid == 0) {
        /* child */

        /* read end of pipe is closed in child */
        close(child_pipe[0]); 

        /* perl script gets the write end as fd 3 */
        r = dup2(child_pipe[1], PIPE_WRITE_FD);
        if (r != child_pipe[1]) close(child_pipe[1]);
        if (r < 0) fatal(EX_OSERR, "dup2");
        fcntl_unset(PIPE_WRITE_FD, FD_CLOEXEC);

        fprintf(stderr, "child: about to exec child.pl\n");

        execl("./child.pl", "./child.pl", NULL); /* doesn't return */
        fatal(EX_OSERR, "execlp");
    }
    else {
        /* parent */
        char buf[16];
        ssize_t nread;

        /* write end of pipe is closed in parent */
        close(child_pipe[1]);

        do {
            fprintf(stderr, "parent: about to read() from pipe...\n");
            errno = 0;
            nread = read(child_pipe[0], buf, sizeof(buf));
            fprintf(stderr, "parent: read() returned %ld (%d) \"%.*s\"\n",
                            nread, errno, (int) nread, buf);
        } while (nread > 0);
    }
}
