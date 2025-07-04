#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 1024

#define DEBUG_PRINT false

void throw_error(const char* error) {
    perror(error);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    char BINARY_NAME[MAX_LINE_SIZE] = "binary";
    char CC[MAX_LINE_SIZE] = "gcc-14";
    char CFLAGS[MAX_LINE_SIZE] = "-Wall -std=c23 -O0";
    char LDFLAGS[MAX_LINE_SIZE] = "-Llib";
    char SRC_PATH[MAX_LINE_SIZE] = "";
    char MODULES[MAX_LINE_SIZE] = "main";
    char* passthrough_buffer;
    int passthrough_size = 0;
    // passthrough_buffer[0] = '\0';
    // int freeform_cur = 0;

    // read byggfile:

    if (DEBUG_PRINT) printf("Opening byggfile\n");  // debug
    FILE* fd = fopen("byggfile", "r");
    if (fd == NULL) {
        throw_error("Error opening byggfile");
    }
    char line_buffer[MAX_LINE_SIZE];
    int line_len = 0;
    int line_no = 1;
    line_buffer[line_len] = fgetc(fd);
    // int read_cur;
    while (line_buffer[line_len] != EOF && line_buffer[line_len] != '\0') {
        if (DEBUG_PRINT) printf("Reading line #%i\n", line_no);  // debug
        if (line_len > MAX_LINE_SIZE) {
            char error_msg[512];
            snprintf(error_msg, 511, "Error: Line #%i too long", line_no);
            throw_error(error_msg);
        }
        while (line_buffer[line_len] != '\n' && line_buffer[line_len] != EOF &&
               line_buffer[line_len] != '\0') {
            line_len++;
            line_buffer[line_len] = fgetc(fd);
        }
        line_buffer[line_len] = '\0';
        int read_cur = 0;
        char* target_var = NULL;
        if (strncmp("BINARY_NAME", line_buffer, strlen("BINARY_NAME")) == 0 &&
            (line_buffer[strlen("BINARY_NAME")] == ' ' ||
             line_buffer[strlen("BINARY_NAME")] == '=')) {
            target_var = BINARY_NAME;
            read_cur = strlen("BINARY_NAME");
        } else if (strncmp("CC", line_buffer, strlen("CC")) == 0 &&
                   (line_buffer[strlen("CC")] == ' ' || line_buffer[strlen("CC")] == '=')) {
            target_var = CC;
            read_cur = strlen("CC");
        } else if (strncmp("CFLAGS", line_buffer, strlen("CFLAGS")) == 0 &&
                   (line_buffer[strlen("CFLAGS")] == ' ' || line_buffer[strlen("CFLAGS")] == '=')) {
            target_var = CFLAGS;
            read_cur = strlen("CFLAGS");
        } else if (strncmp("LDFLAGS", line_buffer, strlen("LDFLAGS")) == 0 &&
                   (line_buffer[strlen("LDFLAGS")] == ' ' ||
                    line_buffer[strlen("LDFLAGS")] == '=')) {
            target_var = LDFLAGS;
            read_cur = strlen("LDFLAGS");
        } else if (strncmp("SRC_PATH", line_buffer, strlen("SRC_PATH")) == 0 &&
                   (line_buffer[strlen("SRC_PATH")] == ' ' ||
                    line_buffer[strlen("SRC_PATH")] == '=')) {
            target_var = SRC_PATH;
            read_cur = strlen("SRC_PATH");
        } else if (strncmp("MODULES", line_buffer, strlen("MODULES")) == 0 &&
                   (line_buffer[strlen("MODULES")] == ' ' ||
                    line_buffer[strlen("MODULES")] == '=')) {
            target_var = MODULES;
            read_cur = strlen("MODULES");
        } else {
            int new_passthrough_size = passthrough_size + line_len + 1;
            char* new_passthrough_buffer = malloc(new_passthrough_size + 1);
            if (passthrough_size > 0) {
                strcpy(new_passthrough_buffer, passthrough_buffer);
                free(passthrough_buffer);
            }
            passthrough_buffer = new_passthrough_buffer;
            passthrough_size = new_passthrough_size;
            strcat(passthrough_buffer, line_buffer);
            strcat(passthrough_buffer, "\n");
        }
        if (target_var != NULL) {
            while (line_buffer[read_cur] == ' ') {
                read_cur++;
            }
            if (line_buffer[read_cur] != '=') {
                char error_buf[128];
                sprintf(error_buf, "Syntax error on line %i", line_no);
                throw_error(error_buf);
            }
            read_cur++;
            strcpy(target_var, &line_buffer[read_cur]);
        }
        line_len = 0;
        line_no++;
        line_buffer[line_len] = fgetc(fd);
        while (line_buffer[line_len] == '\n') {
            line_buffer[line_len] = fgetc(fd);
        }
    }

    fclose(fd);


    // output Makefile:
    if (DEBUG_PRINT) printf("Writing Makefile\n");  // debug
    fd = fopen("Makefile", "w");
    if (fd == NULL) {
        throw_error("Error creating Makefile");
    }
    fprintf(fd,
            "define .RECIPEPREFIX\n"
            " \n"
            "endef\n"
            "\n"
            "MAKEFLAGS=-j 8\n"
            "\n"
            "BINARY_NAME=%s\n"
            "CC=%s\n"
            "CFLAGS=%s\n"
            "LDFLAGS=%s\n"
            "SRC_PATH=%s\n"
            "MODULES=%s\n"
            "OBJS := $(foreach module,$(MODULES),build/$(module).o)\n"
            "\n"
            "ifeq ($(DEBUG),DEBUG)\n"
            "    CFLAGS+= -DDEBUG -g\n"
            "endif\n"
            "\n"
            "main: build build/$(BINARY_NAME)\n"
            "\n"
            "debug: clean main\n"
            "\n"
            "build:\n"
            "    mkdir build\n"
            "\n"
            "build/$(BINARY_NAME): $(OBJS)\n"
            "    $(CC) -o build/$(BINARY_NAME) $(OBJS) $(LDFLAGS)\n"
            "\n"
            "build/%%.o : $(SRC_PATH)%%.c\n"
            "        $(CC) -MMD -c $(CFLAGS) $(CPPFLAGS) $< -o $@\n"
            "\n"
            "clean:\n"
            "    rm -rf build\n"
            "\n"
            "%s\n"
            "\n"
            "-include $(OBJS:.o=.d)\n",
            BINARY_NAME, CC, CFLAGS, LDFLAGS, SRC_PATH, MODULES,
            (passthrough_size > 0) ? passthrough_buffer : "");
    fclose(fd);

    // chain-call make:

    if (DEBUG_PRINT) printf("Calling make\n");  // debug
    // no args passed:
    if (argc <= 1) {
        printf("Executing \"make\"\n");
        int make_result = system("make");
        if (make_result == -1) {
            throw_error("Failed to run make");
        }
        return make_result;
    }

    // passing on args to make:
    char command_buffer[768] = "make ";
    for (int idx = 1; idx < argc; idx++) {
        strcat(command_buffer, argv[idx]);
    }
    printf("Executing \"%s\"\n", command_buffer);
    fflush(NULL);
    int make_result = system(command_buffer);
    if (make_result == -1) {
        char error_buffer[sizeof(command_buffer) + 19];
        snprintf(error_buffer, sizeof(error_buffer), "Failed to run \"%s\"", command_buffer);
        throw_error(error_buffer);
    }
    return make_result;
}