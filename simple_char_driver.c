#include<linux/init.h>
#include<linux/module.h>

#include<linux/fs.h>
#include<linux/sched.h>
#include<asm/current.h>
#include<asm/uaccess.h>
#include<linux/uaccess.h>

#include <linux/kmod.h>
#include<linux/kernel.h>

#define COMMAND_SIZE 1024*10
#define OUTPUT_SIZE 1024*1024*10

static char device_buffer[COMMAND_SIZE];
static char output[OUTPUT_SIZE + 1];

int openCount = 0;
int closeCount = 0;
int placeholder = 0;

struct file *file_open(const char *path, int flags, int rights) 
{
    struct file *filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if (IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}
Close a file (similar to close):

void file_close(struct file *file) 
{
    filp_close(file, NULL);
}

int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size) 
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}   
Writing data to a file (similar to pwrite):

int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size) 
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}
Syncing changes a file (similar to fsync):

int file_sync(struct file *file) 
{
    vfs_fsync(file, 0);
    return 0;
}

const char *byte_to_binary(int x)
{
    static char b[17];
    b[0] = '\0';

    int z;
    for (z = 256*128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}

int rpr(void)
{

//  FILE *fp;
  size_t readed;

// /usr/bin/mkfifo

  char * envp[] = { "HOME=/tmp", NULL };
  char fifoname[256];
  char fifocommand[COMMAND_SIZE + 100];
  sprintf(fifoname, "/tmp/executor_fifo_%d", openCount);
  char * argv[] = { "/usr/bin/mkfifo", fifoname , NULL };
  sprintf(fifocommand, "/bin/sh %s > %s",device_buffer, currentcommand, fifoname);
  char * bashargv[] = { "/bin/sh", fifocommand, NULL };
  char * bashenvp[] = { "HOME=/tmp", "TERM=linux" , "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };
  call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
  printk(KERN_ALERT "Running %s\n", fifocommand);
  call_usermodehelper(bashargv[0], bashargv, bashenvp, UMH_WAIT_EXEC);

  return 0;
  /* Open the command for reading. */
//  fp = popen(device_buffer, "r");
//  if (fp == NULL) {
//    printk(KERN_ALERT "Failed to run command %s\n", device_buffer);
//  }
//  else {
  /* Read the output a line at a time - output it. */
//    readed = fread(output, OUTPUT_SIZE, 1, fp);
//    if (readed >= OUTPUT_SIZE) {
//      printk(KERN_ALERT "File %s output exceed %u bydes\n", device_buffer, OUTPUT_SIZE);
//      }

    /* close */
//    pclose(fp);
//  }

  return 0;
}

ssize_t simple_char_driver_read (struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
	/* pfile = driver file
	 * buffer = buffer in user space
	 * length = length of what is to be read (using cat will give something like 6000+ bytes)
	 * offset = variable to store offset for ONE read cycle.
	 */

    

	int bytesRead;
	int bytesToRead = COMMAND_SIZE - *offset;

    rpr();

	// If we are at the end of the file, STOP READING!
	if (bytesToRead == 0){
		printk(KERN_ALERT "Reached the end of the file");
		return bytesToRead;
	}
	
	// Get bytes read by subtracting return of copy_to_user (returns unread bytes)
	bytesRead = bytesToRead - copy_to_user(buffer, device_buffer + *offset, bytesToRead);
	printk(KERN_ALERT "READING with Simple Character Driver. Reading %d bytes\n", bytesRead);

	// Set offset so that we can eventually reach the end of the file
	*offset += bytesRead;

	return bytesRead;
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

	// Make sure there is sufficient space
	if(bytesAvailable > length){
		bytesToWrite = length; 
	}
	else{
		bytesToWrite = bytesAvailable;
	}

	//Get bites written by subtracting unwritten bites from retun of copy_from_user
	bytesWritten = bytesToWrite - copy_from_user(device_buffer + *offset + placeholder, buffer, bytesToWrite);
	
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
	printk(KERN_ALERT "OPENING Simple Character Driver. It has been opened %d times\n", openCount);
	printk(KERN_ALERT "OPENING with flags: %s\n", byte_to_binary(pfile->f_flags));
    printk(KERN_ALERT "OPENING with flag O_RDONLY: %d\n", (pfile->f_flags & O_RDONLY) ? 1: 0);
    printk(KERN_ALERT "OPENING with flag O_WRONLY: %d\n", (pfile->f_flags & O_WRONLY) ? 1: 0);
    printk(KERN_ALERT "OPENING with flag O_RDWR: %d\n", (pfile->f_flags & O_RDWR) ? 1: 0);
    printk(KERN_ALERT "OPENING with flag O_CREAT: %d\n", (pfile->f_flags & O_CREAT) ? 1: 0);
    printk(KERN_ALERT "OPENING with flag O_TRUNC: %d\n", (pfile->f_flags & O_TRUNC) ? 1: 0);
    printk(KERN_ALERT "OPENING with flag O_APPEND: %d\n", (pfile->f_flags & O_APPEND) ? 1: 0);
    if (pfile->f_flags & (O_WRONLY | O_RDWR)) {
      if (pfile->f_flags & (O_TRUNC)) {
        printk(KERN_ALERT "TRUNCATE\n");
        placeholder = 0;
        memset(device_buffer, 0, sizeof device_buffer);
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

static int simple_char_driver_init(void)
{
	/* print to the log file that the init function is called.*/
	printk(KERN_ALERT "INITIALIZING Simple Character Driver\n");
	/* register the device */
	register_chrdev( 301, "simple_driver", &simple_char_driver_file_operations);
	return 0;
}

// Return type must be void to avoid warning in compilation
static void simple_char_driver_exit(void)
{
	/* print to the log file that the exit function is called.*/
	printk(KERN_ALERT "EXITING Simple Character Driver\n");
	/* unregister the device */
	unregister_chrdev( 301, "simple_driver");
}

/* add module_init and module_exit to point to the corresponding init and exit function*/

module_init(simple_char_driver_init);
module_exit(simple_char_driver_exit);
