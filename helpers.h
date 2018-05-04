#include<linux/slab.h>
#include"file_operations.h"
#define MAX_FILES_OPENED 10


typedef struct buffer_file {
   char __user *user_buffer;
   struct file *opened_file;
} buffer_file;


struct buffer_file *fifo_files[MAX_FILES_OPENED];

const char *byte_to_binary(int x)
{
    static char b[17];
    int z; 
    b[0] = '\0';

    for (z = 256*128; z > 0; z >>= 1)
    {   
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}

int run_command(char __user *buffer, char fifo_file_name[], char command_buffer[])
{

  struct file *file_to_execute;
  char command[512];
  char shell_file_name[128];


  char * bash_argv[] = { "/bin/sh",  "-c", "", NULL };
  char * bash_envp[] = { "HOME=/", "TERM=linux" , "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

  sprintf(shell_file_name, "/tmp/executor_shell_%p", buffer);
  sprintf(command, "cd /; /bin/sh %s > %s", shell_file_name, fifo_file_name);
  bash_argv[2] = command;

  file_to_execute = file_open(shell_file_name, O_WRONLY | O_CREAT, 0x777);
  if (file_to_execute == NULL) {
     printk("Cant open file %s for write\n",  shell_file_name);
     return 0;
     }

  file_write(file_to_execute, 0, command_buffer, strlen(command_buffer));
  file_close(file_to_execute);

  call_usermodehelper(bash_argv[0], bash_argv, bash_envp, UMH_WAIT_PROC);
  printk("Running %s\n", command);
  return 1;

}


void destroy_file_for_buffer(char __user *buffer) {
 int i=0;
  char command[512];
 
// printk("destroy_file_for_buffer buffer=%p", buffer);

 for (i=0; i<MAX_FILES_OPENED; i++) {
    if (fifo_files[i] && fifo_files[i]->user_buffer == buffer) {
      char * bash_argv[] = { "/bin/sh",  "-c", "", NULL };
      char * bash_envp[] = { "HOME=/tmp/", "TERM=linux" , "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };
      sprintf(command, "rm /tmp/executor_shell_%p; rm /tmp/executor_fifo_%p", buffer, buffer);
      bash_argv[2] = command;
      call_usermodehelper(bash_argv[0], bash_argv, bash_envp, UMH_WAIT_PROC);
      kfree(fifo_files[i]);
      fifo_files[i]=NULL;
      return;
      }
    }

}

/*void print_fifo_file(int i) {
    printk("%d %p", i, fifo_files[i]);
    printk("%p %p", fifo_files[i]?fifo_files[i]->user_buffer:NULL, fifo_files[i]?fifo_files[i]->opened_file:NULL);
    return;
}


void print_all_fifo_files(void) {
 int i;
 for (i=0; i<MAX_FILES_OPENED; i++) {
    print_fifo_file(i);
    }
  return;
}*/

struct file *find_or_open_file_for_buffer(char __user *buffer, char command_buffer[]) {

 int i=0;
 char file_name[512];

// printk("find_or_open_file_for_buffer buffer=%p, command_buffer=%s, fifo_files=%p\n", buffer, command_buffer, fifo_files);

 for (i=0; i<MAX_FILES_OPENED; i++) {
    if (fifo_files[i] && fifo_files[i]->user_buffer == buffer) {
      return fifo_files[i]->opened_file;
      }
    }

  for (i=0; i<MAX_FILES_OPENED; i++) {
   if (!fifo_files[i]) {
      sprintf(file_name, "/tmp/executor_fifo_%p", buffer);
      if (!run_command(buffer, file_name, command_buffer)) {
        return NULL;
        }
      fifo_files[i] = kmalloc(sizeof(buffer_file), GFP_KERNEL);
      fifo_files[i]->user_buffer = buffer;
      fifo_files[i]->opened_file = file_open(file_name, O_RDONLY, 0x444);
      return fifo_files[i]->opened_file;
      }
    }
  return NULL;
}


