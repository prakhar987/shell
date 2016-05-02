Implementing a shell which supports semi-colon separated list of commands.Also support '&' operator which lets a program run in background after printing the process id of the newly created process.It can also handle background and foreground processes. It is also able to handle input/output redirections and pipes.


When you execute the code a shell prompt of the following form appears:
<username@system_name:curr_dir>

Builtin commands are contained within the shell itself. When the name of a builtin command is used as the first word of a
simple command the shell executes the command directly, without invoking another program
Commands implemented like :  cd , pwd and echo.

All other commands are treated as system commands like : emacs, vi and so on. The shell is able to execute them either in the
background or in the foreground.
Foreground processes: For example, executing a "vi" command in the foreground implies that your shell will wait for this process
to complete and regain control when this process exits.
Background processes: Any command invoked with "&" is treated as background command. This implies that your shell will
spawn that process and doesn't wait for the process to exit. It will keep taking user commands.
E.g
<Name@UBUNTU:~> ls &
This command when finished, should print its result to stdout.
<Name@UBUNTU:~>emacs &
<Name@UBUNTU:~> ls -l -a ( Make sure all the given flags are executed properly for any command and not just ls.)
. Execute other commands
<Name@UBUNTU:~> echo hello


INPUT OUTPUT Redirection :
Output of running one(or more) commands is redirected to a file. Similarly, a command might
be prompted to read input data from a file and asked to write output to another file.
E.g. Output redirection
<NAME@UBUNTU:~>diff file1.txt file2.txt > output.txt
E.g. Input redirection
<NAME@UBUNTU:~>sort < lines.txt
E.g. Input-Output redirection
<NAME@UBUNTU:~>sort < lines.txt > sorted-lines.txt


PIPELINING: 
A pipe is identified by "|". One or more commands can be piped as the following examples show.
Program is able to support any number of pipes.
E.g. Two Commands
<NAME@UBUNTU:~>more file.txt | wc
E.g. Three commands
<NAME@UBUNTU:~>grep "new" temp.txt | cat - somefile.txt | wc


The following commands are supported by the shell:

-jobs : prints a list of all currently running jobs along with their pid, particularly background jobs, in
order of their creation.
<NAME@UBUNTU:~>jobs
[1] emacs Assign.txt [231]
[2] firefox [234]
[3] vim [5678]

-kjob <jobNumber> <signalNumber>: takes the job id of a running job and sends a signal value to
that process
<NAME@UBUNTU:~> kjob 2 9
it sends sigkill to the process firefox, and as a result it is terminated. Here 9 represents the signal
number, which is sigkill.

-fg <jobNumber>: brings background job with given job number to foreground.
<NAME@UBUNTU:~> fg 3
Either brings the 3rd job which is vim to foreground or returns error if no such background number
exists.

-overkill: kills all background process at once

-quit : exits the shell. Your shell should exit only if the user types this "quit" command. It should
ignore any other signal from user like : CTRL-D, CTRL-C, SIGINT, SIGCHLD etc.
