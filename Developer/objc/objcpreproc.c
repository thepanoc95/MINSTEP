/*
 * objcpreproc.c - MINSTEP Objective-C Preprocessor
 *
 * A Stepstone-style ObjC-to-C translator with NeXT features and
 * selected Objective-C 2.0 extensions.
 *
 * Translation model:
 *   source.m -> [this preprocessor] -> source.c -> [C compiler]
 *
 * Handles:
 *   @interface/@implementation/@protocol/@property/@synthesize/@dynamic
 *   Categories, protocols, forward declarations
 *   Message expressions: [receiver selector:args]
 *   for...in fast enumeration
 *   @autoreleasepool
 *   @selector(), @protocol(), @encode()
 *   #import, #objc
 *   @public/@protected/@private/@package ivar visibility
 *   Class method (+) and instance method (-) definitions
 *   @compatibility_alias
 *
 * Licensed under the BSD License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>

/* ========================================================================
 * Configuration
 * ======================================================================== */

#define MAX_TOKEN      4096
#define MAX_LINE       8192
#define MAX_INCLUDES   64
#define MAX_CLASSES    512
#define MAX_PROTOCOLS  256
#define MAX_PROPERTIES 1024

#define VERSION "0.20.1-public"

/* Preprocessor-local BOOL type */
#ifndef BOOL
typedef int BOOL;
#endif
#ifndef YES
#define YES 1
#endif
#ifndef NO
#define NO  0
#endif

/* ========================================================================
 * State
 * ======================================================================== */

static FILE *input_file = NULL;
static FILE *output_file = NULL;
static const char *input_filename = "<stdin>";
static int line_number = 1;

/* Current class context for method translation */
static char current_class[256] = "";
static BOOL in_implementation = NO;
static BOOL in_interface = NO;
static BOOL in_protocol = NO;
static BOOL in_category = NO;
static BOOL in_ivar_block = NO;

/* Category info */
static char category_class[256] = "";
static char category_name[256] = "";

/* Property tracking for @synthesize */
typedef struct {
    char name[128];
    char ivar[128];
    char type[256];
    char attrs[256];
    char class_name[256];
} property_entry_t;

static property_entry_t properties[MAX_PROPERTIES];
static int property_count = 0;

/* Forward-declared classes */
static char forward_classes[MAX_CLASSES][128];
static int forward_class_count = 0;

/* Include guard tracking */
static char included_files[MAX_INCLUDES][512];
static int include_count = 0;

/* Include path search */
#define MAX_INCLUDE_PATHS 64
static char include_paths[MAX_INCLUDE_PATHS][512];
static int include_path_count = 0;

/* ========================================================================
 * Forward Declarations
 * ======================================================================== */

static void process_interface(void);
static void process_implementation(void);
static void process_protocol(void);
static void process_property(void);
static void process_synthesize(void);
static void process_dynamic(void);
static void process_class_forward(void);
static void process_compatibility_alias(void);
static void process_at_selector(void);
static void process_at_encode(void);
static void process_at_autoreleasepool(void);
static void process_message_expression(void);
static void process_method_declaration(BOOL is_class_method);
static void process_method_definition(BOOL is_class_method);
static void process_import(void);
static void process_source(void);
static void process_objc_directive(void);
static void process_ivar_block(void);
static BOOL check_include_guard(const char *filename);
static int read_char(void);
static int peek_char(void);
static void unread_char(int c);
static void skip_whitespace(void);
static void skip_line(void);
static char *read_identifier(char *buf, int bufsize);
static char *read_until(char *buf, int bufsize, char delim);
static void output(const char *fmt, ...);

/* ========================================================================
 * I/O Helpers
 * ======================================================================== */

static int read_char(void)
{
    int c;
    if (!input_file) return EOF;
    c = fgetc(input_file);
    return c;
}

static int peek_char(void)
{
    int c = read_char();
    if (c != EOF) unread_char(c);
    return c;
}

static void unread_char(int c)
{
    if (c != EOF && input_file) ungetc(c, input_file);
}

static void output(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(output_file, fmt, args);
    va_end(args);
}

/* ========================================================================
 * Token Reading
 * ======================================================================== */

static void skip_whitespace(void)
{
    int c;
    while ((c = peek_char()) != EOF) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            read_char();
        } else {
            break;
        }
    }
}

static void __attribute__((unused)) skip_line(void)
{
    int c;
    while ((c = read_char()) != EOF && c != '\n');
}

static char *read_identifier(char *buf, int bufsize)
{
    int c, i = 0;
    c = read_char();
    if (c == EOF || (!isalpha(c) && c != '_' && c != '@')) {
        if (c != EOF) unread_char(c);
        buf[0] = '\0';
        return buf;
    }
    buf[i++] = c;
    while (i < bufsize - 1) {
        c = peek_char();
        if (isalnum(c) || c == '_') {
            buf[i++] = read_char();
        } else {
            break;
        }
    }
    buf[i] = '\0';
    return buf;
}

static char *read_until(char *buf, int bufsize, char delim)
{
    int c, i = 0;
    while (i < bufsize - 1) {
        c = read_char();
        if (c == EOF || c == delim) break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return buf;
}

/* ========================================================================
 * Include Guard Checking
 * ======================================================================== */

static BOOL check_include_guard(const char *filename)
{
    int i;
    for (i = 0; i < include_count; i++) {
        if (strcmp(included_files[i], filename) == 0)
            return YES;
    }
    if (include_count < MAX_INCLUDES) {
        strncpy(included_files[include_count], filename, 511);
        included_files[include_count][511] = '\0';
        include_count++;
    }
    return NO;
}

/* ========================================================================
 * #import Handling
 * ======================================================================== */

static void preprocess_imported_file(const char *path)
{
    FILE *saved_input = input_file;
    const char *saved_name = input_filename;
    int saved_line = line_number;
    FILE *fp = fopen(path, "r");
    if (!fp) {
        /* Search include paths */
        int i;
        for (i = 0; i < include_path_count && !fp; i++) {
            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", include_paths[i], path);
            fp = fopen(fullpath, "r");
            if (fp) path = fullpath;
        }
    }
    if (fp) {
        input_file = fp;
        input_filename = path;
        line_number = 1;
        process_source();
        fclose(fp);
        input_file = saved_input;
        input_filename = saved_name;
        line_number = saved_line;
    } else {
        output("#include \"%s\"\n", path);
    }
}

static void process_import(void)
{
    char buf[MAX_LINE];
    int c;

    skip_whitespace();
    c = read_char();

    if (c == '"') {
        read_until(buf, MAX_LINE, '"');
        if (check_include_guard(buf)) {
            return;
        }
        preprocess_imported_file(buf);
    } else if (c == '<') {
        int i = 0;
        while ((c = read_char()) != EOF && c != '>' && i < MAX_LINE - 1) {
            buf[i++] = c;
        }
        buf[i] = '\0';
        if (check_include_guard(buf)) {
            return;
        }
        output("#include <%s>\n", buf);
    } else {
        unread_char(c);
        read_identifier(buf, MAX_LINE);
        output("#include %s\n", buf);
    }
}

/* ========================================================================
 * #objc Directive
 * ======================================================================== */

static void process_objc_directive(void)
{
    char buf[MAX_LINE];
    int c;

    c = peek_char();
    if (c == '\n' || c == EOF) {
        if (check_include_guard("objc/objc.h"))
            return;
        preprocess_imported_file("objc/objc.h");
        return;
    }

    skip_whitespace();
    read_until(buf, MAX_LINE, '\n');

    if (buf[0] == '\0') {
        if (check_include_guard("objc/objc.h"))
            return;
        preprocess_imported_file("objc/objc.h");
    } else {
        if (check_include_guard(buf))
            return;
        preprocess_imported_file(buf);
    }
}

/* ========================================================================
 * @interface Processing
 * ======================================================================== */

static void process_interface(void)
{
    char name[256] = "";
    char super[256] = "";
    char protocols[1024] = "";
    int c;
    BOOL has_super = NO;
    BOOL has_protocols = NO;

    in_interface = YES;

    skip_whitespace();
    read_identifier(name, sizeof(name));

    if (name[0] == '\0') {
        fprintf(stderr, "%s:%d: expected class name after @interface\n",
                input_filename, line_number);
        return;
    }

    strncpy(current_class, name, sizeof(current_class) - 1);

    skip_whitespace();
    c = peek_char();

    if (c == ':') {
        read_char();
        skip_whitespace();
        read_identifier(super, sizeof(super));
        has_super = YES;
        skip_whitespace();
    }

    c = peek_char();
    if (c == '<') {
        int i = 0;
        read_char();
        has_protocols = YES;
        while ((c = read_char()) != EOF && c != '>' && i < (int)sizeof(protocols) - 1) {
            protocols[i++] = c;
        }
        protocols[i] = '\0';
    }

    output("\n/* @interface %s", name);
    if (has_super) output(" : %s", super);
    if (has_protocols) output(" <%s>", protocols);
    output(" */\n");

    output("struct _%s_class_t;\n", name);

    output("struct %s {\n", name);
    output("    struct _%s_class_t *isa;\n", name);
    if (has_super) {
        output("    struct %s *_super;\n", super);
    }

    skip_whitespace();
    c = peek_char();
    if (c == '{') {
        process_ivar_block();
    }

    output("};\n\n");
    output("typedef struct %s %s;\n\n", name, name);

    /* in_interface stays YES until @end */
}

/* ========================================================================
 * Ivar Block Processing
 * ======================================================================== */

static void process_ivar_block(void)
{
    int c;
    int depth = 0;

    in_ivar_block = YES;

    c = read_char(); /* { */
    depth = 1;

    while (depth > 0) {
        c = read_char();
        if (c == EOF) break;
        if (c == '{') depth++;
        if (c == '}') { depth--; if (depth == 0) break; }

        if (c == '@') {
            char kw[32];
            read_identifier(kw, sizeof(kw));
            if (strcmp(kw, "public") == 0 || strcmp(kw, "protected") == 0 ||
                strcmp(kw, "private") == 0 || strcmp(kw, "package") == 0) {
                output("    /* %s */\n", kw);
                continue;
            }
        }

        output("%c", c);
    }

    in_ivar_block = NO;
}

/* ========================================================================
 * Method Declaration Processing
 * ======================================================================== */

static void process_method_declaration(BOOL is_class_method)
{
    char ret_type[256] = "id";
    char method_name[512] = "";
    char params[2048] = "";
    int c;
    int arg_idx = 0;

    skip_whitespace();

    c = read_char();
    if (c == '(') {
        int depth = 1, i = 0;
        while (depth > 0 && i < (int)sizeof(ret_type) - 1) {
            c = read_char();
            if (c == '(') depth++;
            else if (c == ')') depth--;
            if (depth > 0) ret_type[i++] = c;
        }
        ret_type[i] = '\0';
    } else {
        unread_char(c);
    }

    skip_whitespace();

    {
        char token[256];
        int i = 0;

        c = peek_char();
        if (c != ':' && c != ';' && c != '{') {
            read_identifier(token, sizeof(token));
            i += snprintf(method_name + i, sizeof(method_name) - i, "%s", token);
        }

        while (1) {
            skip_whitespace();
            c = peek_char();

            if (c == ':') {
                read_char();
                i += snprintf(method_name + i, sizeof(method_name) - i, "_");

                skip_whitespace();
                c = peek_char();
                if (c == '(') {
                    char arg_type[256];
                    int d = 1, j = 0;
                    read_char();
                    while (d > 0 && j < (int)sizeof(arg_type) - 1) {
                        c = read_char();
                        if (c == '(') d++;
                        else if (c == ')') d--;
                        if (d > 0) arg_type[j++] = c;
                    }
                    arg_type[j] = '\0';

                    skip_whitespace();
                    {
                        char arg_name[256];
                        read_identifier(arg_name, sizeof(arg_name));
                        if (arg_idx > 0) strcat(params, ", ");
                        strcat(params, arg_type);
                        strcat(params, " ");
                        strcat(params, arg_name);
                    }
                } else {
                    char arg_name[256];
                    read_identifier(arg_name, sizeof(arg_name));
                    if (arg_idx > 0) strcat(params, ", ");
                    strcat(params, "id ");
                    strcat(params, arg_name);
                }
                arg_idx++;
            } else {
                break;
            }
        }
    }

    skip_whitespace();
    c = peek_char();

    if (c == ';') {
        read_char();
        if (is_class_method) {
            output("    %s %s_cls_%s(Class cls, SEL _cmd%s%s);\n",
                   ret_type, current_class, method_name,
                   params[0] ? ", " : "", params);
        } else {
            output("    %s %s_inst_%s(id self, SEL _cmd%s%s);\n",
                   ret_type, current_class, method_name,
                   params[0] ? ", " : "", params);
        }
    } else if (c == '{') {
        process_method_definition(is_class_method);
    }
}

/* ========================================================================
 * Method Definition Processing
 * ======================================================================== */

static void process_method_definition(BOOL is_class_method)
{
    char ret_type[256] = "id";
    char method_name[512] = "";
    char params[2048] = "";
    int c;
    int arg_idx = 0;

    /* Read return type */
    skip_whitespace();
    c = read_char();
    if (c == '(') {
        int depth = 1, i = 0;
        while (depth > 0 && i < (int)sizeof(ret_type) - 1) {
            c = read_char();
            if (c == '(') depth++;
            else if (c == ')') depth--;
            if (depth > 0) ret_type[i++] = c;
        }
        ret_type[i] = '\0';
    } else {
        unread_char(c);
    }

    /* Read method name and keyword arguments */
    skip_whitespace();
    {
        char token[256] = "";
        read_identifier(token, sizeof(token));
        snprintf(method_name, sizeof(method_name), "%s", token);
    }

    while (1) {
        char token[256] = "";
        skip_whitespace();
        c = peek_char();

        if (c != ':') break;

        read_char(); /* consume ':' */
        strcat(method_name, "_");

        /* Read argument type in parentheses */
        skip_whitespace();
        c = peek_char();
        if (c == '(') {
            char arg_type[256] = "";
            int depth = 1, j = 0;
            read_char(); /* consume '(' */
            while (depth > 0 && j < (int)sizeof(arg_type) - 1) {
                c = read_char();
                if (c == '(') depth++;
                else if (c == ')') depth--;
                if (depth > 0) arg_type[j++] = c;
            }
            arg_type[j] = '\0';

            /* Read argument name */
            skip_whitespace();
            read_identifier(token, sizeof(token));

            if (arg_idx > 0) strcat(params, ", ");
            strcat(params, arg_type);
            strcat(params, " ");
            strcat(params, token);
        } else {
            /* No type parens - assume id */
            read_identifier(token, sizeof(token));
            if (arg_idx > 0) strcat(params, ", ");
            strcat(params, "id ");
            strcat(params, token);
        }
        arg_idx++;
    }

    /* Generate C function */
    output("\n/* %s %s %s */\n",
           is_class_method ? "class" : "instance",
           current_class, method_name);

    if (is_class_method) {
        output("%s %s_cls_%s(Class cls, SEL _cmd%s%s)\n",
               ret_type, current_class, method_name,
               params[0] ? ", " : "", params);
    } else {
        output("%s %s_inst_%s(id _obj, SEL _cmd%s%s)\n",
               ret_type, current_class, method_name,
               params[0] ? ", " : "", params);
    }

    /* Copy and preprocess method body */
    skip_whitespace();
    c = read_char();
    if (c == '{') {
        int depth = 1;
        output("{\n");
        /* For instance methods, re-declare self as proper struct type */
        if (!is_class_method && current_class[0]) {
            output("    struct %s *self = (struct %s *)_obj;\n",
                   current_class, current_class);
        }
        {
            int in_string = 0;
            int in_line_comment = 0;
            int in_block_comment = 0;
            int prev_c = '\n';
            int had_space = 1;  /* treat start-of-line as having had whitespace */
            while (depth > 0) {
                c = read_char();
                if (c == EOF) break;
                if (c == '\n') in_line_comment = 0;
                if (in_line_comment && c != '\n') { output("%c", c); continue; }
                if (in_block_comment) {
                    output("%c", c);
                    if (c == '*' && peek_char() == '/') {
                        output("%c", read_char());
                        in_block_comment = 0;
                    }
                    continue;
                }
                if (c == '"') { in_string = !in_string; output("%c", c); prev_c = c; had_space = 0; continue; }
                if (in_string) { output("%c", c); prev_c = c; had_space = 0; continue; }
                if (c == '/' && peek_char() == '/') { in_line_comment = 1; output("%c", c); continue; }
                if (c == '/' && peek_char() == '*') { in_block_comment = 1; output("%c", c); output("%c", read_char()); continue; }
                if (c == '{') { depth++; if (depth > 1) output("{"); prev_c = c; had_space = 0; continue; }
                if (c == '}') { depth--; if (depth > 0) output("}"); prev_c = c; had_space = 0; continue; }
                if (depth == 0) break;
                if (isspace(c)) { output("%c", c); had_space = 1; continue; }
                if (c == '[') {
                    /* Heuristic: digit after [ = C array; identifier + : or ident = ObjC msg */
                    int ahead = peek_char();
                    if (isalpha(ahead) || ahead == '_') {
                        process_message_expression();
                    } else {
                        output("[");
                    }
                    prev_c = ']'; had_space = 0;
                } else if (c == '@') {
                    char kw[64];
                    int pc = peek_char();
                    if (isalpha(pc) || pc == '_') {
                        read_identifier(kw, sizeof(kw));
                        if (strcmp(kw, "selector") == 0) {
                            process_at_selector();
                        } else if (strcmp(kw, "encode") == 0) {
                            process_at_encode();
                        } else if (strcmp(kw, "autoreleasepool") == 0) {
                            process_at_autoreleasepool();
                        } else {
                            output("@%s", kw);
                        }
                    } else {
                        output("@");
                    }
                    prev_c = '@'; had_space = 0;
                } else if (c == '#') {
                    char directive[64];
                    int pc = peek_char();
                    if (isalpha(pc) || pc == '_') {
                        read_identifier(directive, sizeof(directive));
                        output("#%s", directive);
                    } else {
                        output("#");
                    }
                    prev_c = '#'; had_space = 0;
                } else if (c == '_' && isalpha(peek_char()) && !is_class_method) {
                    /* Bare ivar access: _foo -> self->_foo
                     * Only rewrite if this _ starts a new token (after whitespace,
                     * operator, or start of line), not if it's part of a larger
                     * identifier (e.g. my_var, return_foo). */
                    if (had_space || prev_c == '(' || prev_c == ',' ||
                        prev_c == ';' || prev_c == '=' || prev_c == '!' ||
                        prev_c == '&' || prev_c == '|' || prev_c == '?' ||
                        prev_c == ':' || prev_c == '+' || prev_c == '-' ||
                        prev_c == '*' || prev_c == '/' || prev_c == '%' ||
                        prev_c == '<' || prev_c == '>' || prev_c == '{' ||
                        prev_c == '}' || prev_c == '[' || prev_c == '^' ||
                        prev_c == '~') {
                        output("self->");
                    }
                    output("_");
                    prev_c = '_'; had_space = 0;
                } else {
                    output("%c", c);
                    prev_c = c; had_space = 0;
                }
            }
        }
        output("}\n");
    }
}

/* ========================================================================
 * @implementation Processing
 * ======================================================================== */

static void process_implementation(void)
{
    char name[256] = "";
    int c;

    in_implementation = YES;

    skip_whitespace();
    read_identifier(name, sizeof(name));
    strncpy(current_class, name, sizeof(current_class) - 1);

    skip_whitespace();
    c = peek_char();

    if (c == '(') {
        read_char();
        in_category = YES;
        strncpy(category_class, name, sizeof(category_class) - 1);
        skip_whitespace();
        read_identifier(category_name, sizeof(category_name));
        skip_whitespace();
        read_char(); /* ) */
    }

    output("\n/* @implementation %s", name);
    if (category_name[0]) output(" (%s)", category_name);
    output(" */\n");
}

/* ========================================================================
 * @protocol Processing
 * ======================================================================== */

static void process_protocol(void)
{
    char name[256] = "";
    int c;

    in_protocol = YES;

    skip_whitespace();
    read_identifier(name, sizeof(name));

    skip_whitespace();
    c = peek_char();
    if (c == '<') {
        char super[1024] = "";
        int i = 0;
        read_char();
        while ((c = read_char()) != EOF && c != '>' && i < (int)sizeof(super) - 1) {
            super[i++] = c;
        }
        super[i] = '\0';
    }

    output("\n/* @protocol %s */\n", name);
    output("static Protocol *_proto_%s = NULL;\n", name);

    /* in_protocol stays YES until @end */
}

/* ========================================================================
 * @property Processing
 * ======================================================================== */

static void process_property(void)
{
    char attrs[1024] = "";
    char type_str[256] = "id";
    char name_str[256] = "";
    char getter_str[256] = "";
    char setter_str[256] = "";
    int c;

    skip_whitespace();

    c = read_char();
    if (c == '(') {
        int depth = 1, i = 0;
        while (depth > 0 && i < (int)sizeof(attrs) - 1) {
            c = read_char();
            if (c == '(') depth++;
            else if (c == ')') depth--;
            if (depth > 0) attrs[i++] = c;
        }
        attrs[i] = '\0';
    } else {
        unread_char(c);
    }

    skip_whitespace();

    c = peek_char();
    if (c == '(') {
        int depth = 1, i = 0;
        read_char();
        while (depth > 0 && i < (int)sizeof(type_str) - 1) {
            c = read_char();
            if (c == '(') depth++;
            else if (c == ')') depth--;
            if (depth > 0) type_str[i++] = c;
        }
        type_str[i] = '\0';
        skip_whitespace();
    } else if (isalpha(c) || c == '_') {
        /* Bare type like id, int, NSString * etc.
         * Peek ahead: if followed by another identifier, first is type, second is name */
        char first_id[256] = "";
        int fi = 0;
        while ((c = peek_char()) != EOF && (isalnum(c) || c == '_')) {
            first_id[fi++] = read_char();
        }
        first_id[fi] = '\0';
        skip_whitespace();
        c = peek_char();
        if (isalpha(c) || c == '_') {
            /* Two identifiers: first is type, second is name */
            snprintf(type_str, sizeof(type_str), "%s", first_id);
        } else {
            /* Single identifier: it's the name, type defaults to id */
            snprintf(name_str, sizeof(name_str), "%s", first_id);
            goto property_parse_attrs;
        }
    }

    read_identifier(name_str, sizeof(name_str));

    property_parse_attrs:
    {
        char *p;
        BOOL is_readonly = NO;

        p = strstr(attrs, "readonly");
        if (p) is_readonly = YES;

        p = strstr(attrs, "getter=");
        if (p) {
            int i = 0;
            p += 7;
            while (*p && *p != ',' && i < (int)sizeof(getter_str) - 1) {
                getter_str[i++] = *p++;
            }
            getter_str[i] = '\0';
        } else {
            snprintf(getter_str, sizeof(getter_str), "%s", name_str);
        }

        p = strstr(attrs, "setter=");
        if (p) {
            int i = 0;
            p += 7;
            while (*p && *p != ',' && i < (int)sizeof(setter_str) - 1) {
                setter_str[i++] = *p++;
            }
            setter_str[i] = '\0';
        } else if (!is_readonly) {
            snprintf(setter_str, sizeof(setter_str), "set%c%s:",
                     toupper((unsigned char)name_str[0]), name_str + 1);
        }
        (void)setter_str;
        (void)getter_str;
    }

    skip_whitespace();
    c = peek_char();
    if (c == ';') read_char();

    {
        property_entry_t *pe;
        if (property_count < MAX_PROPERTIES) {
            pe = &properties[property_count++];
            strncpy(pe->name, name_str, sizeof(pe->name) - 1);
            strncpy(pe->type, type_str, sizeof(pe->type) - 1);
            strncpy(pe->attrs, attrs, sizeof(pe->attrs) - 1);
            strncpy(pe->class_name, current_class, sizeof(pe->class_name) - 1);
            snprintf(pe->ivar, sizeof(pe->ivar), "_%s", name_str);
        }
    }

    output("\n/* @property %s %s */\n", type_str, name_str);
    output("- (%s)%s;\n", type_str, name_str);
    {
        BOOL is_readonly = strstr(attrs, "readonly") != NULL;
        if (!is_readonly) {
            output("- (void)set%c%s:(%s)value;\n",
                   toupper((unsigned char)name_str[0]), name_str + 1, type_str);
        }
    }
}

/* ========================================================================
 * @synthesize Processing
 * ======================================================================== */

static void process_synthesize(void)
{
    char name[256] = "";
    char ivar[256] = "";
    int c;

    skip_whitespace();
    read_identifier(name, sizeof(name));

    skip_whitespace();
    c = peek_char();
    if (c == '=') {
        read_char();
        skip_whitespace();
        read_identifier(ivar, sizeof(ivar));
    } else {
        snprintf(ivar, sizeof(ivar), "_%s", name);
    }

    skip_whitespace();
    c = peek_char();
    if (c == ';') read_char();

    {
        int i;
        for (i = 0; i < property_count; i++) {
            if (strcmp(properties[i].name, name) == 0 &&
                strcmp(properties[i].class_name, current_class) == 0) {

                const char *type = properties[i].type;
                BOOL is_readonly = strstr(properties[i].attrs, "readonly") != NULL;

                output("\n/* @synthesize %s = %s */\n", name, ivar);

                output("- (%s)%s {\n", type, name);
                output("    return self->%s;\n", ivar);
                output("}\n");

                if (!is_readonly) {
                    output("- (void)set%c%s:(%s)value {\n",
                           toupper((unsigned char)name[0]), name + 1, type);
                    output("    self->%s = value;\n", ivar);
                    output("}\n");
                }
                break;
            }
        }
    }
}

/* ========================================================================
 * @dynamic Processing
 * ======================================================================== */

static void process_dynamic(void)
{
    char name[256] = "";

    skip_whitespace();
    read_identifier(name, sizeof(name));

    skip_whitespace();
    {
        int c = peek_char();
        if (c == ';') read_char();
    }

    output("/* @dynamic %s - provided at runtime */\n", name);
}

/* ========================================================================
 * @class Forward Declaration
 * ======================================================================== */

static void process_class_forward(void)
{
    char name[256] = "";

    skip_whitespace();
    read_identifier(name, sizeof(name));

    skip_whitespace();
    {
        int c = peek_char();
        if (c == ';') read_char();
    }

    if (forward_class_count < MAX_CLASSES) {
        strncpy(forward_classes[forward_class_count], name, 127);
        forward_class_count++;
    }

    output("struct %s; /* forward class declaration */\n", name);
}

/* ========================================================================
 * @compatibility_alias Processing
 * ======================================================================== */

static void process_compatibility_alias(void)
{
    char alias[256] = "";
    char original[256] = "";

    skip_whitespace();
    read_identifier(alias, sizeof(alias));
    skip_whitespace();
    read_identifier(original, sizeof(original));

    skip_whitespace();
    {
        int c = peek_char();
        if (c == ';') read_char();
    }

    output("#define %s %s /* @compatibility_alias */\n", alias, original);
}

/* ========================================================================
 * @selector() Processing
 * ======================================================================== */

static void process_at_selector(void)
{
    char sel_name[512] = "";
    int c, i = 0;

    skip_whitespace();
    c = read_char();
    if (c == '(') {
        while ((c = read_char()) != EOF && c != ')' && i < (int)sizeof(sel_name) - 1) {
            sel_name[i++] = c;
        }
        sel_name[i] = '\0';
        while (i > 0 && (sel_name[i-1] == ' ' || sel_name[i-1] == '\t'))
            sel_name[--i] = '\0';
    }

    output("sel_registerName(\"%s\")", sel_name);
}

/* ========================================================================
 * @encode() Processing
 * ======================================================================== */

static void process_at_encode(void)
{
    char type[256] = "";
    int c, depth = 0, i = 0;

    skip_whitespace();
    c = read_char();
    if (c == '(') {
        depth = 1;
        while (depth > 0 && i < (int)sizeof(type) - 1) {
            c = read_char();
            if (c == '(') depth++;
            else if (c == ')') depth--;
            if (depth > 0) type[i++] = c;
        }
        type[i] = '\0';
    }

    output("\"%s\"", type);
}

/* ========================================================================
 * @autoreleasepool Processing
 * ======================================================================== */

static void process_at_autoreleasepool(void)
{
    int c;

    output("{ /* @autoreleasepool */\n");
    output("    void *_objc_apool = objc_autoreleasePoolPush();\n");

    skip_whitespace();
    c = read_char();
    if (c == '{') {
        int depth = 1;
        output("    {\n");
        while (depth > 0) {
            c = read_char();
            if (c == EOF) break;
            if (c == '{') depth++;
            else if (c == '}') depth--;
            if (depth > 0) output("    %c", c);
        }
        output("    }\n");
    }

    output("    objc_autoreleasePoolPop(_objc_apool);\n");
    output("}\n");
}

/* ========================================================================
 * Message Expression Processing [receiver method:args]
 * ======================================================================== */

static void process_message_expression(void)
{
    char buf[MAX_LINE];
    int c, depth = 1, i = 0;

    while (depth > 0 && i < (int)sizeof(buf) - 1) {
        c = read_char();
        if (c == EOF) break;
        if (c == '[') depth++;
        else if (c == ']') depth--;
        if (depth > 0) buf[i++] = c;
    }
    buf[i] = '\0';

    {
        char receiver[1024] = "";
        char selector[512] = "";
        char args[2048] = "";
        char *p = buf;
        int arg_idx = 0;

        while (*p == ' ' || *p == '\t' || *p == '\n') p++;

        i = 0;
        {
            int d = 0;
            while (*p) {
                if (*p == '[') d++;
                else if (*p == ']') d--;
                if (d == 0 && (*p == ' ' || *p == '\t' || *p == '\n' ||
                               *p == ':' || *p == ']'))
                    break;
                receiver[i++] = *p++;
            }
            receiver[i] = '\0';
        }

        while (*p == ' ' || *p == '\t' || *p == '\n') p++;

        while (*p) {
            if (*p == ':') {
                p++;
                strcat(selector, ":");
                arg_idx++;

                while (*p == ' ' || *p == '\t' || *p == '\n') p++;

                {
                    int d = 0;
                    char arg[512] = "";
                    int j = 0;
                    while (*p) {
                        if (*p == '[') d++;
                        else if (*p == ']') d--;
                        if (d == 0 && (*p == ' ' || *p == '\t' || *p == '\n' ||
                                       *p == ':' || *p == ']'))
                            break;
                        arg[j++] = *p++;
                    }
                    arg[j] = '\0';

                    if (args[0]) strcat(args, ", ");
                    strcat(args, arg);
                }
            } else if (*p == ' ' || *p == '\t' || *p == '\n') {
                p++;
            } else {
                char token[256] = "";
                int j = 0;
                while (*p && *p != ' ' && *p != '\t' && *p != '\n' &&
                       *p != ':' && *p != ']') {
                    token[j++] = *p++;
                }
                token[j] = '\0';
                strcat(selector, token);
            }
        }

        /* Validate: is this actually an ObjC message expression?
         * Heuristic: receiver must be a valid identifier (not a number),
         * and there must be either ':' or an identifier after it. */
        {
            int is_objc_msg = 1;
            const char *rp;
            char *sp;

            /* Receiver must be a non-empty identifier (not starting with digit) */
            if (receiver[0] == '\0' || isdigit((unsigned char)receiver[0])) {
                is_objc_msg = 0;
            }
            /* Receiver must not contain operators */
            for (rp = receiver; *rp; rp++) {
                if (!isalnum((unsigned char)*rp) && *rp != '_' && *rp != '.' &&
                    *rp != '-' && *rp != '>' && *rp != '[' && *rp != ']') {
                    is_objc_msg = 0;
                    break;
                }
            }
            /* Must have a non-empty selector */
            if (selector[0] == '\0') {
                is_objc_msg = 0;
            }
            /* Selector should not start with a digit (C expression leftover) */
            if (selector[0] && isdigit((unsigned char)selector[0])) {
                is_objc_msg = 0;
            }
            /* If selector contains only digits and operators, it's C */
            for (sp = selector; *sp; sp++) {
                if (isalpha((unsigned char)*sp) || *sp == '_') break;
            }
            if (*sp == '\0' && arg_idx == 0) {
                /* Selector is all digits/operators - C array subscript */
                is_objc_msg = 0;
            }

            if (!is_objc_msg) {
                output("[%s", buf);
                output("]");
                return;
            }
        }

        if (strcmp(receiver, "super") == 0) {
            output("({ struct objc_super _super; _super.receiver = self; _super.super_class = class_getSuperclass(objc_getClass(\"%s\")); objc_msgSendSuper(&_super, sel_registerName(\"%s\")%s%s); })",
                   current_class,
                   selector,
                   args[0] ? ", " : "",
                   args);
        } else {
            const char *rcv = receiver;
            /* If receiver starts with uppercase, it's a class name - use objc_getClass */
            if (isupper((unsigned char)receiver[0])) {
                static char class_get_buf[1024];
                snprintf(class_get_buf, sizeof(class_get_buf),
                         "(id)objc_getClass(\"%s\")", receiver);
                rcv = class_get_buf;
            }
            if (arg_idx == 0) {
                output("objc_msgSend(%s, sel_registerName(\"%s\"))",
                       rcv, selector);
            } else {
                output("objc_msgSend(%s, sel_registerName(\"%s\"), %s)",
                       rcv, selector, args);
            }
        }
    }
}

/* ========================================================================
 * Main Processing Loop
 * ======================================================================== */

static void process_source(void)
{
    int c;

    while ((c = read_char()) != EOF) {
        switch (c) {
        case '#': {
            char directive[64];
            int peek_c = peek_char();

            if (isalpha(peek_c) || peek_c == '_') {
                read_identifier(directive, sizeof(directive));

                if (strcmp(directive, "import") == 0 ||
                    strcmp(directive, "include") == 0) {
                    process_import();
                } else if (strcmp(directive, "objc") == 0) {
                    process_objc_directive();
                } else {
                    output("#%s", directive);
                    while ((c = read_char()) != EOF && c != '\n') {
                        output("%c", c);
                    }
                    output("\n");
                }
            } else {
                output("#");
                while ((c = read_char()) != EOF && c != '\n') {
                    output("%c", c);
                }
                output("\n");
            }
            break;
        }

        case '@': {
            char keyword[64];
            read_identifier(keyword, sizeof(keyword));

            if (strcmp(keyword, "interface") == 0) {
                process_interface();
            } else if (strcmp(keyword, "implementation") == 0) {
                process_implementation();
            } else if (strcmp(keyword, "end") == 0) {
                output("\n/* @end */\n");
                if (in_implementation) {
                    if (category_name[0]) {
                        output("/* @end %s (%s) */\n",
                               current_class, category_name);
                        category_name[0] = '\0';
                        category_class[0] = '\0';
                    } else {
                        output("/* @end %s */\n", current_class);
                    }
                }
                in_interface = NO;
                in_implementation = NO;
                in_protocol = NO;
                in_category = NO;
                current_class[0] = '\0';
            } else if (strcmp(keyword, "protocol") == 0) {
                process_protocol();
            } else if (strcmp(keyword, "property") == 0) {
                process_property();
            } else if (strcmp(keyword, "synthesize") == 0) {
                process_synthesize();
            } else if (strcmp(keyword, "dynamic") == 0) {
                process_dynamic();
            } else if (strcmp(keyword, "class") == 0) {
                process_class_forward();
            } else if (strcmp(keyword, "compatibility_alias") == 0) {
                process_compatibility_alias();
            } else if (strcmp(keyword, "selector") == 0) {
                process_at_selector();
            } else if (strcmp(keyword, "encode") == 0) {
                process_at_encode();
            } else if (strcmp(keyword, "autoreleasepool") == 0) {
                process_at_autoreleasepool();
            } else if (strcmp(keyword, "try") == 0) {
                output("/* @try */ { /* TODO: exception handling */");
            } else if (strcmp(keyword, "catch") == 0) {
                output("} /* @catch */ {");
            } else if (strcmp(keyword, "finally") == 0) {
                output("} /* @finally */ {");
            } else if (strcmp(keyword, "throw") == 0) {
                output("objc_exception_throw(");
            } else if (strcmp(keyword, "public") == 0) {
                output("    /* @public */");
            } else if (strcmp(keyword, "protected") == 0) {
                output("    /* @protected */");
            } else if (strcmp(keyword, "private") == 0) {
                output("    /* @private */");
            } else if (strcmp(keyword, "package") == 0) {
                output("    /* @package */");
            } else if (strcmp(keyword, "optional") == 0) {
                output("    /* @optional */");
            } else if (strcmp(keyword, "required") == 0) {
                output("    /* @required */");
            } else {
                output("@%s", keyword);
            }
            break;
        }

        case '[': {
            process_message_expression();
            break;
        }

        case ']': {
            output("]");
            break;
        }

        case '+': {
            int peek_c;
            skip_whitespace();
            peek_c = peek_char();
            if (in_implementation && (isalpha(peek_c) || peek_c == '(')) {
                process_method_definition(YES);
            } else if ((in_interface || in_protocol) &&
                       (isalpha(peek_c) || peek_c == '(')) {
                process_method_declaration(YES);
            } else {
                output("+");
            }
            break;
        }

        case '-': {
            int peek_c;
            skip_whitespace();
            peek_c = peek_char();
            if (in_implementation && (isalpha(peek_c) || peek_c == '(')) {
                process_method_definition(NO);
            } else if ((in_interface || in_protocol) &&
                       (isalpha(peek_c) || peek_c == '(')) {
                process_method_declaration(NO);
            } else {
                output("-");
            }
            break;
        }

        case '/': {
            int peek_c = peek_char();
            if (peek_c == '*') {
                /* Block comment: pass through verbatim */
                output("/*");
                read_char(); /* consume '*' */
                while ((c = read_char()) != EOF) {
                    if (c == '*') {
                        int next = peek_char();
                        if (next == '/') {
                            output("*/");
                            read_char(); /* consume '/' */
                            break;
                        }
                        output("*");
                    } else {
                        output("%c", c);
                    }
                }
            } else if (peek_c == '/') {
                /* Line comment: pass through verbatim */
                output("//");
                read_char(); /* consume second '/' */
                while ((c = read_char()) != EOF && c != '\n') {
                    output("%c", c);
                }
                if (c == '\n') output("\n");
            } else {
                output("/");
            }
            break;
        }

        default: {
            output("%c", c);
            break;
        }
        }
    }
}

/* ========================================================================
 * Usage and Main
 * ======================================================================== */

static void usage(const char *prog)
{
    fprintf(stderr,
        "MINSTEP Objective-C v%s\n"
        "Usage: %s [options] <input.m> [...]\n"
        "Options:\n"
        "  -h, --help      Show this help message\n"
        "  -v, --version   Show version\n"
        "  -E              Preprocess only (output C to stdout or -o file)\n"
        "  -c              Compile to object file (.o)\n"
        "  -S              Compile to assembly (.s)\n"
        "  -o <file>       Output file\n"
        "  -I <dir>        Add include search path (passed to C compiler)\n"
        "  -D <macro>[=val] Define macro (passed to C compiler)\n"
        "  -L <dir>        Add library search path (passed to linker)\n"
        "  -l <lib>        Link library (passed to linker)\n"
        "  -W <flag>       Pass warning flag to C compiler\n"
        "  -std=<std>      Pass standard flag to C compiler\n"
        "  -g              Include debug symbols\n"
        "  -O<level>       Optimization level (passed to C compiler)\n"
        "  -f<flag>        Pass compiler flag to C compiler\n"
        "\n"
        "Modes:\n"
        "  %s -E file.m              Preprocess to C (stdout)\n"
        "  %s -c file.m              Preprocess + compile to file.o\n"
        "  %s file.m -o program      Preprocess + compile + link to program\n"
        "  %s file.m                 Preprocess + compile to file.o\n",
        VERSION, prog, prog, prog, prog, prog);
}

static void version_info(void)
{
    fprintf(stderr, "MinSTEP Objective-C Preprocessor v%s\n", VERSION);
}

int main(int argc, char *argv[])
{
    const char *input_path = NULL;
    const char *output_path = NULL;
    const char *cc = NULL;        /* C compiler path */
    const char *cc_flags = NULL;  /* extra flags for C compiler */
    int mode_preprocess_only = 0; /* -E */
    int mode_compile_only = 1;    /* -c (default: compile to .o) */
    int mode_assemble = 0;        /* -S */
    int want_debug = 0;           /* -g */
    int i;
    const char *prog = argv[0];
    char tmpfile_path[1024];
    char compiler_cmd[4096];
    int ret;

    property_count = 0;
    forward_class_count = 0;

    /* Find C compiler */
    cc = getenv("OBJCC_CC");
    if (!cc) cc = getenv("CC");
    if (!cc) cc = "gcc";

    /* Parse arguments */
    for (i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            if (!input_path) {
                input_path = argv[i];
            } else {
                /* Extra argument - pass to linker */
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(prog);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            version_info();
            return 0;
        } else if (strcmp(argv[i], "-E") == 0) {
            mode_preprocess_only = 1;
            mode_compile_only = 0;
        } else if (strcmp(argv[i], "-c") == 0) {
            mode_compile_only = 1;
            mode_preprocess_only = 0;
        } else if (strcmp(argv[i], "-S") == 0) {
            mode_assemble = 1;
            mode_compile_only = 0;
            mode_preprocess_only = 0;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_path = argv[++i];
        } else if (strcmp(argv[i], "-g") == 0) {
            want_debug = 1;
        } else if (strncmp(argv[i], "-std=", 5) == 0 ||
                   strncmp(argv[i], "-O", 2) == 0 ||
                   strncmp(argv[i], "-f", 2) == 0 ||
                   strncmp(argv[i], "-W", 2) == 0) {
            /* Accumulate flags for C compiler */
            char *new_flags;
            size_t old_len = cc_flags ? strlen(cc_flags) : 0;
            size_t add_len = strlen(argv[i]) + 2;
            new_flags = (char *)malloc(old_len + add_len + 1);
            if (old_len > 0) {
                snprintf(new_flags, old_len + add_len + 1, "%s %s",
                         cc_flags, argv[i]);
                free((void *)cc_flags);
            } else {
                snprintf(new_flags, add_len + 1, "%s", argv[i]);
            }
            cc_flags = new_flags;
        } else if (strcmp(argv[i], "-I") == 0 && i + 1 < argc) {
            char *new_flags;
            size_t old_len = cc_flags ? strlen(cc_flags) : 0;
            size_t add_len = strlen(argv[i]) + strlen(argv[i+1]) + 8;
            new_flags = (char *)malloc(old_len + add_len + 1);
            if (old_len > 0)
                snprintf(new_flags, old_len + add_len + 1, "%s -I%s",
                         cc_flags, argv[i+1]);
            else
                snprintf(new_flags, add_len + 1, "-I%s", argv[i+1]);
            if (cc_flags) free((void *)cc_flags);
            cc_flags = new_flags;
            /* Also add to preprocessor include path search list */
            if (include_path_count < MAX_INCLUDE_PATHS) {
                strncpy(include_paths[include_path_count], argv[i+1], 511);
                include_paths[include_path_count][511] = '\0';
                include_path_count++;
            }
            i++;
        } else if (strcmp(argv[i], "-D") == 0 && i + 1 < argc) {
            char *new_flags;
            size_t old_len = cc_flags ? strlen(cc_flags) : 0;
            size_t add_len = strlen(argv[i]) + strlen(argv[i+1]) + 8;
            new_flags = (char *)malloc(old_len + add_len + 1);
            if (old_len > 0)
                snprintf(new_flags, old_len + add_len + 1, "%s -D%s",
                         cc_flags, argv[i+1]);
            else
                snprintf(new_flags, add_len + 1, "-D%s", argv[i+1]);
            if (cc_flags) free((void *)cc_flags);
            cc_flags = new_flags;
            i++;
        } else if (strcmp(argv[i], "-L") == 0 && i + 1 < argc) {
            char *new_flags;
            size_t old_len = cc_flags ? strlen(cc_flags) : 0;
            size_t add_len = strlen(argv[i]) + strlen(argv[i+1]) + 8;
            new_flags = (char *)malloc(old_len + add_len + 1);
            if (old_len > 0)
                snprintf(new_flags, old_len + add_len + 1, "%s -L%s",
                         cc_flags, argv[i+1]);
            else
                snprintf(new_flags, add_len + 1, "-L%s", argv[i+1]);
            if (cc_flags) free((void *)cc_flags);
            cc_flags = new_flags;
            i++;
        } else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
            char *new_flags;
            size_t old_len = cc_flags ? strlen(cc_flags) : 0;
            size_t add_len = strlen(argv[i]) + strlen(argv[i+1]) + 8;
            new_flags = (char *)malloc(old_len + add_len + 1);
            if (old_len > 0)
                snprintf(new_flags, old_len + add_len + 1, "%s -l%s",
                         cc_flags, argv[i+1]);
            else
                snprintf(new_flags, add_len + 1, "-l%s", argv[i+1]);
            if (cc_flags) free((void *)cc_flags);
            cc_flags = new_flags;
            i++;
        } else {
            fprintf(stderr, "%s: unknown option '%s'\n", prog, argv[i]);
            usage(prog);
            return 1;
        }
    }

    if (!input_path) {
        fprintf(stderr, "%s: no input file\n", prog);
        usage(prog);
        return 1;
    }

    /* If no -c, -E, or -S given, infer mode from output extension */
    if (!mode_preprocess_only && !mode_assemble) {
        if (!output_path) {
            /* No output specified: compile to .o (same name as input) */
            mode_compile_only = 1;
        } else {
            /* Check output extension */
            const char *ext = strrchr(output_path, '.');
            if (ext && strcmp(ext, ".o") == 0) {
                mode_compile_only = 1;
            } else if (ext && strcmp(ext, ".s") == 0) {
                mode_assemble = 1;
            } else if (ext && strcmp(ext, ".c") == 0) {
                mode_preprocess_only = 1;
            } else {
                /* Assume linking to executable */
                mode_compile_only = 0;
            }
        }
    }

    /* ---- Step 1: Preprocess .m → .c ---- */

    /* Generate temp C file path */
    {
        const char *base;
        const char *slash;
        slash = strrchr(input_path, '/');
        base = slash ? slash + 1 : input_path;
        snprintf(tmpfile_path, sizeof(tmpfile_path), "/tmp/objc_%s.c", base);
    }

    /* Open input */
    input_file = fopen(input_path, "r");
    if (!input_file) {
        fprintf(stderr, "%s: cannot open '%s'\n", prog, input_path);
        return 1;
    }

    input_filename = input_path;
    line_number = 1;

    /* Open temp C output */
    output_file = fopen(tmpfile_path, "w");
    if (!output_file) {
        fprintf(stderr, "%s: cannot create temp file '%s': %s\n",
                prog, tmpfile_path, strerror(errno));
        fclose(input_file);
        return 1;
    }

    output("/* Generated by MinSTEP objc preprocessor v%s */\n", VERSION);
    output("/* Input: %s */\n", input_path);
    output("#line 1 \"%s\"\n", input_path);

    process_source();

    if (input_file) fclose(input_file);
    fclose(output_file);
    output_file = NULL;

    /* ---- Step 2: Preprocess only? ---- */
    if (mode_preprocess_only) {
        if (output_path) {
            /* Copy temp file to output */
            FILE *fin, *fout;
            char buf[8192];
            size_t n;
            fin = fopen(tmpfile_path, "r");
            fout = fopen(output_path, "w");
            if (!fin || !fout) {
                fprintf(stderr, "%s: cannot copy to '%s'\n", prog, output_path);
                if (fin) fclose(fin);
                if (fout) fclose(fout);
                unlink(tmpfile_path);
                return 1;
            }
            while ((n = fread(buf, 1, sizeof(buf), fin)) > 0)
                fwrite(buf, 1, n, fout);
            fclose(fin);
            fclose(fout);
        } else {
            /* Output to stdout */
            FILE *fin;
            char buf[8192];
            size_t n;
            fin = fopen(tmpfile_path, "r");
            if (fin) {
                while ((n = fread(buf, 1, sizeof(buf), fin)) > 0)
                    fwrite(buf, 1, n, stdout);
                fclose(fin);
            }
        }
        unlink(tmpfile_path);
        if (cc_flags) free((void *)cc_flags);
        return 0;
    }

    /* ---- Step 3: Compile / Link ---- */

    /* Build compiler command */
    {
        char *p = compiler_cmd;
        int remaining = sizeof(compiler_cmd);
        int n;

        /* Compiler + flags */
        n = snprintf(p, remaining, "%s", cc);
        p += n; remaining -= n;

        if (want_debug) {
            n = snprintf(p, remaining, " -g");
            p += n; remaining -= n;
        }

        /* Suppress warnings that are normal in ObjC-to-C translation */
        n = snprintf(p, remaining, " -Wno-incompatible-pointer-types -Wno-int-conversion");
        p += n; remaining -= n;

        if (cc_flags) {
            n = snprintf(p, remaining, " %s", cc_flags);
            p += n; remaining -= n;
        }

        if (mode_assemble) {
            /* Compile to assembly */
            n = snprintf(p, remaining, " -S");
            p += n; remaining -= n;
            if (output_path) {
                n = snprintf(p, remaining, " -o %s", output_path);
                p += n; remaining -= n;
            }
        } else if (mode_compile_only) {
            /* Compile to .o */
            n = snprintf(p, remaining, " -c");
            p += n; remaining -= n;
            if (output_path) {
                n = snprintf(p, remaining, " -o %s", output_path);
                p += n; remaining -= n;
            }
        } else {
            /* Link to executable */
            if (output_path) {
                n = snprintf(p, remaining, " -o %s", output_path);
                p += n; remaining -= n;
            }
        }

        /* Input temp C file */
        n = snprintf(p, remaining, " %s", tmpfile_path);
        p += n; remaining -= n;
    }

    fprintf(stderr, "%s: %s\n", prog, compiler_cmd);
    ret = system(compiler_cmd);

    /* Clean up temp file */
    unlink(tmpfile_path);
    if (cc_flags) free((void *)cc_flags);

    return ret;
}
