#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void throw_error(const char* error) {
    perror(error);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    char BINARY_NAME[512] = "binary";
    char CC[512] = "gcc-14";
    char CFLAGS[512] = "-Wall -std=c23 -O0";  // TODO make -c not be optional
    char LDFLAGS[512] = "-Linclude";
    char MODULES[512] = "main";
    char freeform_buffer[2048];
    freeform_buffer[0] = '\0';
    int freeform_cur = 0;
    // read byggfile:

    FILE* fd = fopen("byggfile", "r");
    if (fd == NULL) {
        throw_error("Error opening byggfile");
    }
    char read_buffer[512];
    int line_len = 0;
    int line_no = 1;
    read_buffer[line_len] = fgetc(fd);
    char* target_var;
    // int read_cur;
    while (read_buffer[line_len] != EOF && read_buffer[line_len] != '\0') {
        while (read_buffer[line_len] != '\n' && read_buffer[line_len] != EOF &&
               read_buffer[line_len] != '\0') {
            line_len++;
            read_buffer[line_len] = fgetc(fd);
        }
        read_buffer[line_len] = '\0';
        int read_cur;
        if (strncmp("BINARY_NAME", read_buffer, strlen("BINARY_NAME")) == 0) {
            target_var = BINARY_NAME;
            read_cur = strlen("BINARY_NAME");
        } else if (strncmp("CC", read_buffer, strlen("CC")) == 0) {
            target_var = CC;
            read_cur = strlen("CC");
        } else if (strncmp("CFLAGS", read_buffer, strlen("CFLAGS")) == 0) {
            target_var = CFLAGS;
            read_cur = strlen("CFLAGS");
        } else if (strncmp("LDFLAGS", read_buffer, strlen("LDFLAGS")) == 0) {
            target_var = LDFLAGS;
            read_cur = strlen("LDFLAGS");
        } else if (strncmp("MODULES", read_buffer, strlen("MODULES")) == 0) {
            target_var = MODULES;
            read_cur = strlen("MODULES");
        } else {
            target_var = freeform_buffer;
            strncpy(&freeform_buffer[freeform_cur], read_buffer, line_len);
            freeform_cur += line_len;
            freeform_buffer[freeform_cur + 1] = '\0';
            read_cur = 0;
        }
        while (read_buffer[read_cur] == ' ') {
            read_cur++;
        }
        if (read_buffer[read_cur] != '=') {
            char error_buf[128];
            sprintf(error_buf, "Syntax error on line %i", line_no);
            throw_error(error_buf);
        }
        read_cur++;
        strcpy(target_var, &read_buffer[read_cur]);
        line_len = 0;
        line_no++;
        read_buffer[line_len] = fgetc(fd);
        while (read_buffer[line_len] == ' ' || read_buffer[line_len] == '\n') {
            read_buffer[line_len] = fgetc(fd);
        }
    }

    fclose(fd);

    // output Makefile:
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
            "CFLAGS=-c -MMD %s\n"
            "LDFLAGS=%s\n"
            "MODULES=%s\n"
            "%s\n"
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
            "build/%%.o : %%.c\n"
            "        $(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@\n"
            "\n"
            "clean:\n"
            "    rm -rf build\n"
            "\n"
            "-include $(OBJS:.o=.d)\n",
            BINARY_NAME, CC, CFLAGS, LDFLAGS, MODULES, freeform_buffer);
    fclose(fd);

    // chain-call make:

    // no args passed:
    if (argc <= 1) {
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