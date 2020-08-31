//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for some macros
#include <linux/init.h>
//For finding the parent process ID of the module
#include <asm/current.h>
//For "struct task_struct" that contains current process
#include <linux/sched.h>
//For struct file_operations
#include <linux/fs.h>
//For copy_to_user, copy_from_user, put_user
#include <linux/uaccess.h>
//For providing read function of the entry with ease
#include <linux/seq_file.h>
//For create and register a procfs entry
#include <linux/proc_fs.h>
//For time
#include <linux/time.h>

#define MY_MODULE_NAME "module1"
#define SUCCESS 0
#define MAX_BUF_LEN 4096
#define LOG_ENTRY "number of written bytes:%4lu @Local Time %2d:%2d:%2d (GMT %c%d:%d)\n"
#define LOG_ENTRY_LEN 62

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohammmad Zangoei <mmd1997z@gmail.com>");
MODULE_DESCRIPTION("HW1_part1");
MODULE_VERSION("1.0.0");

static int major, Device_Open = 0;
static struct proc_dir_entry* our_proc_file;
static char dev_buffer[MAX_BUF_LEN], log[MAX_BUF_LEN];
static unsigned long dev_buffer_size = 0, log_size = 0;

static int gmt_hour = 3;
module_param(gmt_hour, int, 0);
MODULE_PARM_DESC(gmt_hour, "This command line argument will differ local time hours from GMT time, default is +3 hours for Tehran, Iran");
static int gmt_minute = 30;
module_param(gmt_minute, int, 0);
MODULE_PARM_DESC(gmt_minute, "This command line argument will differ local time minutes from GMT time, default is +30 minutes for Tehran, Iran");
//This will use to create a nice output
static char gmt_sign = '+';

static int device_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "MODULE1: Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if (Device_Open)
		return -EBUSY;
	Device_Open++;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset){
	printk(KERN_INFO "MODULE1: Read Function, Process \"%s:%i\"\n", current->comm, current->pid);
	printk(KERN_ALERT "MODULE1: Sorry, WRITE operation is not permitted!\n");
	return -EINVAL;
}

static ssize_t device_write(struct file *file, const char *buffer, size_t length, loff_t * off){
	printk(KERN_INFO "MODULE1: Write Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if (length > MAX_BUF_LEN)
		dev_buffer_size = MAX_BUF_LEN;
	else
		dev_buffer_size = length;
	if(raw_copy_from_user(dev_buffer, buffer, dev_buffer_size))
		return -EFAULT;
	printk(KERN_INFO "MODULE1: %lu bytes has wrote to /dev entry\n", dev_buffer_size);
	
	//Calculate time 
	int hour_loc, minute_loc, second_loc;
	struct timespec my_timeofday_loc;
	getnstimeofday(&my_timeofday_loc);
	second_loc = (int) my_timeofday_loc.tv_sec;
	hour_loc = ((second_loc / 3600) % 24 + gmt_hour) % 24;
	minute_loc = (second_loc / 60) % 60 + gmt_minute;
	if(minute_loc>=60){
		hour_loc = (hour_loc + 1) % 24;
		minute_loc %= 60;
		}
	second_loc %= 60;
	//LOG_ENTRY: "number of written bytes:%4lu @Local Time %2d:%2d:%2d (GMT %c%d:%d)\n"
	sprintf(&log[LOG_ENTRY_LEN*log_size], LOG_ENTRY, dev_buffer_size, hour_loc, minute_loc, second_loc, gmt_sign, gmt_hour, gmt_minute);
	log_size++;

	return dev_buffer_size;
}

int device_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "MODULE1: Release Function, Process \"%s:%i\"\n", current->comm, current->pid);
	Device_Open--;
	module_put(THIS_MODULE);
	return SUCCESS;
}


//This function calls on demand of read request from seq_files
static int proc_show(struct seq_file *m, void *v){
	seq_printf(m, "%s", log);
	return SUCCESS;
}

//This is where system functionallity triggers every time some process try to read from our proc entry
static int proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "MODULE1: Proc Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, proc_show, NULL);
}

static const struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
};

static const struct file_operations pops = {
	.open = proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init module1_init(void){
	printk(KERN_ALERT "MODULE1: %s Initialization.\n", MY_MODULE_NAME);
	printk(KERN_INFO "MODULE1: Module \"%s\" Loading,  By Process \"%s:%i\"\n", MY_MODULE_NAME, current->comm, current->pid);
	our_proc_file = proc_create(MY_MODULE_NAME, 0644 , NULL, &pops);
	if(!our_proc_file){
		printk(KERN_ALERT "MODULE1: Registration Failure.\n");
		return -ENOMEM;
	}
	printk(KERN_INFO "MODULE1: /proc/%s has been created.\n", MY_MODULE_NAME);
	major = register_chrdev(0, MY_MODULE_NAME, &fops);
	if (major < 0) {
		printk(KERN_ALERT "MODULE1: Registration Filure, %d Major\n", major);
		return major;
	}
	printk(KERN_INFO "MODULE1: 'mknod /dev/%s c %d 0'\n", MY_MODULE_NAME, major);
	printk(KERN_INFO "MODULE1: 'chmod 777 /dev/%s'.\n", MY_MODULE_NAME);
	return SUCCESS;
}

static void __exit module1_exit(void){
	printk(KERN_INFO "MODULE1: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);
	remove_proc_entry(MY_MODULE_NAME, NULL);
	printk(KERN_INFO "MODULE1: /proc/%s has been removed.\n", MY_MODULE_NAME);
	unregister_chrdev(major, MY_MODULE_NAME);
	printk(KERN_INFO "MODULE1: 'rm /dev/%s'\n", MY_MODULE_NAME);
	printk(KERN_INFO "MODULE1: GoodBye.\n");
}

module_init(module1_init);
module_exit(module1_exit);
