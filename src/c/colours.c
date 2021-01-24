void colourReset () {
    printf("\033[0m");
}

void print(char* message) {
    printf("%s", message);
}


void red(char* message) {
    printf("\033[1;31m");
    print(message);
    colourReset();
}

void yellow(char* message) {
    printf("\033[1;33m");
    print(message);
    colourReset();
}

void green(char* message) {
    printf("\033[1;32m");
    print(message);
    colourReset();
}

void blue(char* message) {
    printf("\033[1;34m");
    print(message);
    colourReset();
}