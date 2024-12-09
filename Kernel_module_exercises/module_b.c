#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_DESCRIPTION("Kernel module B");
MODULE_AUTHOR("ManhTD24");
MODULE_LICENSE("GPL");

extern int x; // Declare the external variable

static int module_b_init(void) {
    printk(KERN_INFO "Module B loaded. Accessed variable x = %d\n", x);
    return 0;
}

static void module_b_exit(void) {
    printk(KERN_INFO "Module B unloaded.\n");
}

module_init(module_b_init);
module_exit(module_b_exit);
