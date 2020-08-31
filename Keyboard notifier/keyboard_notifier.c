#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/keyboard.h>
#include <linux/semaphore.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohammad Zangooei <mmd1997z@gmail.com>");
MODULE_DESCRIPTION("This is keyborad logger");
MODULE_VERSION("1.0.0");

#define SUCCESS 0
#define MAX_BUF_LEN 4096
#define DEVICE_NAME "keyboard_logger"

struct semaphore sem;

static const char* keys[] = { "\0", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "BACKSPACE", "TAB",
	"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "SPACE", "SPACE", "ENTER", "CTRL", "a", "s", "d", "f",
	"g", "h", "j", "k", "l", ";", "'", "`", "SHIFT", "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".",
	"/", "SHIFT", "\0", "\0", "SPACE", "CAPSLOCK", "F1", "F2", "F3", "F4", "F5", "F6", "F7",
	"F8", "F9", "F10", "NUMLOCK", "SCROLLLOCK", "HOME", "UP", "PGUP", "-", "LEFT", "5",
	"RTARROW", "+", "END", "DOWN", "PGDN", "INS", "DELETE", "\0", "\0", "\0", "F11", "F12",
	"\0", "\0", "\0", "\0", "\0", "\0", "\0", "ENTER", "CTRL", "/", "PRTSCR", "ALT", "\0", "HOME",
	"UP", "PGUP", "LEFT", "RIGHT", "END", "DOWN", "PGDN", "INSERT", "DEL", "\0", "\0",
	"\0", "\0", "\0", "\0", "\0", "PAUSE"
	};
static const char* shift_keys[] ={ "\0", "ESC", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "BACKSPACE", "TAB",
	"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "ENTER", "CTRL", "A", "S", "D", "F",
	"G", "H", "J", "K", "L", ":", "\"", "~", "SHIFT", "|", "Z", "X", "C", "V", "B", "N", "M", "<", ">",
	"?", "SHIFT", "\0", "\0", "SPACE", "CAPSLOCK", "F1", "F2", "F3", "F4", "F5", "F6", "F7",
	"F8", "F9", "F10", "NUMLOCK", "SCROLLLOCK", "HOME", "UP", "PGUP", "-", "LEFT", "5",
	"RTARROW", "+", "END", "DOWN", "PGDN", "INS", "DELETE", "\0", "\0", "\0", "F11", "F12",
	"\0", "\0", "\0", "\0", "\0", "\0", "\0", "ENTER", "CTRL", "/", "PRTSCR", "ALT", "\0", "HOME",
	"UP", "PGUP", "LEFT", "RIGHT", "END", "DOWN", "PGDN", "INSERT", "DEL", "\0", "\0",
	"\0", "\0", "\0", "\0", "\0", "PAUSE"
	};

static int shift_key_flag = 0;
static int error = 0;

static struct proc_dir_entry* proc_entry;
	
static int* key_cntr[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0};
static int* shift_key_cntr[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0};


int keyboard_notify( struct notifier_block *nblock, unsigned long code, void *_param ){
	
	struct keyboard_notifier_param *param = _param;

	if(code == KBD_KEYCODE){
		if( param->value==42 || param->value==54 ){   // L-R shift
			down(&sem);
			if(param->down)
				shift_key_flag = 1;
			else
				shift_key_flag = 0;
			up(&sem);
			return NOTIFY_DONE;
		}
		if(param->down){	
			down(&sem);
			if(shift_key_flag){
				shift_key_cntr[param->value] += 1;
			}
			else {
				key_cntr[param->value] += 1;
			}	
			up(&sem);
			return NOTIFY_OK;
		}
	}
	return NOTIFY_OK;
}


static struct notifier_block keyboard_nb ={
	.notifier_call = keyboard_notify,
};


static int proc_show(struct seq_file *m, void *v){
	int i;
	for(i = 0; i < 120; i++){
		int tmp = key_cntr[i];
		int tmp2 = shift_key_cntr[i];
		if(key_cntr[i] != 0)
			seq_printf(m, " %s -> %d \n---------------\n", keys[i], tmp / 4);
		if(shift_key_cntr[i] != 0)
			seq_printf(m, " %s -> %d \n---------------\n", shift_keys[i], tmp2 / 4);
}
	return SUCCESS;
}


static int proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "KEYBOARD LOGGER: Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, proc_show, NULL);
}


static const struct file_operations procfops = {
	.open = proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};


static int __init keyboard_notifier_init(void){
	printk(KERN_INFO "KEYBOARD LOGGER: Init module, Process(%s:%i)\n", current->comm, current->pid);
	proc_entry = proc_create("kb_history", 0644 , NULL, &procfops);	
	error = register_keyboard_notifier(&keyboard_nb);
	if(error){
		printk(KERN_ALERT "KEYBOARD LOGGER: Error in registering keyboard logger\n");
		return error;
		}
	printk(KERN_INFO "KEYBOARD LOGGER: Keyboard logger registered successfully\n");
	sema_init(&sem, 1);
	return error;

}


static void __exit keyboard_notifier_exit(void){
	printk(KERN_INFO "KEYBOARD LOGGER: Init module, Process(%s:%i)\n", current->comm, current->pid);
	remove_proc_entry("kb_history", NULL);
	unregister_keyboard_notifier(&keyboard_nb);
    printk(KERN_INFO "KEYBOARD LOGGER: Keyboard notifier deregistered\n");
}


module_init(keyboard_notifier_init);
module_exit(keyboard_notifier_exit);
