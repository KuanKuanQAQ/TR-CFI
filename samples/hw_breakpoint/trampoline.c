#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */

#include <trampoline.h>

u64 trampoline[TRAMPOLINE_LEN];
EXPORT_SYMBOL_GPL(trampoline);

static int __init trampoline_init(void) {
	printk(KERN_INFO "Trampolines mod loaded\n");
	return 0;
}

static void __exit trampoline_exit(void) {
	printk(KERN_INFO "Trampolines mod unloaded\n");
}

module_init(trampoline_init);
module_exit(trampoline_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kuan");
MODULE_DESCRIPTION("Trampoline");
