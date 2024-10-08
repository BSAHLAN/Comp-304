#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/slab.h>

// Meta Information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ME");
MODULE_DESCRIPTION("A module that knows how to greet");

char *name;
int age;
int root_pid;
char* file_name;

/*
 * module_param(foo, int, 0000)
 * The first param is the parameters name
 * The second param is its data type
 * The final argument is the permissions bits,
 * for exposing parameters in sysfs (if non-zero) at a later stage.
 */

module_param(name, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(name, "name of the caller");

module_param(age, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(age, "age of the caller");

module_param(root_pid, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(root_pid, "pid of the root process");

module_param(file_name, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(file_name, "name of the output file");

//A function for traversing childs
void traverse_processes(struct task_struct *ts, int level){
     struct list_head *list;
     
     list_for_each(list, &ts->children) {

     struct task_struct *child_task = list_entry(list, struct task_struct, sibling);
     // TODO: Process the child_task
      printk("Child PID: %d, Child Name: %s, Child Start Time: %llu, Level: %d, Parent PID: %d\n", child_task->pid, child_task->comm, child_task->start_time, level, child_task->real_parent->pid);
      traverse_processes(child_task, level+1);

      }

}

// A function that runs when the module is first loaded
int simple_init(void) {
	struct task_struct *ts;
       // struct list_head *list;

	ts = get_pid_task(find_get_pid(root_pid), PIDTYPE_PID);

	printk("Hello from the kernel, user: %s, age: %d, root_pid: %d, output_file: %s\n", name, age, root_pid, file_name);
	printk("command: %s\n", ts->comm);


        /*list_for_each(list, &ts->children) {
            struct task_struct *child_task = list_entry(list, struct task_struct, sibling);
            // TODO: Process the child_task
	    
	    printk("Child PID: %d, Child Name: %s, Child Start Time: %llu", child_task->pid, child_task->comm, child_task->start_time);
	    
            }*/
	traverse_processes(ts, 1);

	return 0;
}

// A function that runs when the module is removed
void simple_exit(void) {
	printk("Goodbye from the kernel, user: %s, age: %d, root_pid: %d, output_file: %s\n", name, age, root_pid, file_name);
}

module_init(simple_init);
module_exit(simple_exit);

