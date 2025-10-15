// translate_any_color_final.c
// Compile: gcc -std=gnu11 -o translate_any_color_final translate_any_color_final.c
// Run: ./translate_any_color_final

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_TEXT_LEN 4096
#define READ_BUF 4096
#define MAX_LANG_LEN 64

// ANSI bright color codes
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define DIM "\033[2m"
#define CYAN_B "\033[96m"
#define GREEN_B "\033[92m"
#define BLUE_B "\033[94m"
#define YELLOW_B "\033[93m"
#define MAGENTA_B "\033[95m"
#define WHITE_B "\033[97m"
#define GRAY "\033[90m"

void translate_text(const char *from_lang, const char *to_lang,
                    const char *text, int id)
{
    int inpipe[2], outpipe[2];
    if (pipe(inpipe) == -1 || pipe(outpipe) == -1)
    {
        perror("pipe");
        return;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return;
    }

    if (pid == 0)
    {
        dup2(inpipe[0], STDIN_FILENO);
        dup2(outpipe[1], STDOUT_FILENO);
        close(inpipe[0]);
        close(inpipe[1]);
        close(outpipe[0]);
        close(outpipe[1]);
        execlp("ollama", "ollama", "run", "gemma3", (char *)NULL);
        perror("execlp");
        _exit(127);
    }

    close(inpipe[0]);
    close(outpipe[1]);

    char prompt[MAX_TEXT_LEN + 512];
    snprintf(prompt, sizeof(prompt),
             "Translate the following text from %s to %s. "
             "Respond with only the translation:\n%s\n",
             from_lang, to_lang, text);

    write(inpipe[1], prompt, strlen(prompt));
    close(inpipe[1]);

    char buf[READ_BUF];
    ssize_t r;
    size_t total = 0;
    char *output = NULL;

    while ((r = read(outpipe[0], buf, sizeof(buf))) > 0)
    {
        output = realloc(output, total + r + 1);
        memcpy(output + total, buf, r);
        total += r;
    }
    if (output)
        output[total] = '\0';
    close(outpipe[0]);
    waitpid(pid, NULL, 0);

    printf("\n%s%s=== Translation (%d) ===%s\n", BOLD, MAGENTA_B, id, RESET);
    if (output && total > 0)
    {
        char *start = output;
        while (*start && strchr("\n\r \t", *start))
            start++;
        char *end = output + total - 1;
        while (end > start && strchr("\n\r \t", *end))
            *end-- = '\0';
        printf("%s%s%s%s\n", BOLD, WHITE_B, start, RESET);
    }
    else
    {
        printf("%s(no output)%s\n", GRAY, RESET);
    }
    printf("%s=========================%s\n\n", MAGENTA_B, RESET);

    free(output);
}

void show_prompt(const char *from_lang, const char *to_lang)
{
    printf("%s[%s%s%s → %s%s%s]%s\n",
           BOLD, GREEN_B, from_lang, RESET, CYAN_B, to_lang, RESET, RESET);
    printf("%s%sText>%s ", BOLD, YELLOW_B, RESET);
    fflush(stdout);
}

int main(void)
{
    printf("%s%s🌍 Universal Translator%s\n", CYAN_B, BOLD, RESET);
    printf("%sType any text to translate.%s\n", GRAY, RESET);
    printf("%sCommands:%s\n", BLUE_B, RESET);
    printf("  %s/c%s → change source & target languages\n", BLUE_B, RESET);
    printf("  %s/e%s → quit the program\n\n", BLUE_B, RESET);

    char from_lang[MAX_LANG_LEN] = "English";
    char to_lang[MAX_LANG_LEN] = "Arabic";
    char line[MAX_TEXT_LEN];
    int id = 1;

    printf("%sDefault translation:%s %s%s → %s%s%s\n\n",
           GRAY, RESET, GREEN_B, from_lang, CYAN_B, to_lang, RESET);
    show_prompt(from_lang, to_lang);

    while (fgets(line, sizeof(line), stdin))
    {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        if (strcmp(line, "/e") == 0)
            break;

        if (strcmp(line, "/c") == 0)
        {
            char new_from[MAX_LANG_LEN];
            char new_to[MAX_LANG_LEN];

            printf("%sFrom language (%s): %s", BLUE_B, from_lang, RESET);
            fflush(stdout);
            if (fgets(new_from, sizeof(new_from), stdin))
            {
                len = strlen(new_from);
                if (len > 0 && new_from[len - 1] == '\n')
                    new_from[len - 1] = '\0';
                if (strlen(new_from) > 0)
                    strncpy(from_lang, new_from, MAX_LANG_LEN);
            }

            printf("%sTo language (%s): %s", BLUE_B, to_lang, RESET);
            fflush(stdout);
            if (fgets(new_to, sizeof(new_to), stdin))
            {
                len = strlen(new_to);
                if (len > 0 && new_to[len - 1] == '\n')
                    new_to[len - 1] = '\0';
                if (strlen(new_to) > 0)
                    strncpy(to_lang, new_to, MAX_LANG_LEN);
            }

            printf("\n%sLanguages updated:%s %s%s → %s%s%s\n\n",
                   GRAY, RESET, GREEN_B, from_lang, CYAN_B, to_lang, RESET);
            show_prompt(from_lang, to_lang);
            continue;
        }

        if (strlen(line) == 0)
        {
            show_prompt(from_lang, to_lang);
            continue;
        }

        translate_text(from_lang, to_lang, line, id++);
        show_prompt(from_lang, to_lang);
    }

    printf("\n%sGoodbye.%s\n", DIM, RESET);
    return 0;
}