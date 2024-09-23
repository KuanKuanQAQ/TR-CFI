#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */
#include <linux/kallsyms.h>
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/kthread.h>

#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

#include <trampoline.h>

#define RANDPERIOD 2000

static struct task_struct *kthread = NULL;

struct perf_event * __percpu *wp;
struct perf_event * __percpu *cpu_events;
struct perf_event_attr attr;

static char ksym_name[KSYM_NAME_LEN] = "trampoline";
module_param_string(ksym, ksym_name, KSYM_NAME_LEN, S_IRUGO);

static void watchpoint_handler(struct perf_event *bp,
			       struct perf_sample_data *data,
			       struct pt_regs *regs) {
	// printk(KERN_INFO "%s value is read\n", ksym_name);
	// dump_stack();
	printk(KERN_INFO "Dump stack from sample_hbp_handler\n");
	return;
}

int work_func(void *trampoline) {
	/*
	读写操作次数 1000，单位 ns
	实验：开启 watchpoint，对 trampolines 做读写，每次读写前关闭 watchpoint，读写后重新开启 45527488 42475872 54561136 42120944 42446912 45426470.4  45426.4704
	     关闭 watchpoint，对 trampolines 做读写，                                     10240    10464    11632    11312    10336    10796.8     10.7968   0.682%
		 开启 watchpoint，对未监视区域做读写，每次读写前不需要关闭 watchpoint              10288    11440    10240    11856    10528    10870.4     10.8704
		 开启 watchpoint，对 trampolines 做读写，在所有读写前关闭 watchpoint，所有读写后重新开启，只操作一次 watchpoint
		 																	    ,  2295440  2445024  2412240  2169936  2171184  2298764.8   2298.7648
	*/
	int i;
	ktime_t time;
	// int cpu = get_cpu();
	u64 *trampolines = (u64 *)trampoline;
	
	printk(KERN_INFO "Visiting trampolines\n");
	time = ktime_get();
	// unregister_hw_breakpoint(per_cpu(*wp, cpu));
	for (i = TRAMPOLINE_LEN - 1; i >= 0; i--) {
		printk(KERN_INFO "%dth entry in trampolines ", i);
		printk(KERN_INFO "0x%x\n", trampolines[i]);
		// trampolines[i] = 0xd61f0000;
	}
	// wp = perf_event_create_kernel_counter(&attr, cpu, NULL, watchpoint_handler, context);
	// if (IS_ERR(wp)) return PTR_ERR(wp);
	// per_cpu(*cpu_events, cpu) = wp;
	printk(KERN_INFO "Total time: %lld\n", ktime_get() - time);
	
	return 0;
}


static int __init watchpoint_init(void) {
	int ret;
	void *addr = __symbol_get(ksym_name);

	if (!addr) return -ENXIO;
	printk(KERN_INFO "ksym %s in %p", ksym_name, addr);

	hw_breakpoint_init(&attr);
	attr.bp_mask = 11;
	attr.bp_addr = (unsigned long)addr;
	attr.bp_len = HW_BREAKPOINT_LEN_8;
	attr.bp_type = HW_BREAKPOINT_R;

	wp = register_wide_hw_breakpoint(&attr, watchpoint_handler, NULL);
	if (IS_ERR((void __force *)wp)) {
		ret = PTR_ERR((void __force *)wp);
        printk(KERN_INFO "Watchpoint registration failed\n");
        return ret;
	}
	printk(KERN_INFO "Watchpoint for %s installed %p\n", ksym_name, addr);

	/* Start worker kthread */
	kthread = kthread_run(work_func, (void *)addr, "tl_writer");
	if(kthread == ERR_PTR(-ENOMEM)){
		pr_err("Could not run kthread\n");
		return -1;
	}
	printk(KERN_INFO "Kernel thread start\n");
	
	return 0;
}

static void __exit watchpoint_exit(void) {
	unregister_wide_hw_breakpoint(wp);
	symbol_put(ksym_name);
	printk(KERN_INFO "Watchpoint for %s write uninstalled\n", ksym_name);
}

module_init(watchpoint_init);
module_exit(watchpoint_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kuan");
MODULE_DESCRIPTION("Watchpoint");
