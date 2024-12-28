#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ForgotAlready");

#define procfs_name "tsu"

static struct proc_dir_entry *proc_file = NULL;

static ssize_t procfile_read(struct file *file_pointer, char __user *buffer, size_t buffer_length, loff_t *offset)
{
    struct timespec64 ts;
    struct tm tm;

    ktime_get_real_ts64(&ts);
    time64_to_tm(ts.tv_sec, 0, &tm);
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_mday = 1;
    tm.tm_mon = 0;
    tm.tm_year += 1;
    long new_year_time = mktime64(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    long minutes_until_new_year = (new_year_time - ts.tv_sec) / 60;
    long attempts = minutes_until_new_year / 15;

    char output[128];
    int len = snprintf(output, sizeof(output), "Number of attempts to pass lab until New Year: %ld\n", attempts);

    if (*offset > 0 || buffer_length < len) 
    {
        return 0;
    }

    if (copy_to_user(buffer, output, len)) 
    {
        return -EFAULT;
    }

    *offset = len;
    return len;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
    static const struct proc_ops proc_file_fops = {
        .proc_read = procfile_read,
    };
#else
    static const struct file_operations proc_file_fops = {
        .read = procfile_read,
    };
#endif


static int __init this_module_init(void)
{
    proc_file = proc_create(procfs_name, 0644, NULL, &proc_file_fops);
    return 0;
}

static void __exit this_module_exit(void)
{
    proc_remove(proc_file);
    pr_info("/proc/%s removed\n", procfs_name);
}

module_init(this_module_init);
module_exit(this_module_exit);