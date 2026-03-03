#include <stdio.h>
#include <string.h>

#define INDENT 4
#define MAX_LINE 4096

typedef enum { NORMAL, IN_STRING, IN_CHAR, IN_LINE_COMMENT, IN_BLOCK_COMMENT } State;

/*
 * Scan one line counting { and } outside strings/comments.
 * *state persists across calls to handle block comments spanning lines.
 * Returns pre_close: number of } that appear before the first {.
 * Sets *open to total {, *close to total }.
 */
static int count_braces(const char *s, int *open, int *close, State *state)
{
    int pre_close = 0, seen_open = 0;
    *open = *close = 0;

    for (; *s; s++) {
        char c = s[0], n = s[1];
        switch (*state) {
        case NORMAL:
            if      (c == '"')               { *state = IN_STRING; }
            else if (c == '\'')              { *state = IN_CHAR; }
            else if (c == '/' && n == '/')   { *state = IN_LINE_COMMENT; }
            else if (c == '/' && n == '*')   { *state = IN_BLOCK_COMMENT; s++; }
            else if (c == '{')               { (*open)++; seen_open = 1; }
            else if (c == '}')               { (*close)++; if (!seen_open) pre_close++; }
            break;
        case IN_STRING:
            if      (c == '\\')  s++;           /* skip escaped char */
            else if (c == '"')   *state = NORMAL;
            break;
        case IN_CHAR:
            if      (c == '\\')  s++;
            else if (c == '\'')  *state = NORMAL;
            break;
        case IN_LINE_COMMENT:
            break;                              /* ignore until EOL */
        case IN_BLOCK_COMMENT:
            if (c == '*' && n == '/') { *state = NORMAL; s++; }
            break;
        }
    }
    if (*state == IN_LINE_COMMENT)
        *state = NORMAL;
    return pre_close;
}

static void format(FILE *fp)
{
    char line[MAX_LINE];
    int indent = 0;
    State state = NORMAL;

    while (fgets(line, sizeof line, fp)) {
        /* strip leading whitespace */
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;

        /* strip trailing newline */
        char *end = p + strlen(p);
        while (end > p && (end[-1] == '\n' || end[-1] == '\r')) *--end = '\0';

        /* blank line */
        if (*p == '\0') { putchar('\n'); continue; }

        int open, close;
        int pre_close = count_braces(p, &open, &close, &state);

        indent -= pre_close;
        if (indent < 0) indent = 0;

        for (int i = 0; i < indent * INDENT; i++) putchar(' ');
        puts(p);

        indent += open - (close - pre_close);
        if (indent < 0) indent = 0;
    }
}

int main(int argc, char *argv[])
{
    if (argc == 1) {
        format(stdin);
    } else {
        for (int i = 1; i < argc; i++) {
            FILE *fp = fopen(argv[i], "r");
            if (!fp) { fprintf(stderr, "cfmt: %s: cannot open\n", argv[i]); return 1; }
            format(fp);
            fclose(fp);
        }
    }
    return 0;
}
