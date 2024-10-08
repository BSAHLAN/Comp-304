#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <termios.h> // termios, TCSANOW, ECHO, ICANON
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/sched.h>

const char *sysname = "Shellect";

enum return_codes {
	SUCCESS = 0,
	EXIT = 1,
	UNKNOWN = 2,
};

struct command_t {
	char *name;
	bool background;
	bool auto_complete;
	int arg_count;
	char **args;
	char *redirects[3]; // in/out redirection
	struct command_t *next; // for piping
};

/**
 * Prints a command struct
 * @param struct command_t *
 */
void print_command(struct command_t *command) {
	int i = 0;
	printf("Command: <%s>\n", command->name);
	printf("\tIs Background: %s\n", command->background ? "yes" : "no");
	printf("\tNeeds Auto-complete: %s\n",
		   command->auto_complete ? "yes" : "no");
	printf("\tRedirects:\n");

	for (i = 0; i < 3; i++) {
		printf("\t\t%d: %s\n", i,
			   command->redirects[i] ? command->redirects[i] : "N/A");
	}

	printf("\tArguments (%d):\n", command->arg_count);

	for (i = 0; i < command->arg_count; ++i) {
		printf("\t\tArg %d: %s\n", i, command->args[i]);
	}

	if (command->next) {
		printf("\tPiped to:\n");
		print_command(command->next);
	}
}

/**
 * Release allocated memory of a command
 * @param  command [description]
 * @return         [description]
 */
int free_command(struct command_t *command) {
	if (command->arg_count) {
		/*for (int i = 0; i < command->arg_count; ++i)
			free(command->args[i]);*/
		free(command->args);
	}

	for (int i = 0; i < 3; ++i) {
		if (command->redirects[i])
			free(command->redirects[i]);
	}

	if (command->next) {
		free_command(command->next);
		command->next = NULL;
	}

	free(command->name);
	free(command);
	return 0;
}

/**
 * Show the command prompt
 * @return [description]
 */
int show_prompt() {
	char cwd[1024], hostname[1024];
	gethostname(hostname, sizeof(hostname));
	getcwd(cwd, sizeof(cwd));
	printf("%s@%s:%s %s$ ", getenv("USER"), hostname, cwd, sysname);
	return 0;
}
/*
function to add aliases to the aliases.txt
takes the name of the alias and the command
Return : none 
*/
void addAlias(char *aliasName, struct command_t *command) {

       char tmp_str[1000];
       FILE *aliasesFile = fopen("aliases.txt", "a"); 
       strcpy(tmp_str,command->args[2]);
	
	for(int i = 3 ; i < command->arg_count - 1;i++){
		strcat(tmp_str," ");
		strcat(tmp_str,command->args[i]);
	} 
    fprintf(aliasesFile, "%s=%s\n", aliasName, tmp_str);
	
    fclose(aliasesFile);
}
/*
checks if alias is in aliases.txt 
takes : aliasName 
retuns : pointer to the command that is the alias
*/
char* isAlias(char* aliasName) {

    
    FILE* aliasesFile = fopen("aliases.txt", "r");

   
    char line[100];
    char* command = NULL;
    char* tmp_buf;
    

    while (fgets(line, sizeof(line), aliasesFile) != NULL) {
        
        char buf[100];
	strcpy(buf, line);
        char* line_1 = strtok_r(buf, "=", &tmp_buf);
        //char* line_2 = strtok_r(NULL, "=", &tmp_buf);
       
        if (strcmp(aliasName, line_1) == 0) {
	      char *cmd;
              cmd = strchr(line, '=') + 1;
	      
	      cmd[strlen(cmd)-1] = '\0';
	      printf("cmd: %s\n", cmd);
              command = strdup(cmd);
              break;
        }

    }

    fclose(aliasesFile);
    return command;
    
}

/**
 * Parse a command string into a command struct
 * @param  buf     [description]
 * @param  command [description]
 * @return         0
 */
int parse_command(char *buf, struct command_t *command) {
	const char *splitters = " \t"; // split at whitespace
	int index, len;
	len = strlen(buf);

        char token_buf[100]; //a buff for saving a copy of buf for using future tokenization
        strcpy(token_buf, buf);
        strcat(token_buf, "\0");
	
	// trim left whitespace
	while (len > 0 && strchr(splitters, buf[0]) != NULL) {
		buf++;
		len--;
	}

	while (len > 0 && strchr(splitters, buf[len - 1]) != NULL) {
		// trim right whitespace
		buf[--len] = 0;
	}
	
	// auto-complete
	if (len > 0 && buf[len - 1] == '?') {
		command->auto_complete = true;
	}

	// background
	if (len > 0 && buf[len - 1] == '&') {
		command->background = true;
	}

	char *pch = strtok(buf, splitters);
	
	if (pch == NULL) {
		command->name = (char *)malloc(1);
		command->name[0] = 0;
	} else {
		command->name = (char *)malloc(strlen(pch) + 1);
		strcpy(command->name, pch);
	
	}
        	
        
	//printf("old_args[0]: %s\n", old_args[0]);
	command->args = (char **)malloc(sizeof(char *));

	int redirect_index;
	int arg_index = 0;
	char temp_buf[1024], *arg;

	char* alias = isAlias(command->name);

		
	if(alias != NULL){//handle the case of it existing
		 struct command_t *alias_command = malloc(sizeof(struct command_t));
		 
 			 if (parse_command(alias, alias_command) == 0){
                             	//getting the reamining arguments after alias
	                        char* token_save;
				
                            char* token;
                            char** old_args = (char**)malloc(sizeof(char*));
                                
			        token = strtok_r(token_buf, splitters, &token_save);

                                int old_args_idx = 0;
                                

	 			while(token != NULL){
				     
                    token = strtok_r(NULL, splitters, &token_save);
			         old_args = (char**)realloc(old_args, sizeof(char*) * (old_args_idx+1)); 	     
				     old_args[old_args_idx] = (char*)malloc(10);
				     old_args[old_args_idx] = token;
				     old_args_idx++;
				     //token = strtok_r(NULL, splitters, &token_save);
				}

			      
				free(command->name);
				command->name = alias_command->name;

                                //shifting args to left for eliminating the command name
				for(int i = 1; i < alias_command->arg_count; i++){
				   alias_command->args[i-1] = alias_command->args[i];
				}

			        alias_command->arg_count -= 2;

      			        int idx = 0;

		                while(old_args[idx] != NULL){
		                    alias_command->args = (char **)realloc(alias_command->args, sizeof(char *) * (alias_command->arg_count + 1));
                                    alias_command->args[alias_command->arg_count] = old_args[idx];
				    alias_command->arg_count++;
		                    idx++;
		                }
				command->args = alias_command->args;
				command->arg_count = alias_command->arg_count;
                                
				
			}
			arg_index = command->arg_count;
			//free(alias);
			}
	
	while (1) {
		// tokenize input on splitters
		pch = strtok(NULL, splitters);
		
		if (!pch)
			break;
		arg = temp_buf;
		strcpy(arg, pch);
		len = strlen(arg);

		// empty arg, go for next
		if (len == 0) {
			continue;
		}

		// trim left whitespace
		while (len > 0 && strchr(splitters, arg[0]) != NULL) {
			arg++;
			len--;
		}

		// trim right whitespace
		while (len > 0 && strchr(splitters, arg[len - 1]) != NULL) {
			arg[--len] = 0;
		}

		// empty arg, go for next
		if (len == 0) {
			continue;
		}

		// piping to another command
		if (strcmp(arg, "|") == 0) {
			struct command_t *c = malloc(sizeof(struct command_t));
			int l = strlen(pch);
			pch[l] = splitters[0]; // restore strtok termination
			index = 1;
			while (pch[index] == ' ' || pch[index] == '\t')
				index++; // skip whitespaces

			parse_command(pch + index, c);
			pch[l] = 0; // put back strtok termination
			command->next = c;
			continue;
		}

		// background process
		if (strcmp(arg, "&") == 0) {
			// handled before
			continue;
		}

		// handle input redirection
		redirect_index = -1;
		if (arg[0] == '<') {
			redirect_index = 0; // <
		}

		if (arg[0] == '>') {
			if (len > 1 && arg[1] == '>') {
				redirect_index = 2; //>
				arg++;
				len--;
			} else {
				redirect_index = 1;//>>
			}
		}

		if (redirect_index != -1) {
			command->redirects[redirect_index] = malloc(len);
			strcpy(command->redirects[redirect_index], arg + 1);
			continue;
		}

		// normal arguments
		if (len > 2 &&
			((arg[0] == '"' && arg[len - 1] == '"') ||
			 (arg[0] == '\'' && arg[len - 1] == '\''))) // quote wrapped arg
		{
			arg[--len] = 0;
			arg++;
		}

		command->args =
			(char **)realloc(command->args, sizeof(char *) * (arg_index + 1));

		command->args[arg_index] = (char *)malloc(len + 1);
		strcpy(command->args[arg_index++], arg);
	}
	command->arg_count = arg_index;

	// increase args size by 2
	command->args = (char **)realloc(
		command->args, sizeof(char *) * (command->arg_count += 2));

	// shift everything forward by 1
	for (int i = command->arg_count - 2; i > 0; --i) {
		command->args[i] = command->args[i - 1];
	}

	// set args[0] as a copy of name
	command->args[0] = strdup(command->name);

	// set args[arg_count-1] (last) to NULL
	command->args[command->arg_count - 1] = NULL;

        return 0;
		
}

void prompt_backspace() {
	putchar(8); // go back 1
	putchar(' '); // write empty over
	putchar(8); // go back 1 again
}

/**
 * Prompt a command from the user
 * @param  buf      [description]
 * @param  buf_size [description]
 * @return          [description]
 */
int prompt(struct command_t *command) {
	size_t index = 0;
	char c;
	char buf[4096];
	static char oldbuf[4096];

	// tcgetattr gets the parameters of the current terminal
	// STDIN_FILENO will tell tcgetattr that it should write the settings
	// of stdin to oldt
	static struct termios backup_termios, new_termios;
	tcgetattr(STDIN_FILENO, &backup_termios);
	new_termios = backup_termios;
	// ICANON normally takes care that one line at a time will be processed
	// that means it will return if it sees a "\n" or an EOF or an EOL
	new_termios.c_lflag &=
		~(ICANON |
		  ECHO); // Also disable automatic echo. We manually echo each char.
	// Those new settings will be set to STDIN
	// TCSANOW tells tcsetattr to change attributes immediately.
	tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

	show_prompt();
	buf[0] = 0;

	while (1) {
		c = getchar();
		// printf("Keycode: %u\n", c); // DEBUG: uncomment for debugging

		// handle tab
		if (c == 9) {
			buf[index++] = '?'; // autocomplete
			break;
		}

		// handle backspace
		if (c == 127) {
			if (index > 0) {
				prompt_backspace();
				index--;
			}
			continue;
		}

		if (c == 27 || c == 91 || c == 66 || c == 67 || c == 68) {
			continue;
		}

		// up arrow
		if (c == 65) {
			while (index > 0) {
				prompt_backspace();
				index--;
			}

			char tmpbuf[4096];
			printf("%s", oldbuf);
			strcpy(tmpbuf, buf);
			strcpy(buf, oldbuf);
			strcpy(oldbuf, tmpbuf);
			index += strlen(buf);
			continue;
		}

		putchar(c); // echo the character
		buf[index++] = c;
		if (index >= sizeof(buf) - 1)
			break;
		if (c == '\n') // enter key
			break;
		if (c == 4) // Ctrl+D
			return EXIT;
	}

	// trim newline from the end
	if (index > 0 && buf[index - 1] == '\n') {
		index--;
	}

	// null terminate string
	buf[index++] = '\0';

	strcpy(oldbuf, buf);

	parse_command(buf, command);

	print_command(command); // DEBUG: uncomment for debugging

	// restore the old settings
	tcsetattr(STDIN_FILENO, TCSANOW, &backup_termios);
	return SUCCESS;
}

int process_command(struct command_t *command);

int main() {
	while (1) {
		struct command_t *command = malloc(sizeof(struct command_t));

		// set all bytes to 0
		memset(command, 0, sizeof(struct command_t));

		int code;
		code = prompt(command);
		if (code == EXIT) {
			break;
		}

		code = process_command(command);
		if (code == EXIT) {
			break;
		}

		free_command(command);
	}

	printf("\n");
	return 0;
}

void scheduleGoodMorning(char *minutes_given, char *audioPath) {

    char cronJob[4000]; //main body of the cronjob
    char command[256]; //temp str 
    int minutes = atoi(minutes_given);
    
// getting current local time 
    time_t currentTime;
    struct tm *localTime;
    time(&currentTime);
    localTime = localtime(&currentTime); 
//get the time we will schedule
    int hour = localTime->tm_hour;
    int minute = (localTime->tm_min + minutes) % 60;
    int hourToRun = hour + (localTime->tm_min + minutes) / 60;
// build the command 
    snprintf(command, sizeof(command), "/usr/bin/mpg123 %s", audioPath);

    snprintf(cronJob, sizeof(cronJob), "%d %d * * * %s", minute, hourToRun, command);

   //create temp file that  we will give crontab and put latest cronjob
    FILE *cronFile = fopen("tmpcron", "w");
    if (cronFile != NULL) {
        fprintf(cronFile, "%s\n", cronJob);
        fclose(cronFile);
    }

    // get previos cronjobs and append to temp file 
    system("crontab -l >> tmpcron");

    printf("\nScheduled crontab\n");

    system("crontab tmpcron");// write crontab file 

   
    remove("tmpcron");//remove cronjob file
}

int process_command(struct command_t *command) {
	int r;

	if (strcmp(command->name, "") == 0) {
		return SUCCESS;
	}

	if (strcmp(command->name, "exit") == 0) {
		int is_module_loaded();	
		
		if(is_module_loaded() == 1){
		   system("sudo rmmod mymodule");
		}

		return EXIT;
	}

	if (strcmp(command->name, "cd") == 0) {
		if (command->arg_count > 0) {
			r = chdir(command->args[0]);
			if (r == -1) {
				printf("-%s: %s: %s\n", sysname, command->name,
					   strerror(errno));
			}

			return SUCCESS;
		}
	}

    if(strcmp(command->name, "xdd") == 0){
	   void hexdump_g(int group_size, char* input);
	   void hexdump();
	   
  
	   if(command->arg_count == 2){
	      hexdump();
	   }

	   else{
           int group_size = atoi(command->args[2]);
           char* input = command->args[3];
	
           hexdump_g(group_size, input);
           }	   
	}

	if (strcmp(command->name, "alias") == 0) {
		addAlias(command->args[1],command);

		return SUCCESS;
	}


	if(strcmp(command->name, "good_morning") == 0){
		
		scheduleGoodMorning(command->args[1],command->args[2]);
	// TODO: your implementation here
	}
	
	if(strcmp(command->name, "what_to_watch") == 0) {
	         system("python3 rand_anime.py");
	}

	if(strcmp(command->name, "psvis") == 0){
	    int is_module_loaded();
	    char cmd_str[256];
	    
	    if(is_module_loaded() == 1){
	       printf("Kernel is already loaded\n");
	    
	    }

	    else{
		system("sudo dmesg --clear");
                sprintf(cmd_str, "sudo insmod ./module/mymodule.ko root_pid=%s, file_name=%s", command->args[1], command->args[2]); 
	        system(cmd_str);
		system("dmesg > log.txt");
		system("python draw_tree.py");
		
	    }
  	  
	    }
		if(strcmp(command->name, "offensive_detect") == 0){

		char cmnd[100];
		
		if(command->arg_count == 4){
			
			strcpy(cmnd,"python twitter_offensive_detection/twitter_off.py ");
			strcat(cmnd,command->args[1]);
			strcat(cmnd," ");
			strcat(cmnd,command->args[2]);
			system(cmnd);

		}else if(command->arg_count == 3){
			
			strcpy(cmnd,"python twitter_offensive_detection/twitter_off.py ");
			strcat(cmnd,command->args[1]);
			system(cmnd);


		}else{
			printf("wrong number of arguments");

			}

	}
	

        pid_t pid = fork();
	// child
	if (pid == 0) {
		/// This shows how to do exec with environ (but is not available on MacOs)
		// extern char** environ; // environment variables
		// execvpe(command->name, command->args, environ); // exec+args+path+environ

		/// This shows how to do exec with auto-path resolve
		// add a NULL argument to the end of args, and the name to the beginning
		// as required by exec

		// TODO: do your own exec with path resolving using execv()
		// do so by rplacing the-> execvp call below
		char*  path  = getenv("PATH");
		const char *cmd = command->args[0]; 
		char full_path[1024];
		if (path != NULL) {
                   char *token = strtok(path, ":"); 

                   while (token != NULL) {
                   char full_path[1024]; 
                   snprintf(full_path, sizeof(full_path), "%s/%s", token, cmd);

                if (access(full_path, X_OK) == 0) {
                
                    break; 
                }

                 token = strtok(NULL, ":");
                }
		}
	
		
		//execv(full_path, command->args);
		if(command->redirects[0]!= NULL){

				int input_fd = open(command->redirects[0], O_RDONLY);

				if (input_fd == -1) {
					perror("Failed to open input file");
					exit(1);
				}

				
				if (dup2(input_fd, STDIN_FILENO) == -1) {
					perror("Failed to redirect stdin");
					exit(1);
				}
				
			
				close(input_fd);

		}
		if (command->redirects[1] != NULL && command->redirects[2] != NULL) {
			// Case: file1 > file2 >> file3 and file1 >> file2 > file3 which is actually the same thing
			int output_fd1;
			int output_fd2;
	
			output_fd1 = open(command->redirects[1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
			output_fd2 = open(command->redirects[2], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);

			if (output_fd1 == -1 || output_fd2 == -1) {
				perror("Failed to open output files");
				exit(1);
			}

			if (dup2(output_fd1, STDOUT_FILENO) == -1) {
				perror("Failed to redirect stdout");
				exit(1);
			}
			execv(full_path, command->args);
	
			if (dup2(output_fd2, STDOUT_FILENO) == -1) {
				perror("Failed to redirect stdout");
				exit(1);
			}
			close(output_fd1);
			close(output_fd2);
		}else if (command->redirects[1] != NULL) {
			// Case: file1 > file2
			int output_fd;
	
			output_fd = open(command->redirects[1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

			if (output_fd == -1) {
				perror("Failed to open output file");
				exit(1);
			}

			if (dup2(output_fd, STDOUT_FILENO) == -1) {
				perror("Failed to redirect stdout");
				exit(1);
			}
			execv(full_path, command->args);
			close(output_fd);
		}else if (command->redirects[2] != NULL) {
			// Case: file1 >> file2
			int output_fd;

			output_fd = open(command->redirects[2], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);

			if (output_fd == -1) {
				perror("Failed to open output file");
				exit(1);
			}

			if (dup2(output_fd, STDOUT_FILENO) == -1) {
				perror("Failed to redirect stdout");
				exit(1);
			}
			execv(full_path, command->args);
			close(output_fd);
		}
		
		else{
			execv(full_path, command->args);
		}

		// execvp(command->name, command->args); // exec+args+path
		exit(0);
	} else {
		// TODO: implement background processes here

		if(command->background == 1){

		}else{
		
		wait(0); // wait for child process to finish
		}
		return SUCCESS;
	}

	printf("-%s: %s: command not found\n", sysname, command->name);
	return UNKNOWN;
       
}

//hexdump with -g flag
void hexdump_g(int group_size, char* input){
  
    //printf("input: %s\n", input);
    bool is_file = true;
    int current_size = 0;
    int real_size = (16/group_size);
    int hex_counter = 0;
    FILE* file;

    if(access(input, F_OK) == 0){
         file = fopen(input, "r");
    }

    if(access(input, F_OK) == -1){
	// printf("is_file: false\n");
         is_file = false;
    }  

    if(is_file){

      int chr;
      
      while((chr = fgetc(file)) != EOF){
           if(current_size == real_size){
	     printf("\n");
	     current_size = 0;
	     hex_counter += real_size;
	   }

	   if(current_size == 0){
	      printf("%08X: ", hex_counter);
	   }
           
	   printf("%02X ", chr);
           current_size++;
      }
      fclose(file);
    }

    else if(!is_file){
        int len = strlen(input);
        int chr; 
        int idx = 0;

        while(idx < len){

	    if(current_size == real_size){
	     printf("\n");
	     current_size = 0;
	     hex_counter += real_size;
	    } 

             if(current_size == 0){
	      printf("%08X: ", hex_counter);
	    }

	   chr = input[idx];

           printf("%02X ", chr);
           current_size++;
	   idx++; 

	}	
    
    }

    printf("\n");
 
}

//hexdump without -g flag
void hexdump(){

     //char c;
     char buf[4096];
     //int hex_counter;
     //printf("here\n");
     while(1){

       //read(0, buf, 10);
       scanf("%s", buf);
       //printf("buf: %s\n", buf);
       //for testing starts
       if(strcmp(buf, "quit") == 0){
          break;
       }
       //testing ends       
       printf("%08X: ", 0); 
       for(int i = 0; i < (int)strlen(buf); i++){   
	   printf("%02X ", buf[i]);
        
       } 
       printf("\n");
     }
}

int is_module_loaded() {
    FILE *fp;
    char line[256];

    // Open /proc/modules for reading
    fp = fopen("/proc/modules", "r");
    if (fp == NULL) {
        perror("Error opening /proc/modules");
        exit(EXIT_FAILURE);
    }

    // Check each line in /proc/modules
    while (fgets(line, sizeof(line), fp) != NULL) {
        // Check if the module name is in the line
        if(strstr(line, "mymodule") != NULL) {
            // Module is loaded
            fclose(fp);
            return 1;
        }
    }

    // Module not found
    fclose(fp);
    return 0;
}
