#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <generated/utsrelease.h>
#include <linux/version.h>
#include <linux/cred.h>
#include <linux/uaccess.h>

#define SUCCESS 0
#define DEVICE_NAME "module2"
#define MAX_BUF_LEN 4096

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohammmad Zangoei <mmd1997z@gmail.com>");
MODULE_DESCRIPTION("HW1_part2");
MODULE_VERSION("1.0.0");

static struct proc_dir_entry* our_proc_file1, *our_proc_file2;
static char procfs_buffer1[MAX_BUF_LEN], procfs_buffer2[MAX_BUF_LEN];
static unsigned long procfs_buffer_size1 = 0, procfs_buffer_size2 = 0;
static struct proc_dir_entry *parent;

int procfs_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "MODULE2: Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static ssize_t procfs_read1(struct file *filp, char *buffer, size_t length, loff_t * offset){
	static int ret = 0;

  printk(KERN_INFO "MODULE2: Read Function, Process \"%s:%i\"\n", current->comm, current->pid);

  if(ret){
		printk(KERN_INFO "MODULE2: Read END\n");
		ret = 0;
	}
	else{
		if(raw_copy_to_user(buffer, procfs_buffer2, procfs_buffer_size2))
			return -EFAULT;
		printk(KERN_INFO "MODULE2: Read %lu bytes\n", procfs_buffer_size2);
		ret = procfs_buffer_size2;
	}

  return ret;
}

static ssize_t procfs_read2(struct file *filp, char *buffer, size_t length, loff_t * offset){
	static int ret = 0;

  printk(KERN_INFO "MODULE2: Read Function, Process \"%s:%i\"\n", current->comm, current->pid);

  if(ret){
		printk(KERN_INFO "MODULE2: Read END\n");
		ret = 0;
	}
	else{
		if(raw_copy_to_user(buffer, procfs_buffer1, procfs_buffer_size1))
			return -EFAULT;
		printk(KERN_INFO "MODULE2: Read %lu bytes\n", procfs_buffer_size1);
		ret = procfs_buffer_size1;
	}

  return ret;
}

static ssize_t procfs_write1(struct file *file, const char *buffer, size_t length, loff_t * off){
	printk(KERN_INFO "MODULE2: Write Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if(length > MAX_BUF_LEN)
		procfs_buffer_size1 = MAX_BUF_LEN;
	else
		procfs_buffer_size1 = length;
	if(raw_copy_from_user(procfs_buffer1, buffer, procfs_buffer_size1))
		return -EFAULT;
	printk(KERN_INFO "READWRITEPROCFS: Write %lu bytes\n", procfs_buffer_size1);
	return procfs_buffer_size1;
}

static ssize_t procfs_write2(struct file *file, const char *buffer, size_t length, loff_t * off){
	printk(KERN_INFO "MODULE2: Write Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if(length > MAX_BUF_LEN)
		procfs_buffer_size2 = MAX_BUF_LEN;
	else
		procfs_buffer_size2 = length;
	if(raw_copy_from_user(procfs_buffer2, buffer, procfs_buffer_size2))
		return -EFAULT;
	printk(KERN_INFO "READWRITEPROCFS: Write %lu bytes\n", procfs_buffer_size2);
	return procfs_buffer_size2;
}

int procfs_close(struct inode *inode, struct file *file){
	printk(KERN_INFO "READWRITEPROCFS: Release Function, Process \"%s:%i\"\n", current->comm, current->pid);

	module_put(THIS_MODULE);

	return SUCCESS;
}

static const struct file_operations fops1 = {
  .owner = THIS_MODULE,
	.read = procfs_read1,
	.write = procfs_write1,
	.open = procfs_open,
	.release = procfs_close,
};

static const struct file_operations fops2 = {
  .owner = THIS_MODULE,
	.read = procfs_read2,
	.write = procfs_write2,
	.open = procfs_open,
	.release = procfs_close,
};


static int __init module2_init(void){

	printk(KERN_INFO "MODULE2: Initialization.\n");
	printk(KERN_INFO "MODULE2: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);

	#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 10)

		printk(KERN_INFO "MODULE2: Hello OLD Kernel %s, So Use create_proc_entry Function\n", UTS_RELEASE);

  	our_proc_file1 = create_proc_entry("proc1", 0644 , NULL);
    our_proc_file2 = create_proc_entry("proc2", 0644 , NULL);

		our_proc_file->proc_fops = &fops1;
		our_proc_file->proc_iops = &iops2;

	#else

		printk(KERN_INFO "MODULE2: Hello NEW Kernel %s, So Use proc_create Function\n", UTS_RELEASE);

	parent = proc_mkdir("proc_folder", NULL);
  	our_proc_file1 = proc_create("proc1", 0644 , parent, &fops1);
	our_proc_file2 = proc_create("proc2", 0644 , parent, &fops2);

	#endif

	if(!our_proc_file1 || !our_proc_file2){
		printk(KERN_ALERT "MODULE2: Registration Failure.\n");

		return -ENOMEM;
	}

	printk(KERN_INFO "MODULE2: /proc/%s has been created.\n", DEVICE_NAME);
	printk(KERN_INFO "MODULE2: 'sudo chmod 777 /proc/proc_folder/proc*'.\n");

  return SUCCESS;
}

static void __exit module2_exit(void){

	printk(KERN_INFO "MODULE2: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);

	remove_proc_entry("proc1", NULL);
	proc_remove(parent);
  remove_proc_entry("proc2", NULL);

	printk(KERN_INFO "MODULE2: /proc/%s has been removed.\n", DEVICE_NAME);

	printk(KERN_INFO "MODULE2: GoodBye.\n");
}

module_init(module2_init);
module_exit(module2_exit);
