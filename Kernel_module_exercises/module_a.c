#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_DESCRIPTION("Kernel module A");
MODULE_AUTHOR("ManhTD24");
MODULE_LICENSE("GPL");

int x = 2002;
EXPORT_SYMBOL(x);

static int module_a_init(void) {
    printk(KERN_INFO "Module A loaded. Exported variable x = %d\n", x);
    return 0;
}

static void module_a_exit(void) {
    printk(KERN_INFO "Module A unloaded.\n");
}

module_init(module_a_init);
module_exit(module_a_exit);
