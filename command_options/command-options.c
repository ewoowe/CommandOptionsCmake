#include "command-options.h"

#define COMMAND_OPTIONS_MSG_INVALID "invalid option"
#define COMMAND_OPTIONS_MSG_MISSING "option requires an argument"
#define COMMAND_OPTIONS_MSG_TOOMANY "option takes no arguments"

static int command_options_getopt_error(command_options_t *options, const char *msg, const char *data)
{
    unsigned p = 0;
    const char *sep = " -- '";
    while (*msg)
        options->errmsg[p++] = *msg++;
    while (*sep)
        options->errmsg[p++] = *sep++;
    while (p < sizeof(options->errmsg) - 2 && *data)
        options->errmsg[p++] = *data++;
    options->errmsg[p++] = '\'';
    options->errmsg[p++] = '\0';
    return '?';
}

void command_options_init(command_options_t *options, char **argv)
{
    options->argv = argv;
    options->permute = 1;
    options->optind = 1;
    options->subopt = 0;
    options->optarg = 0;
    options->errmsg[0] = '\0';
}

static int command_options_getopt_is_dashdash(const char *arg)
{
    return arg != 0 && arg[0] == '-' && arg[1] == '-' && arg[2] == '\0';
}

static int command_options_getopt_is_shortopt(const char *arg)
{
    return arg != 0 && arg[0] == '-' && arg[1] != '-' && arg[1] != '\0';
}

static int command_options_getopt_is_longopt(const char *arg)
{
    return arg != 0 && arg[0] == '-' && arg[1] == '-' && arg[2] != '\0';
}

static void command_options_getopt_permute(command_options_t *options, int index)
{
    char *nonoption = options->argv[index];
    int i;
    for (i = index; i < options->optind - 1; i++)
        options->argv[i] = options->argv[i + 1];
    options->argv[options->optind - 1] = nonoption;
}

static int command_options_getopt_argtype(const char *optstring, char c)
{
    int count = OGS_GETOPT_NONE;
    if (c == ':')
        return -1;
    for (; *optstring && c != *optstring; optstring++);
    if (!*optstring)
        return -1;
    if (optstring[1] == ':')
        count += optstring[2] == ':' ? 2 : 1;
    return count;
}

int command_options_getopt(command_options_t *options, const char *optstring)
{
    int type;
    char *next;
    char *option = options->argv[options->optind];
    options->errmsg[0] = '\0';
    options->optopt = 0;
    options->optarg = 0;
    if (option == 0) {
        return -1;
    } else if (command_options_getopt_is_dashdash(option)) {
        options->optind++; /* consume "--" */
        return -1;
    } else if (!command_options_getopt_is_shortopt(option)) {
        if (options->permute) {
            int index = options->optind++;
            int r = command_options_getopt(options, optstring);
            command_options_getopt_permute(options, index);
            options->optind--;
            return r;
        } else {
            return -1;
        }
    }
    option += options->subopt + 1;
    options->optopt = option[0];
    type = command_options_getopt_argtype(optstring, option[0]);
    next = options->argv[options->optind + 1];
    switch (type) {
    case -1: {
        char str[2] = {0, 0};
        str[0] = option[0];
        options->optind++;
        return command_options_getopt_error(options, COMMAND_OPTIONS_MSG_INVALID, str);
    }
    case OGS_GETOPT_NONE:
        if (option[1]) {
            options->subopt++;
        } else {
            options->subopt = 0;
            options->optind++;
        }
        return option[0];
    case OGS_GETOPT_REQUIRED:
        options->subopt = 0;
        options->optind++;
        if (option[1]) {
            options->optarg = option + 1;
        } else if (next != 0) {
            options->optarg = next;
            options->optind++;
        } else {
            char str[2] = {0, 0};
            str[0] = option[0];
            options->optarg = 0;
            return command_options_getopt_error(options, COMMAND_OPTIONS_MSG_MISSING, str);
        }
        return option[0];
    case OGS_GETOPT_OPTIONAL:
        options->subopt = 0;
        options->optind++;
        if (option[1])
            options->optarg = option + 1;
        else
            options->optarg = 0;
        return option[0];
    }
    return 0;
}

char *command_options_getopt_arg(command_options_t *options)
{
    char *option = options->argv[options->optind];
    options->subopt = 0;
    if (option != 0)
        options->optind++;
    return option;
}

static int command_options_getopt_longopts_end(const command_options_long_t *longopts, int i)
{
    return !longopts[i].longname && !longopts[i].shortname;
}

static void command_options_getopt_from_long(const command_options_long_t *longopts, char *optstring)
{
    char *p = optstring;
    int i;
    for (i = 0; !command_options_getopt_longopts_end(longopts, i); i++) {
        if (longopts[i].shortname) {
            int a;
            *p++ = longopts[i].shortname;
            for (a = 0; a < (int)longopts[i].argtype; a++)
                *p++ = ':';
        }
    }
    *p = '\0';
}

/* Unlike strcmp(), handles options containing "=". */
static int command_options_getopt_longopts_match(const char *longname, const char *option)
{
    const char *a = option, *n = longname;
    if (longname == 0)
        return 0;
    for (; *a && *n && *a != '='; a++, n++)
        if (*a != *n)
            return 0;
    return *n == '\0' && (*a == '\0' || *a == '=');
}

/* Return the part after "=", or NULL. */
static char *command_options_getopt_longopts_arg(char *option)
{
    for (; *option && *option != '='; option++);
    if (*option == '=')
        return option + 1;
    else
        return 0;
}

static int command_options_getopt_long_fallback(command_options_t *options, 
                                                const command_options_long_t *longopts,
                                                int *longindex)
{
    int result;
    char optstring[96 * 3 + 1]; /* 96 ASCII printable characters */
    command_options_getopt_from_long(longopts, optstring);
    result = command_options_getopt(options, optstring);
    if (longindex != 0) {
        *longindex = -1;
        if (result != -1) {
            int i;
            for (i = 0; !command_options_getopt_longopts_end(longopts, i); i++)
                if (longopts[i].shortname == options->optopt)
                    *longindex = i;
        }
    }
    return result;
}

int command_options_getopt_long(command_options_t *options, const command_options_long_t *longopts, int *longindex)
{
    int i;
    char *option = options->argv[options->optind];
    if (option == 0) {
        return -1;
    } else if (command_options_getopt_is_dashdash(option)) {
        options->optind++; /* consume "--" */
        return -1;
    } else if (command_options_getopt_is_shortopt(option)) {
        return command_options_getopt_long_fallback(options, longopts, longindex);
    } else if (!command_options_getopt_is_longopt(option)) {
        if (options->permute) {
            int index = options->optind++;
            int r = command_options_getopt_long(options, longopts, longindex);
            command_options_getopt_permute(options, index);
            options->optind--;
            return r;
        } else {
            return -1;
        }
    }

    /* Parse as long option. */
    options->errmsg[0] = '\0';
    options->optopt = 0;
    options->optarg = 0;
    option += 2; /* skip "--" */
    options->optind++;
    for (i = 0; !command_options_getopt_longopts_end(longopts, i); i++) {
        const char *name = longopts[i].longname;
        if (command_options_getopt_longopts_match(name, option)) {
            char *arg;
            if (longindex)
                *longindex = i;
            options->optopt = longopts[i].shortname;
            arg = command_options_getopt_longopts_arg(option);
            if (longopts[i].argtype == OGS_GETOPT_NONE && arg != 0) {
                return command_options_getopt_error(options, COMMAND_OPTIONS_MSG_TOOMANY, name);
            } if (arg != 0) {
                options->optarg = arg;
            } else if (longopts[i].argtype == OGS_GETOPT_REQUIRED) {
                options->optarg = options->argv[options->optind];
                if (options->optarg == 0)
                    return command_options_getopt_error(options, COMMAND_OPTIONS_MSG_MISSING, name);
                else
                    options->optind++;
            }
            return options->optopt;
        }
    }
    return command_options_getopt_error(options, COMMAND_OPTIONS_MSG_INVALID, option);
}