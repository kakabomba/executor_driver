#include<linux/init.h>
#include<linux/module.h>

#include<linux/fs.h>
#include<linux/sched.h>
#include<asm/current.h>
#include<asm/uaccess.h>
#include<linux/uaccess.h>

#define COMMAND_SIZE 1024
#define FILE_BUFFER_SIZE 1024*10
#include"helpers.h"
#include"config.h"

char command_buffer[COMMAND_SIZE];
int placeholder = 0;
char __user  *user_buffers[1024];



ssize_t char_driver_read (struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
	/* pfile = driver file
	 * buffer = buffer in user space
	 * length = length of what is to be read (using cat will give something like 6000+ bytes)
	 * offset = variable to store offset for ONE read cycle.
	 */

	unsigned int readed, toread;
    struct file *file_to_read;
    static char file_buffer[FILE_BUFFER_SIZE];

    printk("READING executor Driver pfile=%p, buffer=%p, lenght=%ld, offset=%lld\n", pfile, buffer, length, *offset);

    file_to_read = find_or_open_file_for_buffer(buffer, command_buffer);
    if (!file_to_read) {
      printk("Can't find nor open file for buffer=%p, Maybe to many files are opened\n", buffer);
      return 0;
      }
	
    
//	bytesToRead = output_size - *offset;
    readed = file_read(file_to_read, *offset, file_buffer, FILE_BUFFER_SIZE>length?length:FILE_BUFFER_SIZE);
	printk("Readed from fifo file %d bytes (offset=%lld, asked=%ld, FILE_BUFFER_SIZE=%d, length=%ld)", readed, *offset, FILE_BUFFER_SIZE>length?length:FILE_BUFFER_SIZE, FILE_BUFFER_SIZE,length);
    
	// Get bytes read by subtracting return of copy_to_user (returns unread bytes)
	toread = copy_to_user(buffer, file_buffer, readed);
	printk("READ with executor Driver. toread=%d bytes\n", toread);

	// Set offset so that we can eventually reach the end of the file
	*offset += (readed-toread);
    if (readed == toread) {
        destroy_file_for_buffer(buffer);
      }

	return readed-toread;
}



ssize_t char_driver_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
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
		printk("The device is out of space.\n");
	}
	else{
		//Increment offset and placeholder
		*offset += bytesWritten;
		placeholder += bytesWritten;

		printk("WRITING with executor Driver. Writing %d bytes\n", bytesWritten);
	}
	return bytesWritten;
}


int char_driver_open (struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is opened and also print the number of times this device has been opened until now*/
    if (pfile->f_flags & (O_WRONLY | O_RDWR)) {
      if (pfile->f_flags & (O_TRUNC)) {
        printk("TRUNCATE\n");
        placeholder = 0;
        memset(command_buffer, 0, sizeof command_buffer);
        }
      }
	return 0;
}


int char_driver_close (struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is closed and also print the number of times this device has been closed until now*/
	printk("CLOSING executor Driver. It has been closed\n");
	return 0;
}

struct file_operations simple_char_driver_file_operations = {
	.owner   = THIS_MODULE,
	.read    = char_driver_read,
	.write   = char_driver_write,
	.open    = char_driver_open,
	.release = char_driver_close
};

static int char_driver_init(void)
{
	/* print to the log file that the init function is called.*/
	printk("INITIALIZING executor Driver!!!\n");
	/* register the device */
	register_chrdev( DEVICE_MAJOR_NUMBER, DEVICE_NAME, &simple_char_driver_file_operations);
	return 0;
}

// Return type must be void to avoid warning in compilation
static void char_driver_exit(void)
{
	/* print to the log file that the exit function is called.*/
	printk("EXITING executor Driver!!!\n");
	/* unregister the device */
	unregister_chrdev( DEVICE_MAJOR_NUMBER, DEVICE_NAME);
}

/* add module_init and module_exit to point to the corresponding init and exit function*/

module_init(char_driver_init);
module_exit(char_driver_exit);

