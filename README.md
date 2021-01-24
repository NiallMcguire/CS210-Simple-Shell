<h3>Simple Shell</h3>
CS210 2019/20 Semester 2

By Shaun Greer, Callum Inglis, Mhari McGill, Niall Mcguire, Douglas Wheeler  

<h5>Build Instructions</h5>
Compile and run with the [GCC](https://gcc.gnu.org/) compiler: `gcc main.c main.h constants.h -o SimpleShell && ./SimpleShell`

<h5>Supported Commands</h5>

| Command 	| Description     	| 
|----------	|------------------	|
| `exit`   	| Exit the program 	|
| `Ctrl+D` 	| Exit the program 	|
| `getpath`	| Print system path |
| `setpath` | Set system path   |
| `addpath` | Append a directory to system path |
| `history` | Print history contents as a numbered list of most recent commands, ordered least to most recent |
| `clearhistory` | Clear all commands from history |
| `!!`      | Invoke the last from history |
| `!<no>`	| Invoke command with number \<no\> from history |
| `!-<no>`  | Invoke command with the number of the current command minus \<no\> |
| `cd <dir>`| Change Directory to \<dir\>, where \<dir\> can be a relative or absolute path |
| `cd ~`    | Change Directory to the users' home directory |
| `cd ~/<dir>` | Change Directory to a path relative to the users' home directory, e.g. ~/Documents |
| `gethome` | Print the home directory |
| `sethome` | Set the home directory |
| `alias` | Display list of current alias' |
| `alias <name> <command>` | Add a new alias \<alias\> for the \<command\> |
| `unalias <command>` | Remove an alias for the \<command\> |
| Note:|You can also enter any system command and this will be executed as an external process