#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct commandTable_s {
    uint8_t numArgs;  // number of arguments that the command needs to execute
    char name[10];    // command
} commandTable_s;

static commandTable_s commandTable[] = {
    {0,
     "halt"},
    {0,
     "poweroff"},
    {0,
     "reboot"}};

int main(int argc, char** argv) {
    int c;
    opterr = 0;
    uint8_t optCmdPos = 0;  // option command position
    int16_t commandSelected = -1;
    while ((c = getopt(argc, argv, ":")) != -1)
        ;

    for (uint8_t i = 0; i < sizeof(commandTable) / sizeof(commandTable[0]); i++) {
        for (int8_t j = optind; j < argc; j++) {
            if (strcmp(argv[j], commandTable[i].name) == 0) {
                optCmdPos = j;        // command position in the argv
                commandSelected = i;  // number of command from the table
                break;
            }
        }
    }

    if (commandSelected < 0) {
        printf("Error: you must specify a command!\r\n");
        return EXIT_FAILURE;
    }

    for (uint8_t i = optCmdPos + 1; i < (optCmdPos + commandTable[commandSelected].numArgs); i++) {
        // iterating through needed arguments and setting options
    }

    if (strcmp(commandTable[commandSelected].name, "halt") == 0) {  // if command was halt, then ..
        printf("Halting! \r\n");
        kill(1, SIGUSR1);
        return EXIT_SUCCESS;
    } else if (strcmp(commandTable[commandSelected].name, "poweroff") == 0) {
        printf("Powering off! \r\n");
        kill(1, SIGUSR2);
        return EXIT_SUCCESS;
    } else if (strcmp(commandTable[commandSelected].name, "reboot") == 0) {
        printf("Rebooting! \r\n");
        kill(1, SIGTERM);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}