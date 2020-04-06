Minimal reproduction of a weird IO::Handle issue.

Scenario:

A program ("parent") creates a pipe, then forks.

The child process closes its read end of the pipe, dup2's the write end of
the pipe to file descriptor 3, makes sure close-on-exec is disabled, then
execs a perl script.

The parent process closes its write end of the pipe, then waits for a message
on the pipe.  In this case, it is very interested in seeing EOF when the
child closes its write end.

The perl script uses IO::Handle->new_from_fd() to get a FILEHANDLE connected
to the write end of the pipe.  It writes the message "ok" to the FILEHANDLE,
then closes it (and then, for good measure, undefs it too).

I expect the parent process to see:

* a 2-byte read, containing "ok" (and it does)
* a 0-byte read, indicating EOF on the pipe, as soon as the perl script
  closes the write end

The perl script sleeps for 10 seconds after closing the write end of the
pipe, so that the effect is obvious:

* the second read in the parent doesn't see EOF until the perl script exits!

(In the real version of this, the perl script is a long-running daemon.
It is unacceptable for the parent to not see EOF on the pipe until it
exits.)

To try it yourself:

```
make parent
./parent
```
