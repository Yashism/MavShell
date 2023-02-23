/*
Name: Yash Waghmare
ID: 1001845079
*/

// The MIT License (MIT)
//
// Copyright (c) 2016 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n" // We want to split our command line up into tokens
                           // so we need to define what delimits our tokens.
                           // In this case  white space
                           // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 20 // Mav shell only supports four arguments

#define MAX_HISTORY 100

int main()
{

    char *command_string = (char *)malloc(MAX_COMMAND_SIZE);
    int history_count = 0;
    char history[MAX_HISTORY][256];
    int pid_list[20];
    int pid_count = 0;

    while (1)
    {
        // Print out the msh prompt
        printf("msh> ");

        // Read the command from the commandline.  The
        // maximum command that will be read is MAX_COMMAND_SIZE
        // This while command will wait here until the user
        // inputs something since fgets returns NULL when there
        // is no input
        while (!fgets(command_string, MAX_COMMAND_SIZE, stdin))
            ;

        /* Parse input */
        char *token[MAX_NUM_ARGUMENTS];

        int token_count = 0;
        char curr_dir[256];
        char save_dir[256];

        // int history_index = 0;

        // Pointer to point to the token
        // parsed by strsep
        char *argument_ptr;

        char *working_string = strdup(command_string);

        // we are going to move the working_string pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *head_ptr = working_string;

        // Tokenize the input strings with whitespace used as the delimiter
        while (((argument_ptr = strsep(&working_string, WHITESPACE)) != NULL) &&
               (token_count < MAX_NUM_ARGUMENTS))
        {
            token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
            if (strlen(token[token_count]) == 0)
            {
                token[token_count] = NULL;
            }
            token_count++;
        }

        // If a whitespace is entered it should print msh again
        if (*token == NULL)
        {
            continue;
        }
        else
        {
            // Copy the command to history
            strcpy(history[history_count], command_string);
            history_count++;
            strcpy(save_dir, getcwd(curr_dir, sizeof(curr_dir)));
        }

        // Make sure the command has upto 10 command line arguments
        if (token_count - 2 > 10)
        {
            printf("Too many arguments\n");
            continue;
        }

        if (!(strcmp(token[0], "cd")))
        {
            chdir(save_dir);
            if (token[1] == NULL)
            {
                printf("No directory specified\n");
                continue;
            }
            else if (!chdir(token[1]))
            {
                strcpy(save_dir, curr_dir);
                continue;
            }
            else
            {
                printf("Error occurred. Couldn't change directory\n");
                continue;
            }
        }
        pid_t pid = fork();
        if (!strcmp(token[0], "ls"))
        {
            chdir(save_dir);
            if (pid == 0)
            {
                execvp(token[0], token);
            }
            else
            {
                wait(NULL);
            }
        }
        else if (!(strcmp(*token, "quit")) || !(strcmp(*token, "exit")))
        {
            exit(0);
        }
        else if (*token[0] == '!')
        {
            // if (0 <= token[0][1] <= 14)
            // {
            //     //Run the nth command from history
            //     strcpy(command_string, history[token[0][1]]);
            //     printf("%s", command_string);
            //     continue;

            // }
            // else
            // {
            //     printf("%s ", history_count);
            // }
            if (pid == 0)
            {
                // Split the token and test other cases
                char *nstring;
                int n = token[0][1] - '0';
                printf("%d", n);
                if (n <= history_count)
                {
                    nstring = history[n];
                    nstring[strlen(nstring) - 1] = '\0';
                    execvp(nstring, token);
                    nstring[strlen(nstring)] = ' ';
                }
                else
                {
                    printf("Command not found in history\n");
                }
            }
            else
            {
                wait(NULL);
            }
        }
        else if (!strcmp(*token, "history"))
        {
            // Remove the extra space from the history of all commands
            for (int i = 0; i < history_count; i++)
            {
                history[i][strlen(history[i]) - 1] = '\0';
            }
            if (history_count > 15)
            {
                int start_count = history_count - 15;
                for (int i = start_count; i < history_count; i++)
                {
                    printf("%d: %s\n", i, history[i]);
                }
            }
            else
            {
                for (int i = 0; i < history_count; i++)
                {
                    printf("%d: %s\n", i, history[i]);
                }
            }
            for (int i = 0; i < history_count; i++)
            {
                history[i][strlen(history[i])] = ' ';
            }
        }
        else if (!strcmp(*token, "listpids"))
        {
            if (pid_count > 20)
            {
                int start_count = pid_count - 20;
                for (int i = start_count; i < pid_count; i++)
                {
                    printf("%d ", pid_list[i]);
                }
            }
            else
            {
                for (int i = 0; i < pid_count; i++)
                {
                    printf("%d ", pid_list[i]);
                }
            }
            printf("\n");
        }
        else
        {
            FILE *file;
            if (file = fopen(*token, "r"))
            {
                if (pid == 0)
                {
                    execvp(token[0], &token[0]);
                }
                else
                {
                    wait(NULL);
                }
                fclose(file);
                chdir(save_dir);
                continue;
            }
            else
            {
                chdir("/usr/local/bin/");
                if (file = fopen(*token, "r"))
                {
                    if (pid == 0)
                    {
                        execvp(token[0], &token[0]);
                    }
                    else
                    {
                        wait(NULL);
                    }
                    fclose(file);
                    chdir(save_dir);
                    continue;
                }
                else
                {
                    chdir("/usr/bin/");
                    if (file = fopen(*token, "r"))
                    {
                        if (pid == 0)
                        {
                            execvp(token[0], &token[0]);
                        }
                        else
                        {
                            wait(NULL);
                        }
                        fclose(file);
                        chdir(save_dir);
                        continue;
                    }
                    else
                    {
                        chdir("/bin/");
                        if (file = fopen(*token, "r"))
                        {
                            if (pid == 0)
                            {
                                chdir(save_dir);
                                execvp(token[0], &token[0]);
                            }
                            else
                            {
                                wait(NULL);
                            }
                            fclose(file);
                            chdir(save_dir);
                            continue;
                        }
                        else
                        {
                            printf("%s: Command not found.\n\n", *token);
                            chdir(save_dir);
                            continue;
                        }
                    }
                }
            }
        }

        // Now print the tokenized input as a debug check
        // \TODO Remove this code and replace with your shell functionality

        // int token_index = 0;
        // for (token_index = 0; token_index < token_count; token_index++)
        // {
        //     printf("token[%d] = %s\n", token_index, token[token_index]);
        // }

        free(head_ptr);
    }
    return 0;
    // e2520ca2-76f3-90d6-0242ac120003
}

// Make one fork
