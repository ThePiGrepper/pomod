pomod(1)

# NAME

pomod - pomodoro daemon

# SYNOPSIS

*pomod* <options...> [COMMANDS]

# DESCRIPTION

pomod implements the Pomodoro Technique, as designed by Francesco Cirillo.
As it's name implies, pomod is implemented as a daemon, that means that the server-side process runs in
the background, waiting for a request, usually sent by the client-side process. This design decision
makes easier to show session information in status bars and other notification methods.

# OPTIONS

*-a* _cmd_
	Execute _cmd_ when a phase ends. Its intended use is sound playback. Recommended/tested commands are:
	*aplay*, *play* (If another command works fine, please report). Any filepath argument in _cmd_ *must* be absolute.

*-h*
	Show help message and quit.

# COMMANDS

*start*
	If used for the first time, start the server, as well as the first pomodoro cycle.
	In any other situation, restart clock, if halted.
*halt*
	Stop the clock.
*info*
	Query the server for session information. Information is sent to stdout.
	The output *FORMAT* is explained below.
*kill*
	Kill the pomodoro daemon. Internally, send *SIGINT* signal to the daemon.

# FORMAT

Data returned by _info_ is structured with a semicolon-separated, numerical format, as follows:++
"<phase>;<clock_state>;<pomodoro_count>;<remaining_time>"

Where:
	*phase* can be pomodoro(0), small break(1), or large break(2);

	*clock_state* can be halted(0), or running(1).

	*pomodoro_count* shows the number of pomodoros completed.

	*remaining_time* shows the amount of seconds to finish the current phase.

# AUTHOR

Maintained by Erick Cafferata <erick@cafferata.me>. For information about pomod development,
see https://gitlab.com/erickcc/pomod.
