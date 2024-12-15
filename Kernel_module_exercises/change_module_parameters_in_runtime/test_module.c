#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_DESCRIPTION("Kernel module A");
MODULE_AUTHOR("ManhTD24");
MODULE_LICENSE("GPL");

// Default parameters
static int int_param = 10;               
static char *string_param = "default";   

// Assign parameter
/**
 * 0660 allows user to read/ write parameter in 
 * /sys/module/<module_name>/parameters/.
 */
module_param(int_param, int, 0660);    
module_param(string_param, charp, 0660);

// Parameter description
MODULE_PARM_DESC(int_param, "An integer parameter");
MODULE_PARM_DESC(string_param, "A string parameter");

// Init
static int __init param_module_init(void) {
  printk(KERN_INFO "Kernel Module Loaded with params:\n");
  printk(KERN_INFO "int_param = %d\n", int_param);
  printk(KERN_INFO "string_param = %s\n", string_param);
  return 0;
}

// Exit
static void __exit param_module_exit(void) {
  printk(KERN_INFO "Kernel Module Unloaded\n");
}

module_init(param_module_init);
module_exit(param_module_exit);

