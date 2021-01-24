 /*
    CS210 - Semester 2 - Simple Shell
    Authors: Shaun Greer, Callum Inglis, Mhari McGill, Niall Mcguire, Douglas Wheeler

    Assumptions:
        - Each command (line) is at most 512 characters long (As per the spec)
        - There is a maximum number of tokens that can be used in an input, as the array of tIndex commands is static in size. This is set to 50 tokens (As per the spec)
        - We will store the last 20 commands in a history array

    Important Info:
        - Ctrl+D will invoke EOF, EOF will be interpreted as NULL when using fgets(). Therefore to detect Ctrl+D we must check if fgets() == NULL
            It is also worth noting that if Ctrl+D is pressed twice mid-line, then the input stream will be closed without a \n character at the end,
            so we must check for this case, and if required exit the program
        - The command history (.hist_list) file will be saved in the user's home directory regardless of which directory they are currently in,
            because the closeShell() function restores the user's home directory before saving the history structure to a file.

    Stage 1 - Prompt user, and read and parse user input, exit shell and initialise the working directory
    Stage 2 - Executing external commands
    Stage 3 - Setting the current directory to Home, and getting and setting the path
    Stage 4 - Allow users to change directory
    Stage 5 - Store last 20 commands in history and allow users to re-run the commands
    Stage 6 - Persistent history - stores last 20 commands to a file on closing, reads the file and populates the history data structure on start up
    Stage 7 - Allow user to store up to 10 aliased commands
    Stage 8 - Persistent aliases - Store up to 10 alias'
    Stage 9 - Alias an alias
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h> /* Forking */
#include <sys/wait.h> /* Forking */
#include <unistd.h> /* Forking, change directory */
#include <dirent.h> /* CD Directory */
#include <errno.h> /* CD Directory */
#include <ctype.h> /* isDigit */

#include "src/h/display.h"
#include "src/h/colours.h"
#include "src/h/constants.h"
#include "src/h/enviroment.h"
#include "src/h/main.h"

#include "src/c/display.c" /* Displays state of the shell, such as CWD and path in a user-readable format */
#include "src/c/colours.c" /* Format the terminal output */
#include "src/c/enviroment.c" /* Getters & Setters for enviromental variables HOME and PATH */

int main(int argc, char const *argv[]) {

    /* Initialise variables for reading user input */
    char command[MAX_COMMAND_LENGTH]; // Input buffer to read command entered by the user
    char *tokens[T_MAX + 1]; // Array of input tokens (Individual commands)
    int tIndex; // Number of tokens entered per command. Points to where next token should be placed in *tokens[]

    /* Initialise History */
    char **history = malloc(MAX_HISTORY * sizeof(char *)); // Array to store last 20 commands entered by user
    int hIndex; // Index of where the next command should be stored in history array

    /* Initialise Alias */
    char **alias = malloc(MAX_ALIAS * sizeof(char *));
    int aIndex; // Index of where the next alias should be stored in alias array

    /* Initialise Shell */
    startShell(history, &hIndex, alias, &aIndex);

    /* Main Loop */
    for (;;) {
        prompt();

        // Read user input
        if (fgets(command, MAX_COMMAND_LENGTH, stdin) != NULL) {

            // Ensure the command entered does not exceed the maximum character length, as defined in MAX_COMMAND_LENGTH
            if (command[strlen(command) - 1] != '\n' && strlen(command) > MAX_COMMAND_LENGTH - 2) {
                // Display error, clear input buffer and prompt for next input
                red("[Error] ");
                printf("Input too long. The maximum command length is %i, please try again.\n\n", MAX_COMMAND_LENGTH - 2);
                clearBuffer(command);
                continue;

            // Ctrl+D pressed twice mid-line, exit the shell
            } else if (command[strlen(command) - 1] != '\n') {
                printf("\n");
                closeShell(history, hIndex, alias, aIndex);
            }


            // Check if the command is an alias. isAlias returns the index of the alias, or -1 if no alias found
            // Override stores the command from alias. If the command is not an alias, it remains null
            int aCount = isAlias(command, alias, aIndex);
            char *override = NULL;

            int foundAlias = 0;
            int circularAlias = 0;

            if (aCount >= 0) { foundAlias = 1; }

            char *originalAlias = alias[aCount]; // Store original alias to compare to command to prevent circular aliases

            // Deal with aliases of aliases
            while (foundAlias == 1) {
                foundAlias = 0;
                char *aliasName = alias[aCount];

                for (int i = 0; i < (MAX_ALIAS / 2); i++) { // loop through alias array to look for a match
                    char *compareTo = alias[i * 2];
                    if (strcmp(aliasName, compareTo) == 0) {
                        if (strcmp(originalAlias, alias[i * 2 + 1]) == 0) { // if circular alias detected
                            circularAlias = 1;
                            red("[Error] ");
                            printf("Circular alias called. Try \"unalias <command>\" to resolve.\n");

                        } else { // valid alias
                            aCount = (i * 2) + 1;
                            aliasName = alias[aCount]; // update alias
                            foundAlias = 1;
                            break;
                        }
                    }
                }
            }

            // only process command if a circular alias is not detected (either no alias or non-circular)
            if (!circularAlias) {
                // swap out alias for command
                if (aCount >= 0) { // >= 0 means it's an aliased command
                    override = malloc(sizeof(alias[aCount])); // Replace the first token of command with alias[aCount]
                    strcpy(override, alias[aCount]);
                }

                /* Check if this is a history invocation
                * If the input begins with !<no>, !! or !-<no> then the user is trying to execute a command from their
                * history
                * isHistory returns
                *      >= 0 when this is a valid history invocation
                *          This is the index of the command to be re-executed
                *      -2 on error
                *          The user has started their input with "!" but hasn't followed the correct input thereafter
                *          isHistory will have displayed an error message to the user about what went wrong
                *      -1 if not a history invocation
                *          In which case we will deal with the command the user entered instead
                *
                * rerunIndex: Index of the command to be rerun, or -2 on error, or -1 if not history invocation
                */
                int rerunIndex = isHistory(command, hIndex);

                /* User is calling a command from history, and it is valid.
                 * Copy command from history[rerunIndex] into command, and then carry on as normal.
                 * Instead of processing what the user entered (i.e. !!, !<no>, !-<no>), run the history call instead.
                 */
                if (rerunIndex >= 0) {
                    strcpy(command, history[rerunIndex]); /* Copy command from history array */

                    /* Check there is something in this index of history */
                    if (strtok(history[rerunIndex], "\n") != NULL) {
                        printf("%s\n", strtok(history[rerunIndex], "\n")); /* Display command that the user is running */

                        /* History[rerunIndex] is empty, display error then prompt user for next input */
                    } else {
                        red("[Error] ");
                        printf("Invalid history call. Please use \"history\" to view commands currently saved in shell history\n");
                        continue;
                    }
                tIndex = parseInput(command, tIndex, tokens, override, aIndex, history, hIndex);

                    /* Invalid format, an error will have been displayed by isHistory function already.
                     * Prompt user for next input
                     */
                } else if (rerunIndex == -2) {
                    continue;

                    /* Not a history invocation
                     * Check that command isn't empty - NOTE: A more through check is carried out in parseInput()
                     * Add this command to the history.
                     * Increment hIndex to where the next command should be stored in history[] array
                     */
                } else {
                    /* Command empty, prompt user for next command */
                    if (strtok(command, "\n") == NULL)
                        continue;

                    /* Copy command into history */
                    strcpy(history[hIndex], command);
                    hIndex = incrementHIndex(hIndex); /* Increment index for the next command */
                    numCommands++; // Increment number of commands run since startup
                }


                /* Check if the command is an alias. isAlias returns the index of the alias, or -1 if no alias found
                 * Override stores the command from alias. If the command is not an alias, it remains null
                 */
                int aCount = isAlias(command, alias, aIndex);
                char *override = NULL;

                int foundAlias = 0;
                int circularAlias = 0;

                if (aCount >= 0) {
                    foundAlias = 1;
                }

                char *originalAlias = alias[aCount]; // store original alias to compare to command to prevent circular aliases

                // Deal with aliases of aliases
                while (foundAlias == 1) { // >= 0 means it's an aliased command
                    foundAlias = 0;
                    char *aliasName = alias[aCount];

                    for (int i = 0; i < (MAX_ALIAS / 2); i++) { // loop through alias array to look for a match
                        char *compareTo = alias[i * 2];
                        if (strcmp(aliasName, compareTo) == 0) {
                            if (strcmp(originalAlias, alias[i * 2 + 1]) == 0) {
                                circularAlias = 1;
                                red("[Error] ");
                                printf("Circular alias called. Try \"unalias <command>\" to resolve.\n");
                            } else {
                                aCount = (i * 2) + 1;
                                aliasName = alias[aCount]; // update alias
                                foundAlias = 1;
                                break;
                            }
                        }
                    }
                }

                // only process command if a circular alias is not detected (either no alias or non-circular)
                if (!circularAlias) {
                    // swap out alias for command
                    if (aCount >= 0) { // >= 0 means it's an aliased command
                        override = malloc(sizeof(alias[aCount])); // Replace the first token of command with alias[aCount]
                        strcpy(override, alias[aCount]);
                    }


                    // Parse user input - Split up into tokens, also taking into account the override for alias
                    // Returns the number of tokens entered by the user
                    tIndex = parseInput(command, tIndex, tokens, override);

                    // Ensure at least one token entered, if not, prompt user for next command
                    if (tIndex == 0)
                        continue;

                    // Process each of the tokens entered by the user
                    processCommand(tIndex, tokens, history, &hIndex, alias, &aIndex, tIndex);
                }

            // EOF, End program. Also handles Ctrl+D
            } else {
                printf("\n");
                closeShell(history, hIndex, alias, aIndex);
            }

        }
    }
}



/**
 * Clears the input buffer so that on the next read the input stream is clear
 * @param command[]: the users original input, from fgets
 */
void clearBuffer(char command[]) {
    while (command[strlen(command)-1] != '\n')
        if (fgets(command,MAX_COMMAND_LENGTH,stdin) == NULL)
            break;
}


/**
 * Takes the command entered by the user and splits it up, based on DELIMITERS into individual commands, stored in
 * *tokens[].
 * We also perform a check to see if we've exceeded the size of the tokens array, if we have, then display a message to
 * the user and stop parsing any more commands. The program will continue, however it will only consider the first T_MAX
 * tokens that the user entered
 *
 * @param command[]: the users original input from fgets
 * @param tIndex: Number of tokens (commands) entered and index of where next token should be stored in tokens[]
 * @param tokens[]: Pointer to array of tokens (commands) entered by the user, to be filled by this function
 * @param alias: What tokens[0] should be changed to
 *
 * @return the number of tokens entered by the user
 *          *tokens[] is passed by reference so the array will be updated without having to pass anything back
 */
 int parseInput(char command[], int tIndex, char *tokens[], char *alias, int aIndex, char **history, int hIndex) {

    // Handle any alias, this will be passed in through alias and we should replace the first token of command with this
    // NOTE: alias may itself need to be tokenised, so we will add alias onto the beginning of command, remove the
    // first thing from command and then tokenise the new command
    if (alias != NULL) { // We have an alias

        char *tokensAlias[MAX_COMMAND_LENGTH]; // Array to hold the tokenised alias command

        // Begin to tokenise the alias
        tokensAlias[0] = strtok(NULL, DELIMITERS);

        int tAliasIndex = 0;
        while (tokensAlias[tAliasIndex] != NULL) {
            tAliasIndex++;
            tokensAlias[tAliasIndex] = strtok(NULL, DELIMITERS);
        }

        // We now add the alias onto the beginning of the normal command
        char *fullCommand = malloc(sizeof(MAX_COMMAND_LENGTH));

        strcpy(fullCommand, alias);

        for (int j = 0; j < tAliasIndex; ++j) {
            strcat(fullCommand, " ");
            strcat(fullCommand, tokensAlias[j]);
        }

        command = malloc(sizeof(fullCommand));
        strcpy(command, fullCommand);

        int i = isHistory(command, hIndex);

        if (i-1 == NULL){ // @TODO @ngb18130 what is this doing?
//            int aIndex = isAlias(command, alias, aIndex);
//            if (aIndex != -1){
//                command = history[aIndex];
//            } else {
                command = history[20];
//            }

        } else {
            command = history[i-1];
        }

        // Display the command to be executed to the user
        blue("[Info] ");
        printf("Executing: %s\n", command);
    }

     tIndex = 0; // Initialise token index
     tokens[tIndex] = strtok(command, DELIMITERS); // Take the first token from input

     while(tokens[tIndex] != NULL) { // The last token will be NULL, keep reading tokens until we get to the last one

         // There are now more tokens than we have space for
         if (tIndex >= T_MAX) {
             yellow("[Warning] ");
             printf("There are more tokens than there is space to store them. Considering the first %i tokens only.\n", T_MAX);
             return tIndex;
         }

         tIndex++; // Increment number of tokens

         tokens[tIndex] = strtok(NULL, DELIMITERS); // Take next token from the input and add to tokens array
     }

     // Hold a command as we build it up whilst checking for quotes at either end count the number of tokens that we remove
     char *commandQuotes = malloc(MAX_COMMAND_LENGTH);
     int numRemoved;

     for (int i = 0; i < tIndex; ++i) {
         // Does this token begin with a quote? Check it's either 1 character long or doesn't end with a quote
         if (tokens[i][0] == '"' && (tokens[i][strlen(tokens[i]) - 1] != '"' || strlen(tokens[i]) == 1)) {

             numRemoved = 0; // Keep track of the number of tokens we are removing
             strcpy(commandQuotes, tokens[i]); // We will build up a string with a full command surrounded by quotes
             commandQuotes++; // Removing the leading quote

             // Compare every token after this one, concatenate it with the command we are building up
             // If the token ends with a quote then this is the end of the command, so exit the loop
             for (int j = i+1; j < tIndex; ++j) {
                 numRemoved++;

                 // This token ends with a quote, so concatenate and then exit the loop
                 if (tokens[j][strlen(tokens[j]) - 1] == '"'){
                     strcat(commandQuotes, " ");
                     strcat(commandQuotes, tokens[j]);

                     j = tIndex; // Exit this for loop

                 // This token does not end with a quote, just concatenate
                 } else {
                     strcat(commandQuotes, " ");
                     strcat(commandQuotes, tokens[j]);
                 }
             }

             // Remove trailing quote then copy this new longer token into the correct index of the tokens array
             if (commandQuotes[strlen(commandQuotes)-1] == '"') { commandQuotes[strlen(commandQuotes)-1] = 0; }
             tokens[i] = malloc(MAX_COMMAND_LENGTH);
             strcpy(tokens[i], commandQuotes);

             // Shift all elements in tokens array to the left, relative to the number of tokens we removed
             for (int k = i + 1; k <= tIndex - numRemoved; ++k) {
                 tokens[k] = tokens[k+numRemoved];
             }

             // Decrement the number of tokens in the array
             tIndex -= numRemoved;

         } else {
             // Remove leading and trailing quotes
             if (tokens[i][0] == '"' && strlen(tokens[i]) > 1) { tokens[i]++; }
             if (tokens[i][strlen(tokens[i]) - 1] == '"') { tokens[i][strlen(tokens[i]) - 1] = 0; }
         }
     }

     return tIndex;
 }


/**
 * Based on the first token of the users' input, decide what action is to be performed
 * tokens[0] is the considered the command
 * tokens[1:] is considered the tokens
 *
 * If command not defined here, then we will fork and execute it as a system command. It may be the case that what the
 * user entered is also not a system command, in which case an error will be shown.
 *
 * @param n: Number of tokens (commands) entered
 * @param tokens[]: Pointer to array of tokens (commands) entered by the user
 * @param history: Pointer to String array of last executed commands
 * @param hIndex: Pointer to index of next command to be stored in history (Pointer as we may update it in this function)
 * @param aIndex: Index of the next alias in alias array
 * @param tIndex: Index of the next token in tokens array
 */
 void processCommand(int n, char *tokens[], char **history, int *hIndex, char **alias, int *aIndex, int tIndex) {

     /* Exit Program */
    if (strcmp(tokens[0], "exit") == 0) {
        closeShell(history, *hIndex, alias, *aIndex);

    /* Set environmental PATH variable */
    } else if (strcmp(tokens[0], "setpath") == 0) {
        // Ensure only one argument entered
        if (n > 2) { red("[Error] ");printf("\"setpath\" only accepts one argument. Try calling \"setpath <new path>\"\n"); return;}
        setPath(tokens[1]);

    /* Append a directory to environmental PATH variable */
    } else if (strcmp(tokens[0], "addpath") == 0) {
        // Ensure only one argument entered
        if (n > 2) {red("[Error] "); printf("\"addpath\" only accepts one argument. Try calling \"addpath <new path>\"\n"); return;}
        addPath(tokens[1]);

    /* Display the current stage of environmental PATH variable */
    } else if (strcmp(tokens[0], "getpath") == 0) {
        // Ensure no arguments entered
        if (n > 1) {red("[Error] "); printf("\"getpath\" does not accept any arguments. Try calling \"getpath\" by itself\n"); return;}
        displayPath();

    /* Set the users' home directory */
    } else if (strcmp(tokens[0], "sethome") == 0) {
        // Ensure only one argument entered
        if (n > 2) {red("[Error] "); printf("\"sethome\" only accepts one argument. Try calling \"sethome <new home dir>\"\n"); return;}
        setHome(tokens[1]);

    /* Display the current users home directory */
    } else if (strcmp(tokens[0], "gethome") == 0) {
        // Ensure no arguments entered
        if (n > 1) {red("[Error] "); printf("\"gethome\" does not accept any arguments. Try calling \"gethome\" by itself\n"); return;}
        displayHome();

    /* Display the current working directory */
    } else if (strcmp(tokens[0], "getcwd") == 0) {
        displayCWD();

    /* Change directory */
    } else if (strcmp(tokens[0], "cd") == 0) {
        // Ensure max of 1 argument
        if (n > 2) {red("[Error] "); printf("\"cd\" only accepts zero or one arguments. Try calling \"cd\" or \"cd <dir>\"\n"); return;}
        cd(tokens[1]);

    /* Display history to the user */
    } else if (strcmp(tokens[0], "history") == 0) {
        // Ensure no arguments
        if (n > 1) {red("[Error] "); printf("\"history\" does not accept any arguments. Try calling \"history\" by itself\n"); return;}
        dispHistory(*hIndex, history);

    /* Clear all commands from history */
    } else if (strcmp(tokens[0], "clearhistory") == 0) {
        // Ensure no arguments
        if (n > 1) {red("[Error] "); printf("\"clearhistory\" does not accept any arguments. Try calling \"clearhistory\" by itself\n"); return;}
        *hIndex = clearHistory(history);

    /* Display all aliases' or add a new alias */
    } else if (strcmp(tokens[0], "alias") == 0) {

        // Display all alias'
        if (n == 1) { // 0 args
            dispAlias(alias, *aIndex);

        // Add a new alias
        } else if (n >= 3) { // 2 args
            *aIndex = addAlias(alias, *aIndex, tokens[1], tokens[2], tokens, tIndex, history, hIndex);

        } else {
            red("[Error] "); printf("\"alias\" accepts zero or two (or more) arguments. Try calling \"alias\" or \"alias <name> <command>\"\n"); return;
        }

    /* Remove an alias */
    } else if (strcmp(tokens[0], "unalias") == 0){
        // Ensure 1 argument
        if (n != 2) {red("[Error] "); printf("\"unalias\" requires one argument. Try calling \"unalias <command>\"\n"); return;}
        *aIndex = unAlias(alias, *aIndex, tokens[1]);

    /* Not a command that we have defined, try to execute it as a system command */
    } else {
        id_t pid = fork();

        if (pid < 0) { // Something has went wrong with the new process
            red("[Error] ");
            printf("Error spawning child process...\n");

        } else if (pid == 0) { // Child process

            if (execvp(tokens[0], tokens) == -1) { /* Execute command, with arguments. Returns -1 on error */
                red("[Error] ");
                printf("That command was not found: %s\n", tokens[0]);
            }

            exit(0); // Return to parent

        } else { // Parent Process
            wait(NULL); // Wait for child process to finish
        }
    }
}


/**
 * Handles startup processes for the shell
 *
 * @param hIndex: The index that will hold the next command in history array
 * @param history: The array that will hold previous commands entered by the user
 * @param alias: The array that will hold all alias'
 * @param aIndex: The index for where the next alias should be stored
 */
 void startShell(char** history, int* hIndex, char** alias, int* aIndex) {
     originalPATH = getPath();
     originalHOME = getHome();

     system("clear"); // Clear terminal

     cd(NULL); // Navigate to users' home directory, NULL specifies home dir
     
     displayHome();
     displayPath();
     displayCWD();

     *hIndex = initialiseHistory(history); // Initialise history array and load history from file (if one exists)
     *aIndex = initialiseAlias(alias); // Initialise alias

     green(TOP_BOX);
     yellow(WELCOME);
     blue(CREDITS);
     green(TOP_BOX);
 }


 /**
  * Change the current working directory to filepath
  *
  * If filepath empty, "~" or "~/", go to users' home directory
  * Otherwise, check valid directory and go to that directory, otherwise display error message from errno
  *
  * @param filepath: new directory (Note, this could also be ../ or ./)
  */
 void cd(char *filepath) {

     // No filepath specified, go to users' home directory
     if (filepath == NULL) {
         chdir(getHome());

     /* User has typed in a directory that is relative to their home directory, such as "cd ~/Documents"
      * Decide if we should go to their home directory (e.g. either "~" or "~/")
      * or if we should go to a directory in their home directory, such as "~/Documents"
      * Remove the leading tilda, then construct an absolute path for the new directory getHome()/Documents (in this
      * example) and then return a recursive call to the cd function. This way we avoid repeating the same code to
      * actually change the directory!
      */
     } else if (filepath[0] == '~') {
         // user has typed "cd ~/" or "~", go to their home directory
         if (strcmp(filepath, "~/") == 0 || strcmp(filepath, "~") == 0) {
             return cd(NULL);
         }

         filepath = &filepath[1]; // Remove the tilda (~) from the filepath by pointing filepath to second char in filepath
         char *home = malloc(MAX_PATH); // We will concatenate the users' home dir and new filepath

         strcpy(home, getHome()); // Copy the users' home directory
         strcat(home, filepath); // Concatenate the target directory relative to home directory
         return cd(home); // Return recursive call to go to this directory

     // Handle the case of "cd .", which shouldn't do anything
     } else if (filepath[0] == '.' && strlen(filepath) == 1) {
         return;

     // Change to specified directory
     } else {
         // Try to open the path passed in by the user
         if (opendir(filepath) == NULL) { // Failed to open, display error from errno and return
             perror(filepath);
             red("[Error] ");
             printf("Please check %s exists and you have access\n", filepath);
             return;
         }

         chdir(filepath); // Navigate to the directory
     }
 }

/**
 * Restore original PATH and HOME, this was stored in the startShell() function
 * Save command history
 * Save aliases
 * Display closing message to user
 * Exit program
 *
 * @param history Array of history commands
 * @param hIndex Index of the next command in history
 * @param alias Array of aliased commands
 * @param aIndex Index of the next aliased command
 */
void closeShell(char **history, int hIndex, char **alias, int aIndex){
    setenv("PATH", originalPATH, 1); // Restore the original PATH
    setenv("HOME", originalHOME, 1); // Restore the original HOME
    cd(NULL); // Navigate home

    /* Save history to file */
    FILE *historyFP = fopen(".hist_list", "w"); /* Open a new file to write the history to, if the file already exists, its content is erased and it is treated as a new empty file*/

    int c = 0; /* Count the number of history items we have saved, such that we know once all have been displayed */

    while (c < MAX_HISTORY) { /* Stop when we have displayed the max number of items that could be in history */

        if (strtok(history[hIndex], "\n") != NULL) { /* If there is a command (That's not null), then save it */
            fprintf(historyFP, "%s\n", history[hIndex]);
        }

        hIndex = (hIndex + 1) % MAX_HISTORY; /* Increment the index we are now looking at */
        c++; /* Increment number of commands saved */
    }

     fclose(historyFP); // Close file stream
     blue("[Info] "); printf("History saved to file\n");


     /*Save aliases to file */
     int count = 0; /*Count the number of alias items we have saved, such that we know once all have been displayed */
     FILE *aliasFP = fopen(".alias", "w"); /*Open a new file to write the aliases to, if the file already exists, its content is erased and it is treated as a new empty file*/

     while (count < aIndex) { /* Stop when we have displayed the max number of items that could be in aliases */

        if (alias[count] != NULL) { /* If there is a command (That's not null), then save it */
            fprintf(aliasFP, "%s\t%s\n", alias[count], alias[count + 1]);
        }

       /* aIndex = aIndex + 2; /* Increment the index we are now looking at */
        count = count + 2; /* Increment number of commands saved */
    }


    fclose(aliasFP); // Close file stream
    blue("[Info] "); printf("Aliases saved to file\n");


    displayCWD();
    displayPath();
    displayHome();

    blue("[Info] "); printf("Exiting shell... Goodbye!\n"); // Closing message
    exit(0); // End program
}

/**
 * Increment the index for the next command to be entered into history
 * We are not using index 0 of history array, so there must be a special case to check for this
 * @param hIndex The current index, where the last command was placed in history array
 * @return newIndex The index where the next command should be placed in history array
 */
 int incrementHIndex(int hIndex) {
     int newIndex = (hIndex + 1) % (MAX_HISTORY); // Increment index for the next command

     if (newIndex == 0) // We are not using index 0!
         newIndex = 1;

     return newIndex;
 }

/**
 * function initialiseHistory
 * --------------------------
 * Allocate space to store a command at each index of the history array and
 * load commands from history file (if there is one)
 *
 * @param **history Array of history commands
 * @return the index of where the next command should be stored in the history array
 */
 int initialiseHistory(char **history) {

     int hIndex = 1; /* history index based on the contents of the file */

     FILE *fp = fopen(".hist_list", "r"); // Attempt to open history file

     char buffer[MAX_COMMAND_LENGTH]; // Input buffer for file reading

     // Initialise space for all history commands
     for (int i = hIndex; i < MAX_HISTORY; ++i) {
         history[i] = malloc(MAX_COMMAND_LENGTH * sizeof(char));
     }

     // Load in previous commands from file, if file is present
     if (fp != NULL) {
         while ((fgets(buffer, MAX_COMMAND_LENGTH, fp)) != NULL) {
             strtok(buffer, "\n"); // Remove trailing \n
             strcpy(history[hIndex], buffer); // Copy this command into history array

             hIndex = incrementHIndex(hIndex); // Increment file index, checking we haven't looped past MAX_HISTORY
             numCommands++;
         }
         fclose(fp);

         blue("[Info] ");
         printf("History loaded from file\n");
     }

     return hIndex; // Reset index for the new index
 }

/**
 * function clearHistory
 * ---------------------
 * Clears the history array
 *
 * @param **history Array of history commands
 * @return index where next command should be stored
 */
 int clearHistory(char **history) {

     for (int i = 1; i < MAX_HISTORY; ++i) {
         history[i] = malloc(MAX_COMMAND_LENGTH * sizeof(char));
     }

     blue("[Info] ");
     printf("Command History Cleared\n");

     numCommands = 0; // Commands displayed since shell startup

     return 1; // Reset index for the new index
 }

/**
 * Function isHistory
 * ------------------
 * Is the command that was just entered a history command?
 * This also looks for arrow key presses as well, and determines if they should be considered a history invocation.
 *
 * @param *command The command as entered by the user
 * @param hIndex The index where the next command should be stored
 *
 * @return   -1 = Not a history invocation
 *           -2 = Invalid command
 *
 *           Otherwise returns the index of the command to be run, i.e. 1-20
 */
 int isHistory(char *command, int hIndex) {

     /* Check if up/down arrows pressed, and treat them as history calls
      * Count number of up arrow presses, minus number of down arrow presses then compute history index to recall
      * Note that index will be 0 in the event that we should just run a normal command
      */
     int index, arrows = 0;

     for (int i = 0; i < strlen(command); i++) { // Check over the entire string
         if (command[i] == '\33') { // Arrow Key
             if(command[i+2] == 'A') { // UP
                 index++;arrows++;

             } else if(command[i+2] == 'B') { // DOWN
                 index--;arrows++;

                 /* Not valid, return -2 as sign of error, then prompt for next command */
             } else { return -2; }
         }
     }

     /* Check we have a valid index */
     if (index > 0 && index <= 20 && arrows > 0) {

         /* Ensure we are not returning an index < 1 (We are starting the indexing of history from 1) */
         if (hIndex - index < 1) { return hIndex - index + MAX_HISTORY - 1; }
         else { return hIndex - index; }

         /* Not a valid command, return such, then prompt for next command */
     } else if ((index < 0 || index > 20) && arrows > 0) { return -2; }


     /* Not a history invocation */
     if (command[0] != '!') { return -1; }


     /* Calculate the index of the first command available in history */
     int minCommand = numCommands - MAX_HISTORY + 2;
     if (minCommand < 1) { minCommand = 1; }

     /*  !! - Re-run last command */
     if (command[1] == '!') {

         /* There is no history so we can't actually display anything! Give user an error message */
         if (numCommands == 0) {
             red("[Error] ");
             printf("That history invocation was invalid: %s"
                    "\tYou must build up a history before you can recall previous commands.\n", command);
             return -2;

             /* Re-run the last command */
         } else {
             index = numCommands;
             while (index > 20) { index -= 20; } // Adxjust input to be in range of array
             return index;
         }


         /* !-<no> - Re-run command <no> characters ago */
     } else if (command[1] == '-') {

         char *c = malloc(sizeof(command[2]));

         strcpy(c, command);

         c += 2;

         for (int i=0;i<strlen(c); i++) {
             if (!isdigit(c[i])) {
                 red("[Error] ");
                 printf("Command was not found: %s"
                        "\tIt looks as though you've tried to enter an invalid number when invoking a command from history\n", command);
                 return -2;
             }
         }

         int minus = strtol(c, &c, 10);

         /* Out of range */
         if (minus == 0 || minus > 20 || minus > numCommands) {
             red("[Error] ");
             printf("That history invocation was invalid: %s", command);
             if (minCommand > numCommands) { printf("\tYou must build up a history before you can recall previous commands.\n"); }
             else { printf("\tPlease enter a number between 1 and 20\n"); }
             return -2;
         }

         /* Ensure we are not returning an index < 0 */
         if (hIndex - minus < 1) { return hIndex - minus + MAX_HISTORY - 1; }
         else { return hIndex - minus; }




         /* !<no> - Re-run the command at index <no> */
     } else if (strlen(command) > 2 && command[1] != '-'){
         char *c = &command[1]; // Take off the first 2 character of command (!)
         index = 0; /* Index of command to be run */

         /* Go through rest of !- command and store all of the numbers */
         while (*c) {
             if (isdigit(*c)) {
                 index = index * 10; /* Move all digits left 1 place, i.e. make 1 into 10 */
                 index += strtol(c, &c, 10); /* Add on the most recent number found */

             } else {
                 red("[Error] ");
                 printf("That command was not found: %s"
                        "\tIt looks as though you've tried to enter an invalid number when invoking a command from history\n", command);

                 return -2;
             }

             c++; /* Move onto next character */
         }

         /* Index out of range, either too small or too large. Display appropriate message */
         if (index < minCommand || index > numCommands) {
             red("[Error] ");
             printf("That history invocation was invalid: %s", command);
             if (minCommand > numCommands) { printf("\tYou must build up a history before you can recall previous commands.\n"); }
             else { printf("\tPlease enter a number between %i and %d\n", minCommand, numCommands); }
             return -2;

         } else {
             while (index > 20) { index -= 20; } // Adjust input to be in range of array
             return index;
         }



     } else { /* Invalid */
         red("[Error] ");
         printf("That command was not found: %s"
                "\tPerhaps you meant to pass in a history number? e.g. !<no>\n", command);
         return -2;
     }
 }


/**
 * Initialise the array to hold alias'
 * @param alias The array of alias'
 * @return The index to the next empty position in alias array
 */
int initialiseAlias(char** alias) {

    int aIndex = 0;

    FILE *fp = fopen(".alias", "r");

    char buffer[MAX_COMMAND_LENGTH];

    // Initialise space for all alias'
    for (int i = 0; i < MAX_ALIAS; ++i) {
        alias[i] = malloc(MAX_COMMAND_LENGTH * sizeof(char));
    }

     // Load in previous commands from file, if file is present
    if (fp != NULL) {
        while ((fgets(buffer, MAX_COMMAND_LENGTH, fp)) != NULL) {

            if (aIndex < MAX_ALIAS) {
                char *a = strtok(buffer, "\t");
                char *c = strtok(NULL, "\n");

                strcpy(alias[aIndex], a); // Copy this command into alias array
                strcpy(alias[aIndex + 1], c); // Copy this command into alias array

                aIndex += 2; // Increment file index, checking we haven't looped past MAX_ALIAS
            } else {
                yellow("[Warning] ");
                printf("Alias file corrupted. File too large. Loaded first %i aliases only\n", MAX_ALIAS/2);
            }
        }
        fclose(fp);

        blue("[Info] ");
        printf("Aliases loaded from file\n");
    }

    return aIndex; //Reset index for the new index
}

/**
 * Add a new alias. Check that it does not already exist
 *
 * Structure:
 *      Even Index: Name        e.g.    dir
 *      Odd Index:  Command     e.g.    ls -l
 *
 * @param alias The array of alias'
 * @param aIndex The index to the next empty position in alias array
 * @param name The alias of the command
 * @param command The command to be aliased
 * @param tokens The list of tokens entered by the user, we will store the command along with it's arguments
 * @param tIndex The index to the next token in tokens array
 * @param history History array to check is alias is a history invocation
 * @param hIndex index of history array
 */
int addAlias(char **alias, int aIndex, char *name, char *command, char *tokens[], int tIndex, char **history, int *hIndex){
    if (aIndex >= MAX_ALIAS) {
        red("[Error] "); printf("You have reached the maximum number of tokens that can be stored.\n"
                                                          "\tPlease run \"unalias <command>\" to free up space.\n");
        return aIndex;
    }

     // Concatenate the command we are storing with it's arguments (tokens[3:])
     char copy[MAX_COMMAND_LENGTH];
     strcpy(copy, command);
     command = malloc(MAX_COMMAND_LENGTH);

     if (*copy == '!') {
         int histIndex = isHistory(copy, *hIndex);
         if (histIndex >= 0) {
             histIndex -= 1;
             strcpy(copy, history[histIndex]);
        } else if (histIndex == -2) {
             yellow("[Warning] ");
             printf("\"%s\" is not a valid history invocation, adding anyway.\n", copy);

         }
     }

     strcpy(command, copy);

     int i = 3;
     while(tokens[i] != NULL) {
         strcat(command, " ");
         strcat(command, tokens[i]);
         i++;
     }

     if (strcmp(name, command) == 0) {
         red("[Error] "); printf("You can not alias a command as itself. Please try again.\n");
         return aIndex;
     }

     // Check to see if this alias already exists
     for(int i = 0; i < 20; i+=2){
        if(strcmp(alias[i], name) == 0){
            if (strcmp(alias[i+1], command) == 0) {
                blue("[Info] ");
                printf("You already have an alias with this name and command. Nothing to change.\n");
                return aIndex;
            }

            yellow("[Warning] ");
            printf("\"%s\" is already an alias for the command \"%s\"\n", name, alias[i+1]);

            blue("[Info] ");
            printf("Overriding the alias \"%s\" to be the new command \"%s\"\n", name, command);

            strcpy(alias[i+1], command);
            return aIndex;
        }
    }

    // Alias doesn't already exists
    strcpy(alias[aIndex], name);
    strcpy(alias[aIndex+1], command);
     blue("[Info] "); printf("Added \"%s\" under the alias \"%s\"\n", command, name);

    return aIndex + 2; // Increment the index for the next alias
}

/**
 * Remove an aliased command if it exists.
 * Shift all future alias' forward in the array.
 * Decrement aIndex
 *
 * @param alias The array of alias'
 * @param aIndex The index to the next empty position in alias array
 * @param command The command to be removed from alias
 * @return aIndex The index to the next empty position in alias array
 */
 int unAlias(char **alias, int aIndex, char *command) {

    for(int i = 0; i < 20; i+=2) {
        if (strcmp(command, alias[i]) == 0) {
            blue("[Info] "); printf("Removing alias %s for %s\n", alias[i], alias[i+1]);

            // Shift all future elements in alias array forward two places
            for (int j = i+2; j < aIndex; ++j) {
                alias[j-2] = alias[j];
            }

            // Allocate space in the alias array
            alias[aIndex-1] = malloc(MAX_COMMAND_LENGTH * sizeof(char));
            alias[aIndex-2] = malloc(MAX_COMMAND_LENGTH * sizeof(char));

            // Recrement the index for the next command in alias array
            return aIndex-2;
        }
    }

     yellow("[Warning] "); printf("Could not find an alias \"%s\", therefore it could not be removed!\n", command);
    return aIndex;
 }

/**
 * Check if the command entered has an alias, if so, return the index of that alias, otherwise return -1
 * @param command The command entered by the user
 * @param alias The array of alias'
 * @param aIndex The index to the next empty position in alias array
 * @return Index of alias, -1 if alias does not exist
 */
 int isAlias(char *command, char **alias, int aIndex){

     // Work with a copy of the command so as not to affect it's value elsewhere
     char tempCommand[MAX_COMMAND_LENGTH];
     strcpy(tempCommand, command);

     // Take the first token from the command
     char *command1 = strtok(tempCommand, DELIMITERS);

     // Does the first token of the command match any of our alias'?
     for (int i = 0; i < aIndex; i+=2) {
         if(strcmp(alias[i], command1) == 0){ // We have a match!
             return i+1; // Return the index where the aliased command is found
         }
     }

     return -1; // Failed to find the aliased command, return -1
 }
