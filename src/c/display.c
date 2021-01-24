// Here we define methods used to display the state of the shell to the user
// This includes: Home Directory, Current Path, Current Working Directory, Command History & Aliased Commands

/**
 * Display a prompt to the user, including the current working directory
 * If we are in the users' home directory, then display ~/ instead
 */
void prompt() {
    char cwd[MAX_PATH];

    // We are in users' home directory
    if (strcmp(getcwd(cwd, MAX_PATH), getHome()) == 0) {
        green("~/");

    // We are not in users' home directory
    } else {
        green(getcwd(cwd, MAX_PATH));
    }

    blue(PROMPT);
}

/* Display the users home directory */
void displayHome() {
    blue("[Info] ");
    printf("Current home directory is: %s\n", getHome());
}

/* Display the current path */
void displayPath() {
    blue("[Info] ");
    printf("Current path is: %s\n", getPath());
}

/* Display the current working directory */
void displayCWD() {
    blue("[Info] ");
    printf("Current working directory is: %s\n", printCwd());
}

/**
 * Display the last [MAX_HISTORY] commands that the user has entered, along with their reference number
 * The user can then enter !<num> where <num> in the reference number to re-run the command
 *
 * @param hIndex: The index of the next command in history array
 * @param **history: The array of previous commands entered by the user
 */
void dispHistory(int hIndex, char **history) {

    blue(" = Command History Begin =\n");

    int start = hIndex; // hIndex points to the next element in the array
    int c = 1; // Count the number of tokens that we have displayed
    int display; // Index of the command to be displayed, relative to the number of commands since shell startup
    int calcIndex = 0; // Index of the command to be displayed, relative to the array holding the history

    // Stop when we have displayed the max number of items that could be in history
    while (c < MAX_HISTORY) {
        if (strtok(history[start], "\n") != NULL) { // If there is a command, then display it

            // Calculate the index of the current command being displayed, checking it's not negative
            if (numCommands + 1 - MAX_HISTORY + c < 0) { display = c; }
            else { display = numCommands + 1 - MAX_HISTORY + c; }

            // Calculate the index of the current command relative to the array holding the history
            // If > 20, then keep removing 20 until we have a valid index
            calcIndex = display;
            while (calcIndex > 20) { calcIndex -= 20; }

            // Display command index (Since startup), followed by the command from the array
            printf(" %i\t%s\n", display, strtok(history[calcIndex], "\n"));
        }

        start = incrementHIndex(start); // Increment the index we are now looking at
        c++; // Increment number of tokens displayed
    }

    blue(" = Command History End =\n");
}

/**
 * Display all of the users alias'
 * @param alias The array of alias'
 * @param aIndex The index to the next empty position in alias array
 */
void dispAlias(char **alias, int aIndex) {
    // Check there are some alias' to display
    if (aIndex == 0) {
        blue("[Info] ");
        printf("There are currently no aliased commands. Add an alias with \"alias <name> <command>\"\n");
        return;
    }

    int start = aIndex;
    int c = 0; // Number of alias' displayed

    blue(" = Alias Begin =\n");
    printf(" Name\tCommand\n");

    while (c < MAX_ALIAS) {
        // If there is an alias, then display it
        if (strtok(alias[c], "\n") != NULL) {
            printf(" \"%s\"\t\"%s\"\n", strtok(alias[c], "\n"), strtok(alias[c + 1], "\n"));
        }

        c += 2; // Increment the index we are looking at by 2
    }
    blue(" = Alias End =\n");
}