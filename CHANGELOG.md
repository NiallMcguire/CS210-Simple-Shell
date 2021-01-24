#Simple Shell: Changelog
CS210 2019/20 Semester 2

By Shaun Greer, Callum Inglis, Mhari McGill, Niall Mcguire, Douglas Wheeler  

##Progress
Stage 1 Complete  
Stage 2 Complete  
Stage 3 Complete  
Stage 4 Complete  
Stage 5 Complete  

##Updates
23/01/2020: Stage 1 Started

*All working, just need to tidy up and refactor code

25/01/2020: Stage 1 completed [CI]

* Code tidied and refactored
* Handling each of the tokens (commands) is now handled in a separate function, as the number of recognised commands
      is going to increase as the program is built up
* Split rest of main function up into smaller functions
      
28/01/2020: Stage 2 Started [CI]

* Implementation of the fork command to call a subprocess, as well as error handling in the event that something goes
wrong
* After reading documentation for the exec() command, decided to use execvp as this takes into account the enviromental
variable PATH and allows us to call a command and its arguments
* A new assumption was made: The Simple Shell program will take one "full" command per line, for example "mkdir test".
The first token of this command "mkdir" is considered the command that is to be run, with subsequent tokens, "temp", 
considered the arguments of the command. Unlike a typical Linux shell, it is not currently possible to enter two commands
sequentially, such as "mkdir test && cd test". This was discussed with the Lecturer and is the recommended approach for
now.
* As such, we no longer iterate through the array of tokens and execute each one individually, instead we only look at
the token with index 0
* Removed the function call to display all of the arguments, but kept the function in the code. This was a required step
for stage 1 but not required for any subsequent stages.

30/01/2020: Stage 2 Completed [CI]
* Tidied up legacy code and function calls, tested all new features and all working well.
* Discussed steps for going forward in the group
* New assumption that the max number of tokens is 50, as per the spec.

30/01/2020: Stage 3 Completed [CI, SG, DW, MM, NM]
* Stage 3 completed and tested, fully working

30/01/2020: Stage 4 Completed [CI, SG, DW, MM, NM]
* Users can now change directories though the simple shell
* Current working directory is shown to the user in the prompt
* Navigate to home directory when shell starts up, now uses the cd function we added in stage 4

30/01/2020: Stage 5 Completed [NM, CI]

31/01/2020: Review  
* Reviewed all code up until this point and checked that we are meeting the requirements
* Refactoring of code where appropriate
* Addition of @TODO blocks for tasks that should be performed going forwards
* Added error message in the event that user types `setpath` but doesn't specify a directory to be added
* `setpath` now supports a user typing `setpath .`, `setpath ./` and `setpath ./Path/In/Current/Dir` as well as displaying the path to be added

31/01/2020: Minor correction
* `setpath` also now supports a user typing `setpath ~`, `setpath ~/` and `setpath ~/Path/In/Home/Dir` as well as displaying the path to be added`

04/02/2020: Stage 6 Completed
* Added `gethome` and `sethome` methods
* Added colour to terminal
* Fixed malloc issues when not allocating correct amount of space for checking a new path is valid
* Implemented Stage 6
* History saved to .hist_list in the users' home directory.