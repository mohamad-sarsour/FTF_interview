#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/sched.h>


asmlinkage long sys_hello(void) {
	printk("Hello, World!\n");
	return 0;
}

//////////////////////////////////  7.1  ///////////////////////////////

asmlinkage long sys_set_weight(int weight){
  if(weight < 0){
    return -EINVAL;
  }
  current->weight = weight;
  return 0;
}

//////////////////////////////////  7.2  ///////////////////////////////

asmlinkage long sys_get_weight(void){
	return current->weight;
}

//////////////////////////////////  7.3  ///////////////////////////////

long traverse_children_sum_weight(struct task_struct *root_task){

  struct task_struct *task;
  struct list_head *list;
  long sum = root_task->weight;
 
  list_for_each(list, &root_task->children){
    task = list_entry(list, struct task_struct, sibling);
    sum += traverse_children_sum_weight(task);
  }
  return sum;
}

asmlinkage long sys_get_children_sum(void){
  if(list_empty(&current->children)){
	 return -ECHILD;  
  }
  return traverse_children_sum_weight(current)-current->weight;
}

//////////////////////////////////  7.4  ///////////////////////////////

asmlinkage pid_t sys_get_heaviest_ancestor(void){
	pid_t max_p=current->pid;
	int max_w=current->weight;
	struct task_struct *task=current->real_parent;
	while(task->pid!=0){
		if(task->weight > max_w ){
		     max_w=task->weight;
		     max_p=task->pid;
		}
		task=task->real_parent;
	}
	return max_p;
}