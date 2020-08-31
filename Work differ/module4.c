#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/smp.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>

#define SUCCESS 0
#define DEVICE_NAME "module4"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohammad Zangooei <mmd1997z@gmail.com>");
MODULE_VERSION("1.0.0");

static struct workqueue_struct *our_workqueue_TinW;
static struct workqueue_struct *our_workqueue_WinT;
static struct workqueue_struct *our_workqueue_SinW;
static struct workqueue_struct *our_workqueue_WinS;

static struct tasklet_struct our_tasklet_TinW;
static struct tasklet_struct our_tasklet_WinT;
static struct tasklet_struct our_tasklet_SinT;
static struct tasklet_struct our_tasklet_TinS;

static unsigned long before_delay_TinW, after_delay_TinW,before_delay_WinT, after_delay_WinT;
static unsigned long before_delay_SinW, after_delay_SinW, before_delay_WinS, after_delay_WinS;
static unsigned long before_delay_SinT, after_delay_SinT, before_delay_TinS, after_delay_TinS;

static char our_tasklet_argument_TinW[20] = DEVICE_NAME, string_argument_TinW[20];
static char our_tasklet_argument_WinT[20] = DEVICE_NAME, string_argument_WinT[20];
static char our_tasklet_argument_SinT[20] = DEVICE_NAME, string_argument_SinT[20];
static char our_tasklet_argument_TinS[20] = DEVICE_NAME, string_argument_TinS[20];

static void our_tasklet_function_TinW(unsigned long data){
	tasklet_trylock(&our_tasklet_TinW);
	strcpy(string_argument_TinW, data);
	after_delay_TinW = jiffies;
	printk(KERN_INFO "MODULE4: Tasklet in workqueue function of %s is running on CPU %d \n", string_argument_TinW, smp_processor_id());
	printk(KERN_INFO "MODULE4: Tasklet in workqueue function scheduled on: %ld \n", before_delay_TinW);
	printk(KERN_INFO "MODULE4: Tasklet in workqueue function executed on: %ld \n", after_delay_TinW);
	printk(KERN_ALERT "MODULE4: Tasklet in workqueue runtime: %ld \n", after_delay_TinW - before_delay_TinW);
	tasklet_unlock(&our_tasklet_TinW);
}

static void our_work_function_TinW(struct work_struct *our_work){
	tasklet_init(&our_tasklet_TinW, &our_tasklet_function_TinW, (unsigned long) &our_tasklet_argument_TinW);
	tasklet_schedule(&our_tasklet_TinW);
}

static DECLARE_DELAYED_WORK(our_work_TinW, our_work_function_TinW);

static void our_work_function_WinT(struct work_struct *our_work){
	after_delay_WinT = jiffies;
	printk(KERN_INFO "MODULE4: workqueue in tasklet function is running on CPU %d \n", smp_processor_id());
	printk(KERN_INFO "MODULE4: Work function is running on CPU %d \n", smp_processor_id());
	printk(KERN_INFO "MODULE4: workqueue in tasklet scheduled on: %ld \n", before_delay_WinT);
	printk(KERN_INFO "MODULE4: workqueue in tasklet executed on: %ld \n", after_delay_WinT);
	printk(KERN_ALERT "MODULE4: workqueue in tasklet runtime: %ld \n", after_delay_WinT - before_delay_WinT);
}

static DECLARE_DELAYED_WORK(our_work_WinT, our_work_function_WinT);

static void our_tasklet_function_WinT(unsigned long data){
	tasklet_trylock(&our_tasklet_WinT);
	strcpy(string_argument_WinT, data);
	our_workqueue_WinT = create_singlethread_workqueue("ourqueue1");
	if(!our_workqueue_WinT){
		printk(KERN_ALERT "MODULE4: Creating the workqueue has been failed!\n");
	}
	queue_delayed_work(our_workqueue_WinT, &our_work_WinT, HZ);
	tasklet_unlock(&our_tasklet_WinT);
}

static void our_work_function_SinW_inner(struct work_struct *our_work){
	after_delay_SinW = jiffies;
	printk(KERN_INFO "MODULE4: sharedqueue in workqueue function is running on CPU %d \n", smp_processor_id());
	printk(KERN_INFO "MODULE4: sharedqueue in workqueue scheduled on: %ld \n", before_delay_SinW);
	printk(KERN_INFO "MODULE4: sharedqueue in workqueue executed on: %ld \n", after_delay_SinW);
	printk(KERN_ALERT "MODULE4: sharedqueue in workqueue runtime: %ld \n", after_delay_SinW - before_delay_SinW);
}

static DECLARE_DELAYED_WORK(our_work_SinW_inner, our_work_function_SinW_inner);

static void our_work_function_SinW(struct work_struct *our_work){
	schedule_delayed_work(&our_work_SinW_inner,HZ);
}

static DECLARE_DELAYED_WORK(our_work_SinW, our_work_function_SinW);

static void our_work_function_WinS_inner(struct work_struct *our_work){
	after_delay_WinS = jiffies;
	printk(KERN_INFO "MODULE4: workqueue in sharedqueue function is running on CPU %d \n", smp_processor_id());
	printk(KERN_INFO "MODULE4: workqueue in sharedqueue scheduled on: %ld \n", before_delay_WinS);
	printk(KERN_INFO "MODULE4: workqueue in sharedqueue executed on: %ld \n", after_delay_WinS);
	printk(KERN_ALERT "MODULE4: workqueue in sharedqueue runtime: %ld \n", after_delay_WinS - before_delay_WinS);
}

static DECLARE_DELAYED_WORK(our_work_WinS_inner, our_work_function_WinS_inner);

static void our_work_function_WinS(struct work_struct *our_work){
	our_workqueue_WinS = create_singlethread_workqueue("ourqueue2");
	if(!our_workqueue_WinT){
		printk(KERN_ALERT "MODULE4: Creating the workqueue4 has been failed!\n");
	}
	queue_delayed_work(our_workqueue_WinS, &our_work_WinS_inner, HZ);
}

static DECLARE_DELAYED_WORK(our_work_WinS, our_work_function_WinS);


static void our_tasklet_function_TinS(unsigned long data){
	tasklet_trylock(&our_tasklet_TinS);
	strcpy(string_argument_TinS, data);
	after_delay_TinS = jiffies;
	printk(KERN_INFO "MODULE4: tasklet in sharedqueue scheduled on: %ld \n", before_delay_TinS);
	printk(KERN_INFO "MODULE4: tasklet in sharedqueue executed on: %ld \n", after_delay_TinS);
	printk(KERN_ALERT "MODULE4: tasklet in sharedqueue runtime: %ld \n", after_delay_TinS - before_delay_TinS);
	tasklet_unlock(&our_tasklet_TinS);
}

static void our_work_function_TinS(struct work_struct *our_work){
	tasklet_init(&our_tasklet_TinS, &our_tasklet_function_TinS, (unsigned long) &our_tasklet_argument_TinS);
	tasklet_schedule(&our_tasklet_TinS);
}

static DECLARE_DELAYED_WORK(our_work_TinS, our_work_function_TinS);

static void our_work_function_SinT(struct work_struct *our_work){
	after_delay_SinT = jiffies;
	printk(KERN_INFO "MODULE4: sharedqueue in tasklet function is running on CPU %d \n", smp_processor_id());
	printk(KERN_INFO "MODULE4: sharedqueue in tasklet scheduled on: %ld \n", before_delay_SinT);
	printk(KERN_INFO "MODULE4: sharedqueue in tasklet executed on: %ld \n", after_delay_SinT);
	printk(KERN_ALERT "MODULE4: sharedqueue in tasklet runtime: %ld \n", after_delay_SinT - before_delay_SinT);
}

static DECLARE_DELAYED_WORK(our_work_SinT, our_work_function_SinT);

static void our_tasklet_function_SinT(unsigned long data){
	tasklet_trylock(&our_tasklet_SinT);
	strcpy(string_argument_SinT, data);
	schedule_delayed_work(&our_work_SinT,HZ);
	tasklet_unlock(&our_tasklet_SinT);
}


static int __init module4_init(void){

	printk(KERN_INFO "MODULE4: Initialization.\n");

	// tasklet in workqueue	
	before_delay_TinW = jiffies;
	our_workqueue_TinW = create_singlethread_workqueue("ourqueue3");
	if(!our_workqueue_TinW){
		printk(KERN_ALERT "MODULE4: Creating the workqueue has been failed!\n");
		return -EFAULT;
	}
	queue_delayed_work(our_workqueue_TinW, &our_work_TinW, HZ);

	// sharedQueue in workqueue 
	before_delay_SinW = jiffies;
	our_workqueue_SinW = create_singlethread_workqueue("ourqueue4");
	if(!our_workqueue_SinW){
		printk(KERN_ALERT "MODULE4: Creating the workqueue3 has been failed!\n");
		return -EFAULT;
	}
	queue_delayed_work(our_workqueue_SinW, &our_work_SinW, HZ);

	// workqueue in tasklet	
	before_delay_WinT = jiffies;
	tasklet_init(&our_tasklet_WinT, &our_tasklet_function_WinT, (unsigned long) &our_tasklet_argument_WinT);
	tasklet_schedule(&our_tasklet_WinT);

	// sharedqueue in Tasklet
	before_delay_SinT = jiffies;
	tasklet_init(&our_tasklet_SinT, &our_tasklet_function_SinT, (unsigned long) &our_tasklet_argument_SinT);
	tasklet_schedule(&our_tasklet_SinT);

	// workqueue in sharedqueue 
 	before_delay_WinS = jiffies;
 	schedule_delayed_work(&our_work_WinS,HZ);

	// tasklet in sharedqueue  
	before_delay_TinS = jiffies;
	schedule_delayed_work(&our_work_TinS,HZ);

	return SUCCESS;
}


static void __exit module4_exit(void){
	cancel_delayed_work(&our_work_TinW);
	cancel_delayed_work(&our_work_WinT);
	cancel_delayed_work(&our_work_SinW);
	cancel_delayed_work(&our_work_SinW_inner);
	cancel_delayed_work(&our_work_WinS);
	cancel_delayed_work(&our_work_WinS_inner);
	cancel_delayed_work(&our_work_SinT);
	cancel_delayed_work(&our_work_TinS);
	
	if(our_workqueue_TinW)
		destroy_workqueue(our_workqueue_TinW);
	if(our_workqueue_WinT)
		destroy_workqueue(our_workqueue_WinT);
	if(our_workqueue_SinW)
		destroy_workqueue(our_workqueue_SinW);
	if(our_workqueue_WinS)
		destroy_workqueue(our_workqueue_WinS);

	tasklet_disable_nosync(&our_tasklet_TinW);
	tasklet_kill(&our_tasklet_TinW);
	tasklet_disable_nosync(&our_tasklet_WinT);
	tasklet_kill(&our_tasklet_WinT);
	tasklet_disable_nosync(&our_tasklet_SinT);
	tasklet_kill(&our_tasklet_SinT);
	tasklet_disable_nosync(&our_tasklet_TinS);
	tasklet_kill(&our_tasklet_TinS);

	printk(KERN_INFO "MODULE4: GoodBye.\n");
}

module_init(module4_init);
module_exit(module4_exit);
