#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ======= structs ======= //

typedef struct Scheme {
    char* str;
    int len;
} Scheme;

// Represents an option that takes no parameter.
// If this option is specified, handler is called.
typedef struct FlagOption {
    char* short_name;
    char* long_name;
    void (*handler)(void);
} FlagOption;

// Represents an option that takes an parameter.
// If this option is specified, handler is called with the parameter.
typedef struct ParamOption {
    char* short_name;
    char* long_name;
    void (*handler)(char*);
} ParamOption;

// ======= function declarations ======= //

static void parse_options(int* argc, char** argv[]);
static char parse_flag_option(char* name);
static char parse_param_option(char* name, char* param);
static void print_help();
static void parse_schemes(char* schemes);
static void fail(char* error_msg);
static void run(FILE* in_file);
static char* find_next_url(FILE* in_file);
static char* read_next_candidate(FILE* in_file);
static char is_valid_url(char* url);
static char is_url_char(int c);
static char is_valid_scheme(char* scheme);
static char is_scheme_char(int c);

// ======= global variables ======= //

#define BUFSIZE 512
// Holds the current URL (candidate).
char buffer[BUFSIZE];

// Holds the schemes that are recognized as valid starts of a URL.
static Scheme* schemes = NULL;
// Used to determine the number of schemes in the "schemes" variable.
static int schemes_count = 0;
#define NUM_DEFAULT_SCHEMES 6
static Scheme default_schemes[NUM_DEFAULT_SCHEMES] = {
    { "ftp", 3 },
    { "http", 4 },
    { "https", 5 },
    { "mailto", 6 },
    { "tel", 3 },
};

#define NUM_FLAG_OPTIONS 1
static FlagOption flag_options[NUM_FLAG_OPTIONS] = {
    { "h", "help", print_help },
};

#define NUM_PARAM_OPTIONS 1
static ParamOption param_options[NUM_PARAM_OPTIONS] = {
    // TODO add -n / --minlen option to specify a required minimum number of
    // characters after the scheme
    { "s", "schemes", parse_schemes },
};

// ======= function implementations ======= //

int main(int argc, char* argv[]) {
    // Skip the program name
    --argc;
    ++argv;

    parse_options(&argc, &argv);

    // If the -s / --schemes option was not specified, use the default schemes.
    if (schemes == NULL) {
        schemes = default_schemes;
        schemes_count = NUM_DEFAULT_SCHEMES;
    }

    if (argc == 0) {
        // If no files where specified, default to standard input.
        run(stdin);
    } else {
        // Otherwise, iterate through all files.
        do {
            if (!strcmp(*argv, "-")) {
                run(stdin);
            } else {
                // TODO if *argv is a directory, skip it
                FILE* in_file = fopen(*argv, "r");
                if (!in_file) {
                    fail(strerror(errno));
                }
                run(in_file);
                if (fclose(in_file)) {
                    fail(strerror(errno));
                }
            }

            --argc;
            ++argv;
        } while (argc > 0);
    }

    if (schemes != default_schemes) {
        free(schemes);
    }

    return 0;
}

// Parse all the command line options in the args.
//
// If this function returns, all options have been successfully parsed and the
// arguments remaining are the files from which URLs should be extracted.
void parse_options(int* argc, char** argv[]) {
    while ((*argc > 0) && ((*argv)[0][0] == '-')) {
        if ((*argc > 1) && parse_param_option((*argv)[0], (*argv)[1])) {
            *argc -= 2;
            *argv += 2;
        } else if (parse_flag_option((*argv)[0])) {
            *argc -= 1;
            *argv += 1;
        } else {
            // TODO better error message
            fail("Unrecognized argument");
        }
    }
}

// Parses a command line option that takes no paramter.
//
// If an option with the given name exists, its handler is called and 1 is
// returned. Otherwise, 0 is returned.
char parse_flag_option(char* name) {
    for (FlagOption* opt = flag_options; opt != flag_options + NUM_FLAG_OPTIONS; ++opt) {
        char matches = !strcmp(name + 1, opt->short_name)
            || ((name[1] == '-') && !strcmp(name + 2, opt->long_name));
        if (matches) {
            opt->handler();
            return 1;
        }
    }

    return 0;
}

// Parses a command line option that takes a paramter.
//
// If an option with the given name exists, its handler is called with the
// given parameter and 1 is returned. Otherwise, 0 is returned.
char parse_param_option(char* name, char* param) {
    for (ParamOption* opt = param_options; opt != param_options + NUM_PARAM_OPTIONS; ++opt) {
        char matches = !strcmp(name + 1, opt->short_name)
            || ((name[1] == '-') && !strcmp(name + 2, opt->long_name));
        if (matches) {
            opt->handler(param);
            return 1;
        }
    }

    return 0;
}

// Display a short help message and exit.
void print_help() {
    puts(
        "Usage: urls [OPTION]... [FILE]...\n"
        "\n"
        "Extract URLs from every FILE and write them to standard out, separated\n"
        "by newlines.\n"
        "With no FILE, or when FILE is -, read standard input.\n"
        "\n"
        "Options:\n"
        "  -h, --help        Display this help and exit\n"
        "  -s, --schemes     Comma-separated list of schemes to extract"
    );
    exit(0);
}

// Parse a comma-separated list of schemes. This is the value of the
// -s / --schemes command line option.
//
// If this function returns, "schemes_str" was valid and the parsed schemes
// have been stored in the global variable "stores".
void parse_schemes(char* schemes_str) {
    // Determine the number of schemes
    int count = 1;
    for (char* c = schemes_str; *c != '\0'; ++c) {
        if (*c == ',') {
            *c = '\0';
            ++count;
        }
    }

    schemes = realloc(schemes, count * sizeof(Scheme));
    if (!schemes) {
        fail("Failed to allocate memory");
    }

    // This does the actual parsing
    char* c = schemes_str;
    for (Scheme* s = schemes; s != schemes + count; ++s) {
        if (!is_valid_scheme(c)) {
            // TODO better error message
            fail("Invalid scheme");
        }
        size_t scheme_len = strlen(c);
        s->len = scheme_len;
        s->str = c;
        c += scheme_len + 1;
    }

    schemes_count = count;
}

// Prints "error_msg" to standard error and exits with a non-zero exit code.
void fail(char* error_msg) {
    fputs("Error: ", stderr);
    fputs(error_msg, stderr);
    exit(1);
}

// Extract URLs from "in_file" and write them to standard output separated by
// newlines.
void run(FILE* in_file) {
    char* next_url;
    while ((next_url = find_next_url(in_file))) {
        fputs(next_url, stdout);
        fputc('\n', stdout);
    }

    // TODO close the file if an error occurred
    if (ferror(in_file)) {
        fail(strerror(errno));
    }
}

// Find the next valid URL in the input stream (as determined by the
// "is_valid_url()" function).
//
// If successful, returns the URL found. Otherwise, that is, if EOF is reached
// or if an error occurred, returns NULL. In that case, feof and ferror can be
// used to disambiguate between the 2 possibilities.
char* find_next_url(FILE* in_file) {
    while (1) {
        char* next_candidate = read_next_candidate(in_file);

        if (!next_candidate || is_valid_url(next_candidate)) {
            return next_candidate;
        }
    }
}

// Find the next string in the input stream that consists of valid URL
// characters.
//
// If successful, returns the a string containing only valid URL characters.
// Otherwise, that, is if EOF is reached or if an error occurred, returns NULL.
// In that case, feof and ferror can be used to disambiguate between the 2
// possibilities.
char* read_next_candidate(FILE* in_file) {
    int next_char;
    while (1) {
        // Advance the stream until a valid URL character is found.
        while (1) {
            next_char = fgetc(in_file);
            if (next_char == EOF) {
                return NULL;
            } else if (is_url_char(next_char)) {
                *buffer = next_char;
                break;
            }
        }

        // Read characters into buffer until a non-URL character is found or
        // buffer is full.
        for (char* p = buffer + 1; p != buffer + BUFSIZE; ++p) {
            next_char = fgetc(in_file);

            if (!is_url_char(next_char)) {
                *p = 0;
                return buffer;
            }

            *p = next_char;
        }

        // Candidate is too large for buffer, skip it
        while (is_url_char(fgetc(in_file)));
    }
}

// Determine if a given string is a valid URL.
//
// A URL is considered valid if it starts with one of the schemes in the global
// variable "schemes" followed by a colon, followed by at least 1 arbitrary
// valid URL character.
char is_valid_url(char* url) {
    for (Scheme* s = schemes; s != schemes + schemes_count; ++s) {
        // This could cause undefined behaviour if there is a scheme s with
        //     s->len > BUFSIZE - 2
        char is_valid = !strncmp(s->str, url, s->len)
            && (url[s->len] == ':')
            && is_url_char(url[s->len + 1]);
        if (is_valid) { return 1; }
    }

    return 0;
}

// Determine if a character is a valid URL character.
//
// Returns false if the argument is EOF.
char is_url_char(int c) {
    return (c > 0x20)
        && (c != '"')
        && (c != '<')
        && (c != '>')
        && (c != '\\')
        && (c != '^')
        && (c != '`')
        && ((c < 0x7b) || (c == '~'));
}

// Determine if a given string is a valid scheme.
//
// A scheme is considered valid if it starts with an alphabetic character
// followed by any number of valid scheme characters (as determined by the
// function "is_scheme_char()".
//
// Additionally, this function imposes a restriction on the length of "scheme"
// to prevent a buffer overflow in the "is_valid_url()" function.
char is_valid_scheme(char* scheme) {
    if (!isalpha(scheme[0])) {
        return 0;
    }

    for (int idx = 1; scheme[idx] != '\0'; ++idx) {
        // If scheme is too long, it causes undefined behaviour in the function
        // is_valid_url()
        char is_too_long = idx > BUFSIZE - 2;
        if (is_too_long || !(is_scheme_char(scheme[idx]))) {
            return 0;
        }
    }

    return 1;
}

// Determine if a character is a valid scheme character.
//
// Returns false if the argument is EOF.
char is_scheme_char(int c) {
    return isalnum(c) || (c == '+') || (c == '-') || (c == '.');
}
