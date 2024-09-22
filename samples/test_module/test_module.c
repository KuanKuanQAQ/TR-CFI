// test_module.c
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kuan");
MODULE_DESCRIPTION("A simple test module");
MODULE_VERSION("1.0");

typedef void (*func_ptr) (void);

int  __attribute__ ((visibility("hidden"))) printk_trampoline_out(const char *fmt, ...)
{
    va_list args;
    int r;

    va_start(args, fmt);
    r = vprintk(fmt, args);
    va_end(args);

    return r;
}

void func1(void) {
    printk_trampoline_out(KERN_INFO "\tthis is func1\n");
}
void func2(void) {
    printk_trampoline_out(KERN_INFO "\tthis is func2\n");
}
void func3(void) {
    printk_trampoline_out(KERN_INFO "\tthis is func3\n");
}
void func4(void) {
    printk_trampoline_out(KERN_INFO "\tthis is func4\n");
}

static int __init test_module_init(void);

void test_trampoline_in(int argv);

void  __attribute__ ((__noinline__, visibility("hidden"))) test(int argv) {
    void *caller = __builtin_return_address(0);

    // TODO: check caller is in test_trampoline_in
    if (0) {
        printk_trampoline_out(KERN_INFO "\tcalled by test_trampoline_in.\n");
    }

    func_ptr jt[4] = {func1, func2, func3, func4};
    jt[argv]();
}

void __attribute__ ((__noinline__)) test_trampoline_in(int argv) {
    test(argv);
}

static int __init test_module_init(void) {
    printk_trampoline_out(KERN_INFO "Test module loaded.\n");
    for (int i = 0; i < 4; ++i) 
        test_trampoline_in(i);
    return 0;
}

static void __exit test_module_exit(void) {
    printk_trampoline_out(KERN_INFO "Test module unloaded.\n");
}

module_init(test_module_init);
module_exit(test_module_exit);