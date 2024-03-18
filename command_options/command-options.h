#ifndef COMMAND_OPTIONS_H
#define COMMAND_OPTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct command_options_s {
    char **argv;
    int permute;
    int optind;
    int optopt;
    char *optarg;
    char errmsg[64];
    int subopt;
} command_options_t;

typedef enum {
    OGS_GETOPT_NONE,
    OGS_GETOPT_REQUIRED,
    OGS_GETOPT_OPTIONAL
} command_options_argtype_e;

typedef struct command_optioons_long_s {
    const char *longname;
    int shortname;
    command_options_argtype_e argtype;
} command_options_long_t;

void command_options_init(command_options_t *options, char **argv);
int command_options_getopt(command_options_t *options, const char *optstring);
int command_options_getopt_long(command_options_t *options, const command_options_long_t *longopts, int *longindex);
char* command_options_getopt_arg(command_options_t *options);

#ifdef __cplusplus
}
#endif

#endif