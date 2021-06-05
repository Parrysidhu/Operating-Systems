#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
MODULE_LICENSE("GPL");

static int buff_size;
module_param(buff_size, int, S_IRUGO);

#define DEVICE_NAME "linepipe"

static DEFINE_SEMAPHORE(full);
static DEFINE_SEMAPHORE(empty);
static DEFINE_MUTEX(mutex);
static struct file_operations my_fops;
static struct miscdevice my_device;

static char **my_data;
int ret, ret_val, position = 0, position1 = 0, size_message, read_size_message, ret_access_ok;

static int my_open(struct inode *inode, struct file *file) {
	printk(KERN_ALERT "Misc device opened");
	return 0;
}

static ssize_t my_read(struct file *file, char __user * out, size_t size, loff_t * off) {
	if(down_interruptible (&full) < 0)
		return -1;
	if(mutex_lock_interruptible (&mutex) < 0)
		return 0;
	if(position1 == buff_size)
		position1 = 0;
	ret_access_ok = access_ok(VERIFY_WRITE, out, sizeof(*out));
	if (!ret_access_ok)
		return -1;

	ret = copy_to_user(out, my_data[position1], size);
	if (ret > 0)
		return -1;

	read_size_message = strlen(my_data[position1]);
	position1++;
	mutex_unlock(&mutex);
	up(&empty);

	return read_size_message;
}

static ssize_t my_write(struct file *file, const char __user * out, size_t size, loff_t * off) {
	if(down_interruptible(&empty) < 0)
		return -1;
	if(mutex_lock_interruptible(&mutex) < 0)
		return -1;
	if(position == buff_size)
		position = 0;
	ret_access_ok = access_ok(VERIFY_READ, out, sizeof(*out));
	if (!ret_access_ok)
		return -1;
	ret = copy_from_user(my_data[position], out, size);
	size_message = strlen(my_data[position]);
	position++;
	mutex_unlock(&mutex);
	up(&full);
	return size_message;
}

int __init init_module(void) {
	int counter;
	sema_init(&empty, buff_size);
	sema_init(&full, 0);
	mutex_init(&mutex);

	my_fops.read = my_read;
	my_fops.write = my_write;
	my_fops.open = my_open;

	my_device.minor = MISC_DYNAMIC_MINOR;
	my_device.name = "linepipe";
	my_device.fops = &my_fops;
	ret_val = misc_register(&my_device);

	if(ret_val < 0) {
		printk(KERN_ALERT "Creating misc device failed: %d\n", ret_val);
		return ret_val;
	}
	// GFP_KERNEL This is a normal allocation and allows process to sleep
	my_data = kmalloc(sizeof(char *) * (buff_size), GFP_KERNEL);

	for(counter = 0; counter < buff_size; counter++)
		my_data[counter] = kmalloc(sizeof(char *) * (100), GFP_KERNEL);

	printk(KERN_ALERT "Module initialized\n");
	return 0;
}

void __exit cleanup_module(void) {
	printk(KERN_ALERT "Module exit\n");
	kfree(my_data);
	misc_deregister(&my_device);
}