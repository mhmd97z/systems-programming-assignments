//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for some macros
#include <linux/init.h>
//For "struct task_struct" that contains current process
#include <linux/sched.h>
//For struct file_operations
#include <linux/fs.h>
//For copy_to_user, copy_from_user, put_user
#include <linux/uaccess.h>
//For create and register a procfs entry
#include <linux/proc_fs.h>
//For macro UTS_RELEASE
#include <generated/utsrelease.h>
//For creating/deleting of a kobject and relevent manipulation functions
#include <linux/kobject.h>
//For getting system information
#include <linux/utsname.h>
//This is for working with sysfs entries and Linux device model
#include <linux/sysfs.h>
//For creating/deleting of a kobject and relevent manipulation functions
#include <linux/kobject.h>
//For create and register a procfs entry
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/string.h> 
//For ioctl commands and macros
#include <linux/ioctl.h>
#include <asm/ioctl.h>

#include "commonioctlcommands.h"

#define SUCCESS 0
#define MY_MODULE_NAME "module3"
#define BUF_LEN 4096
#define ENTRY_LEN 32
#define DEV_NAME "dmod"
#define LOG_ENTRY " stack size: %2d \n list size: %2d \n"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohammmad Zangoei <mmd1997z@gmail.com>");
MODULE_DESCRIPTION("HW2");
MODULE_VERSION("2.0.0");

static int major, Device_Open = 0, stack_top = 0, list_head = 0, list_tale = 0, flag_stack = 0, flag_list = 1, err_stack =0, err_list = 0;
static int read_check = 0;
static char list[BUF_LEN], stack[BUF_LEN];
static unsigned long dev_buffer_size = 0;
static struct kobject *our_kobj;
static struct proc_dir_entry* pfifo_proc_file;
static struct proc_dir_entry* plog_proc_file;
static struct proc_dir_entry* pstack_proc_file;
static struct proc_dir_entry* ioctl_proc_file;


// -------------------------------------------------------------------------- proc Interface


long proc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	
	static int retval = 0;
	char output[30];
	printk(KERN_INFO "MODULE3: IOCTLPROCFS: IOCTL Command, %d, %d\n", cmd, IOCTL_SFIFO_RESET);
	
	if( cmd == IOCTL_SFIFO_RESET) {

		list_head = 0;
		list_tale = 0;
	} else if (cmd == IOCTL_SSTACK_RESET) {
		stack_top = 0;
	} else if ( cmd == IOCTL_ALL_RESET ){
		list_head = 0;
		list_tale = 0;
		stack_top = 0;
	} else{
		printk(KERN_ALERT "IOCTLPROCFS: Invalid IOCTL Command!\n");
		return -ENOTTY;
	}

	return retval;
}

static int proc_show_ioctl(struct seq_file *m, void *v)
{
	
	seq_printf(m, "List size: %d\n", ((list_tale - list_head)/ENTRY_LEN) );
	seq_printf(m, "Stack size: %d\n", ((stack_top)/ENTRY_LEN) - 1 );

	return SUCCESS;
}


static int proc_open_ioctl(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "MODULE3: IOCTLPROCFS, Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, proc_show_ioctl, NULL);
}


static const struct file_operations fops_ioctl = {
	.open = proc_open_ioctl,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.unlocked_ioctl = proc_ioctl,
};


// -------------------------------------------------------------------------- proc Interface


int procfs_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "MODULE3: Proc Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

int procfs_close(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "MODULE3: Proc Release Function, Process \"%s:%i\"\n", current->comm, current->pid);
	module_put(THIS_MODULE);
	return SUCCESS;
}


static int proc_show_plog(struct seq_file *m, void *v)
{
	printk(KERN_EMERG "MODULE3: you entered the show function \n");

	seq_printf(m, "stack size: %d\n", stack_top / ENTRY_LEN);
	seq_printf(m, "list size: %d\n", (list_tale - list_head) / ENTRY_LEN);

	return SUCCESS;
}

static int proc_open_plog(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "MODULE3: plog Proc Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, proc_show_plog, NULL);
}

static const struct file_operations fops_plog = {
	.open = proc_open_plog,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};


static int proc_show_pfifo(struct seq_file *m, void *v)
{
	printk(KERN_EMERG "MODULE3: you entered the fifo show function \n");

	int list_head_tmp = list_head;
	int list_tale_tmp = list_tale;
	
	char tmp[33];
	tmp[32] = "\0";
	
	while(list_head_tmp != list_tale_tmp){
		strncpy(tmp,  &list[list_head_tmp], ENTRY_LEN);
		seq_printf(m, tmp);
		list_head_tmp = list_head_tmp + ENTRY_LEN;
	}

	return SUCCESS;
}

static int proc_open_pfifo(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "MODULE3: fifo Proc Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, proc_show_pfifo, NULL);
}

static const struct file_operations fops_pfifo = {
	.open = proc_open_pfifo,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};


static int proc_show_pstack(struct seq_file *m, void *v)
{
	printk(KERN_EMERG "MODULE3: you entered the stack show function \n");

	int stack_top_tmp = stack_top;
	
	char tmp[33];
	tmp[32] = "\0";
	
	while(stack_top_tmp != 0){
		strncpy(tmp,  &stack[stack_top_tmp - ENTRY_LEN], ENTRY_LEN);
		seq_printf(m, tmp);
		stack_top_tmp = stack_top_tmp - ENTRY_LEN;
	}

	return SUCCESS;
}

static int proc_open_pstack(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "MODULE3: stack Proc Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, proc_show_pstack, NULL);
}

static const struct file_operations fops_pstack = {
	.open = proc_open_pstack,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

// -------------------------------------------------------------------------- Sys Interface

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	printk(KERN_INFO "MODULE3: Show Function, %s Attribute,  Process \"%s:%i\"\n",  attr->attr.name, current->comm, current->pid);

	if(strcmp(attr->attr.name, "sfifo") == 0)
		return sprintf(buf, "%d\n", flag_list);
	else if (strcmp(attr->attr.name, "sstack") == 0)
		return sprintf(buf, "%d\n", flag_stack);
	else
		printk(KERN_INFO  "MODULE3: I don't know what you are doing, but it seems you are not doing it right!\n");

	return NULL;
}

static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	printk(KERN_INFO "MODULE3: Store Function, %s Attribute,  Process \"%s:%i\"\n",  attr->attr.name, current->comm, current->pid);

		if(strcmp(attr->attr.name, "sfifo") == 0){
			
			if(strcmp(buf,"1\n") == 0 ){
				flag_list = 1;
			} else if(strcmp(buf,"0\n") == 0){
				flag_list = 0;
			} else
				printk(KERN_ALERT "MODULE3: Not a valid Input");

		} else if (strcmp(attr->attr.name, "sstack") == 0){
			
			if(strcmp(buf,"1\n") == 0 ){
				flag_stack = 1;
			} else if(strcmp(buf,"0\n") == 0){
				flag_stack = 0;
			} else
				printk(KERN_ALERT "MODULE3: Not a valid Input");
			
		} else
			printk(KERN_INFO  "MODULE3: I don't know what you are doing, but it seems you are not doing it right!\n");

	return count;
}

static struct kobj_attribute sfifo = __ATTR(sfifo, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute sstack = __ATTR(sstack, 0664, sysfs_show, sysfs_store);

static struct attribute *attrs[] = {
	&sfifo.attr,
	&sstack.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};


// -------------------------------------------------------------------------- Dev Interface
static int device_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "HELLOWORLDCHARDEV: Open Function, Process \"%s:%i\"\n", current->comm, current->pid);

	if (Device_Open)
		return -EBUSY;
	Device_Open++;
	read_check = 1;

	try_module_get(THIS_MODULE);

	return SUCCESS;
}


static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset)
{
	
	printk(KERN_INFO "MODULE3: Read Function, Process \"%s:%i\"\n", current->comm, current->pid);

	if(flag_stack && flag_list)
	{
		printk(KERN_ALERT "MODULE3: Sorry, READ operation is permitted only in one mode!\n");
		return -EINVAL;
	}

	if(read_check){ 

		read_check = 0;

		if(flag_stack){

			if(stack_top != 0){

				if(raw_copy_to_user(buffer , &stack[stack_top - ENTRY_LEN], ENTRY_LEN))
					return -EFAULT;

				stack_top = stack_top - ENTRY_LEN;

				printk(KERN_INFO "MODULE3: %d bytes has read from stack\n", ENTRY_LEN);

				return ENTRY_LEN;
			} else{
				printk(KERN_INFO "MODULE3: stack is empty\n");
				return 0;
			}
		}

		if(flag_list){

			if( list_head == 0  ) {
				if( list_tale != 0){

					if(raw_copy_to_user(buffer, &list[list_head], ENTRY_LEN))
						return -EFAULT;

					list_head = list_head + ENTRY_LEN;

					printk(KERN_INFO "MODULE3: %d bytes has read from list\n", ENTRY_LEN);

					return ENTRY_LEN;

				}
				else{
					printk(KERN_INFO "MODULE3: list is empty\n");
					return 0;
				}
			}
			else if ( list_head != list_tale )
			{
				if(raw_copy_to_user(buffer, &list[list_head], ENTRY_LEN))
					return -EFAULT;

				list_head = list_head + ENTRY_LEN;

				printk(KERN_INFO "MODULE3: %d bytes has read from list\n", ENTRY_LEN);

				return ENTRY_LEN;
			}
			else{
				list_tale = 0;
				list_head = 0;
				printk(KERN_INFO "MODULE3: list is empty\n");
				return 0;	
			}
		
		}
	}
	return 0;
}


static ssize_t device_write(struct file *filp, const char *buffer, size_t length, loff_t * off){
	
	printk(KERN_INFO "MODULE3: Write Function, Process \"%s:%i\"\n", current->comm, current->pid);
	
	err_stack = 0;
	err_list = 0;
	int i;

	if (length > ENTRY_LEN)
		dev_buffer_size = ENTRY_LEN;
	else
		dev_buffer_size = length;

	if(flag_stack) {

		if(stack_top != (BUF_LEN - 1)){													// check wether stack has room
			
			if(raw_copy_from_user(&stack[stack_top], buffer, dev_buffer_size)){	
				err_stack = 1;
				printk(KERN_INFO "MODULE3: unsuccessful write to the stack\n");	
			}

			if(!err_stack){

				if(dev_buffer_size == length){

					for(i = -1; i < (ENTRY_LEN - length); i++)
						stack[stack_top + length + i] = "\0";

				}

				stack_top = stack_top + ENTRY_LEN;

				printk(KERN_INFO "MODULE3: %lu bytes has wrote to the stack\n", dev_buffer_size);
				//return dev_buffer_size;
			}

		} else{
			err_stack = 1;
			printk(KERN_INFO "MODULE3: stack is full!\n");	
		}

	}

	if(flag_list){

		if( list_tale != (BUF_LEN - 1) ) {											// check wether stack has room

			if(raw_copy_from_user(&list[list_tale], buffer, dev_buffer_size)){
				err_list = 1;
				printk(KERN_INFO "MODULE3: unsuccessful write to the list\n");
			}

			if(!err_list){

				if(dev_buffer_size == length){

					for(i = -1; i < (ENTRY_LEN - length); i++)
						list[list_tale + length + i] = "\0";

				}

				list_tale = list_tale + ENTRY_LEN;

				printk(KERN_INFO "MODULE3: %lu bytes has wrote to the list\n", dev_buffer_size);

			}

		} else{
			err_list = 1;
			printk(KERN_INFO "MODULE3: list is full!\n");	
		}
	}

	if( ((!err_stack) && flag_stack) || ((!err_list) && flag_list) ){
		printk(KERN_INFO "MODULE3: write successful return\n");
		return dev_buffer_size;

	} else{
		printk(KERN_INFO "MODULE3: write unsuccessful return\n");
		return -EFAULT;
	}

}


static int device_release(struct inode *inode, struct file *file)
{

	printk(KERN_INFO "MODULE3: Release Function, Process \"%s:%i\"\n", current->comm, current->pid);

	Device_Open--;

	module_put(THIS_MODULE);

	return SUCCESS;
}


static struct file_operations dev_fops = {

	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release

};


static int __init module3_init(void)
{

	printk(KERN_ALERT "MODULE3: %s Initialization.\n", MY_MODULE_NAME);
	printk(KERN_INFO "MODULE3: Module \"%s\" Loading,  By Process \"%s:%i\"\n", MY_MODULE_NAME, current->comm, current->pid);
	
	major = register_chrdev(0, DEV_NAME, &dev_fops);
	if (major < 0) {
		printk(KERN_ALERT "MODULE3: Registration Filure, %d Major\n", major);
		return major;
	}

	printk(KERN_INFO "MODULE3: 'sudo mknod /dev/%s c %d 0'\n", DEV_NAME, major);
	printk(KERN_INFO "MODULE3: 'sudo chmod 777 /dev/%s'.\n", DEV_NAME);
	
	int retval;
	our_kobj = kobject_create_and_add(MY_MODULE_NAME, kernel_kobj);
	if (!our_kobj){
		printk(KERN_ALERT "MODULE3: KOBJECT Registration Failure.\n");
		return -ENOMEM;
	}

	retval = sysfs_create_group(our_kobj, &attr_group);
	if (retval){
		printk(KERN_ALERT "MODULE3: Creating attribute groupe has been failed.\n");
		kobject_put(our_kobj);
	}
	printk(KERN_INFO "MODULE3: /sys/kernel/%s and all its attributes has been added.\n", MY_MODULE_NAME);
	printk(KERN_INFO "MODULE3: sudo chmod 777 /sys/kernel/%s/s* \n", MY_MODULE_NAME);

	plog_proc_file = proc_create("plog", 0644 , NULL, &fops_plog);
	if(!plog_proc_file){
		printk(KERN_ALERT "MODULE3: plog Registration Failure.\n");
		return -ENOMEM;
	}
	printk(KERN_INFO "MODULE3: /proc/%s has been created.\n", "plog");


	pfifo_proc_file = proc_create("pfifo", 0644 , NULL, &fops_pfifo);
	if(!pfifo_proc_file){
		printk(KERN_ALERT "MODULE3: pfifo Registration Failure.\n");
		return -ENOMEM;
	}
	printk(KERN_INFO "MODULE3: /proc/%s has been created.\n", "pfifo");


	pstack_proc_file = proc_create("pstack", 0644 , NULL, &fops_pstack);
	if(!pstack_proc_file){
		printk(KERN_ALERT "MODULE3: pstack Registration Failure.\n");
		return -ENOMEM;
	}


	ioctl_proc_file = proc_create("module3_ioctl", 0644 , NULL, &fops_ioctl);
	//Put an error message in kernel log if cannot create proc entry
	if(!ioctl_proc_file){
		printk(KERN_ALERT "MODULE3: IOCTLPROCFS: Registration Failure.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}

	printk(KERN_INFO "MODULE3: IOCTLPROCFS: /proc/%s has been created.\n", "module3_ioctl");

	printk(KERN_INFO "MODULE3: /proc/%s has been created.\n", "pstack");

	return SUCCESS;
}


static void __exit module3_exit(void)
{

	printk(KERN_INFO "MODULE3: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);

	unregister_chrdev(major, DEV_NAME);
	printk(KERN_INFO "MODULE3: 'sudo rm /dev/%s'\n", DEV_NAME);
	
	kobject_put(our_kobj);
	printk(KERN_INFO "MODULE3: /sys/kernel/%s and all its attributes has been removed.\n", MY_MODULE_NAME);

	remove_proc_entry("plog", NULL);
	printk(KERN_INFO "MODULE3: /proc/%s has been removed.\n", "plog");

	remove_proc_entry("pstack", NULL);
	printk(KERN_INFO "MODULE3: /proc/%s has been removed.\n", "pstack");

	remove_proc_entry("pfifo", NULL);
	printk(KERN_INFO "MODULE3: /proc/%s has been removed.\n", "pfifo");

	remove_proc_entry("module3_ioctl", NULL);
	printk(KERN_INFO "MODULE3: IOCTLPROCFS: /proc/%s has been removed.\n", "module3_ioctl");

	printk(KERN_INFO "MODULE3: GoodBye.\n");

}

module_init(module3_init);
module_exit(module3_exit);
