#include<linux/init.h>
#include<linux/module.h>

#include<linux/fs.h>
#include<linux/sched.h>
#include<asm/current.h>
#include<asm/uaccess.h>
#include<linux/uaccess.h>

#include<linux/kmod.h>
#include<linux/kernel.h>

#define COMMAND_SIZE 1024
#define FILE_BUFFER_SIZE 1024*10

// #include"file_operations.h"
#include"helpers.h"

char command_buffer[COMMAND_SIZE];

int openCount = 0;
int closeCount = 0;
int placeholder = 0;
long int output_size = 0;

char __user  *user_buffers[1024];



ssize_t simple_char_driver_read (struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
	/* pfile = driver file
	 * buffer = buffer in user space
	 * length = length of what is to be read (using cat will give something like 6000+ bytes)
	 * offset = variable to store offset for ONE read cycle.
	 */

	unsigned int readed, toread;
    struct file *file_to_read;
    static char file_buffer[FILE_BUFFER_SIZE];

    printk(KERN_ALERT "READING Simple Character Driver pfile=%p, buffer=%p, lenght=%ld, offset=%lld\n", pfile, buffer, length, *offset);

    file_to_read = find_or_open_file_for_buffer(buffer, command_buffer);
    if (!file_to_read) {
      printk(KERN_ALERT "Can't find nor open file for buffer=%p, Maybe to many files are opened\n", buffer);
      return 0;
      }
	
    
//	bytesToRead = output_size - *offset;
//    br = bytesToRead>length?length:bytesToRead;
//    read_pointer = offset;
    readed = file_read(file_to_read, *offset, file_buffer, FILE_BUFFER_SIZE>length?length:FILE_BUFFER_SIZE);
	printk(KERN_ALERT "Readed from fifo file %d bytes (offset=%d, asked=%d, FILE_BUFFER_SIZE=%d, length=%d)", readed, *offset, FILE_BUFFER_SIZE>length?length:FILE_BUFFER_SIZE, FILE_BUFFER_SIZE,length);
//    file_close(file_to_read);
    
	// If we are at the end of the file, STOP READING!
//	if (readed == 0){
//		printk(KERN_ALERT "Reached the end of the file");
//        destroy_file_for_buffer(buffer);
//        return cleanup();
//	}

	// Get bytes read by subtracting return of copy_to_user (returns unread bytes)
	toread = copy_to_user(buffer, file_buffer, readed);
	printk(KERN_ALERT "READ with Simple Character Driver. toread=%d bytes\n", toread);

	// Set offset so that we can eventually reach the end of the file
	*offset += (readed-toread);
    if (readed == toread) {
        destroy_file_for_buffer(buffer);
      }

	return readed-toread;
}



ssize_t simple_char_driver_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
	/* pfile = driver file
	 * buffer = buffer in user space
	 * length = length of what is to be written
	 * offset = variable to store offset for ONE write
	 * placeholder = global var for the current write spot in the buffer
	 */

	int bytesToWrite;
	int bytesWritten;
	int bytesAvailable = COMMAND_SIZE - *offset - placeholder;
//    if (bytesAvailable<1) {
//		 printk(KERN_ALERT "The device is out of space COMMAND_SIZE=%d offset=%d placeholder=%d", COMMAND_SIZE, *offset, placeholder);
//         sprintf(command_buffer, "echo 'command is to long (max %d bytes)'", COMMAND_SIZE);
//	     return 0;
//        }

	// Make sure there is sufficient space
	if(bytesAvailable > length){
		bytesToWrite = length; 
	}
	else{
		bytesToWrite = bytesAvailable;
	}

	//Get bites written by subtracting unwritten bites from retun of copy_from_user
	bytesWritten = bytesToWrite - copy_from_user(command_buffer + *offset + placeholder, buffer, bytesToWrite);
	
	// If no space left:
	if(bytesWritten == 0){
		printk(KERN_ALERT "The device is out of space.\n");
	}
	else{
		//Increment offset and placeholder
		*offset += bytesWritten;
		placeholder += bytesWritten;

		printk(KERN_ALERT "WRITING with Simple Character Driver. Writing %d bytes\n", bytesWritten);
	}
	return bytesWritten;
}


int simple_char_driver_open (struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is opened and also print the number of times this device has been opened until now*/
	openCount++;
/*	printk(KERN_ALERT "OPENING Simple Character Driver. It has been opened %d times\n", openCount);
	printk(KERN_ALERT "OPENING with flags: %s\n", byte_to_binary(pfile->f_flags));
    printk(KERN_ALERT "OPENING with flag O_RDONLY: %d\n", (pfile->f_flags & O_RDONLY) ? 1: 0);
    printk(KERN_ALERT "OPENING with flag O_WRONLY: %d\n", (pfile->f_flags & O_WRONLY) ? 1: 0);
    printk(KERN_ALERT "OPENING with flag O_RDWR: %d\n", (pfile->f_flags & O_RDWR) ? 1: 0);
    printk(KERN_ALERT "OPENING with flag O_CREAT: %d\n", (pfile->f_flags & O_CREAT) ? 1: 0);
    printk(KERN_ALERT "OPENING with flag O_TRUNC: %d\n", (pfile->f_flags & O_TRUNC) ? 1: 0);
    printk(KERN_ALERT "OPENING with flag O_APPEND: %d\n", (pfile->f_flags & O_APPEND) ? 1: 0);*/
    if (pfile->f_flags & (O_WRONLY | O_RDWR)) {
      if (pfile->f_flags & (O_TRUNC)) {
        printk(KERN_ALERT "TRUNCATE\n");
        placeholder = 0;
        memset(command_buffer, 0, sizeof command_buffer);
        }
      }
	return 0;
}


int simple_char_driver_close (struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is closed and also print the number of times this device has been closed until now*/
	closeCount++;
	printk(KERN_ALERT "CLOSING Simple Character Driver. It has been closed %d times\n", closeCount);
	return 0;
}

struct file_operations simple_char_driver_file_operations = {
	.owner   = THIS_MODULE,
	.read    = simple_char_driver_read,
	.write   = simple_char_driver_write,
	.open    = simple_char_driver_open,
	.release = simple_char_driver_close
};

typedef struct ts {
  int *one;
  int *two;
  } ts;

struct ts *tsa[10];

void printts(void) {
  int i;
  for (i=0; i<10; i++) {
    printk(KERN_ALERT "tsa[%d]=%p={%p->%i, %p->%i}", i, tsa[i], 
tsa[i]?tsa[i]->one:NULL, tsa[i]?*(tsa[i]->one):-1,
tsa[i]?tsa[i]->two:NULL, tsa[i]?*(tsa[i]->two):-1);
    }
}

void tts(int i, int k) {
   struct ts a;
   struct ts *pa;
   int o, t;
   o = k +1;
   t = k + 2;
//   printk(KERN_ALERT "po=%p, pt=%p", &o, &t);
   a = (struct ts){&o, &t};
   pa = &a;
    printk(KERN_ALERT "i=%d, pa=%p={%p->%d, %p->%d}", i, pa,
pa->one, *(pa->one),
pa->two, *(pa->two)
);
   
   tsa[i] = pa;
}


static int simple_char_driver_init(void)
{
	/* print to the log file that the init function is called.*/
	printk(KERN_ALERT "INITIALIZING Simple Character Driver!!!\n");
	/* register the device */
	register_chrdev( 303, "simple_driver", &simple_char_driver_file_operations);
	return 0;
}

// Return type must be void to avoid warning in compilation
static void simple_char_driver_exit(void)
{
	/* print to the log file that the exit function is called.*/
	printk(KERN_ALERT "EXITING Simple Character Driver!!!\n");
	/* unregister the device */
	unregister_chrdev( 303, "simple_driver");
}

/* add module_init and module_exit to point to the corresponding init and exit function*/

module_init(simple_char_driver_init);
module_exit(simple_char_driver_exit);
