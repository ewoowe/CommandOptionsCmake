#include <stdio.h>
#include <stdlib.h>

#include "command_options/command-options.h"

int main(int argc, const char *const argv[]) {
    int option;
    command_options_t options;

    command_options_init(&options, (char**)argv);
    while ((option = command_options_getopt(&options, "a:b:c:d:e:f:")) != -1) {
        switch (option) {
            case 'a':
                printf("%c = %s\n", option, options.optarg);
                break;
            case 'b':
                printf("%c = %s\n", option, options.optarg);
                break;
            case 'c':
                printf("%c = %s\n", option, options.optarg);
                break;
            case 'd':
                printf("%c = %s\n", option, options.optarg);
                break;
            case 'e':
                printf("%c = %s\n", option, options.optarg);
                break;
            case 'f':
                printf("%c = %s\n", option, options.optarg);
                break;
            default:
                printf("unknow %c = %s\n", option, options.optarg);
                break;
        }
    }
    return 0;
}
