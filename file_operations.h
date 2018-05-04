struct file *file_open(const char *path, int flags, int rights)
{
    struct file *filp = NULL;
    mm_segment_t oldfs;
    int err = 0;
    
    printk("Opening file %s", path);
    
    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if (IS_ERR(filp)) {
        printk("Error opening file %s", path);
        err = PTR_ERR(filp);
        printk("Error opening file %d", err);
        return NULL;
    }
    return filp;
}
// Close a file (similar to close):

void file_close(struct file *file)
{
    printk("Closing file %p", file);
    filp_close(file, NULL);
}

int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{

    printk("Reading file %p", file);
    
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}
// Writing data to a file (similar to pwrite):

int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
    printk("Writing file %p", file);
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

// Syncing changes a file (similar to fsync):

int file_sync(struct file *file)
{
    printk("Syncing file %p", file);
    vfs_fsync(file, 0);
    return 0;
}


