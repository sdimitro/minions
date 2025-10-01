// SPDX-License-Identifier: GPL-2.0
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Serapheim Dimitropoulos");
MODULE_DESCRIPTION("A fake virtual accelerator");
MODULE_VERSION("0.1");

/* Module parameters for configurable base address */
static unsigned long virtax_base = 0x10000000;
module_param(virtax_base, ulong, 0444);
MODULE_PARM_DESC(virtax_base, "Base address for VirtAx registers (default: 0x10000000)");

static dev_t virtax_dev_number;
static struct class *virtax_class;
static struct cdev virtax_cdev;

static int is_busy;

/* Memory-mapped register simulation */
static void __iomem *io_reg_base;
static unsigned long virtax_phys_addr;
#define IO_REG_OFFSET   0x00

/* Register access functions */
static u32 virtax_read_reg(void)
{
	return ioread32(io_reg_base + IO_REG_OFFSET);
}

static void virtax_write_reg(u32 value)
{
	iowrite32(value, io_reg_base + IO_REG_OFFSET);
}

static int virtax_open(struct inode *inode, struct file *file)
{
	pr_debug("VirtAx: Opened.\n");
	return 0;
}

static int virtax_release(struct inode *inode, struct file *file)
{
	pr_debug("VirtAx: Released.\n");
	return 0;
}

static ssize_t virtax_read(struct file *file, char __user *buf,
		size_t count, loff_t *pos)
{
	u32 reg_value;
	int ret;

	if (count < sizeof(u32))
		return -EINVAL;

	reg_value = virtax_read_reg();
	if (reg_value != 0) {
		virtax_write_reg(0x0);
		is_busy = 0;
	} else {
		reg_value = 0x0;
	}

	ret = copy_to_user(buf, &reg_value, sizeof(u32));
	if (ret)
		return -EFAULT;


	pr_debug("VirtAx: Read register value 0x%08x\n", reg_value);
	return sizeof(u32);
}

static ssize_t virtax_write(struct file *file, const char __user *buf,
		size_t count, loff_t *pos)
{
	u32 reg_value;
	int ret;

	if (count < sizeof(u32))
		return -EINVAL;

	ret = copy_from_user(&reg_value, buf, sizeof(u32));
	if (ret)
		return -EFAULT;

	if (is_busy)
		return -EBUSY;

	virtax_write_reg(reg_value);
	is_busy = 1;

	pr_debug("VirtAx: Wrote register value 0x%08x\n", reg_value);
	return sizeof(u32);
}

static const struct file_operations virtax_fops = {
	.owner = THIS_MODULE,
	.open = virtax_open,
	.release = virtax_release,
	.read = virtax_read,
	.write = virtax_write,
};

static int __init virtax_init(void)
{
	int ret;

	pr_info("VirtAx: Initializing the module.\n");

	/* Allocate a page of memory and get its physical address */
	virtax_phys_addr = __get_free_page(GFP_KERNEL);
	if (!virtax_phys_addr) {
		pr_err("VirtAx: Failed to allocate memory for registers\n");
		return -ENOMEM;
	}

	/* Convert virtual address to physical address */
	virtax_phys_addr = virt_to_phys((void *)virtax_phys_addr);

	/* Map the physical memory as I/O memory */
	io_reg_base = ioremap(virtax_phys_addr, 4096);
	if (!io_reg_base) {
		pr_err("VirtAx: Failed to map I/O memory at 0x%lx\n", virtax_phys_addr);
		free_page(virt_to_phys((void *)virtax_phys_addr));
		return -ENOMEM;
	}
	pr_info("VirtAx: Allocated and mapped I/O memory at %p (physical: 0x%lx)\n",
		io_reg_base, virtax_phys_addr);

	ret = alloc_chrdev_region(&virtax_dev_number, 0, 1, "virtax");
	if (ret < 0) {
		pr_err("VirtAx: Failed to allocate device number.\n");
		iounmap(io_reg_base);
		free_page((unsigned long)phys_to_virt(virtax_phys_addr));
		return ret;
	}
	pr_info("VirtAx: Allocated device number %d:%d\n",
			MAJOR(virtax_dev_number),
			MINOR(virtax_dev_number));

	virtax_class = class_create("virtax");
	if (IS_ERR(virtax_class)) {
		pr_err("VirtAx: Failed to create device class.\n");
		unregister_chrdev_region(virtax_dev_number, 1);
		iounmap(io_reg_base);
		free_page((unsigned long)phys_to_virt(virtax_phys_addr));
		return PTR_ERR(virtax_class);
	}

	cdev_init(&virtax_cdev, &virtax_fops);
	virtax_cdev.owner = THIS_MODULE;
	ret = cdev_add(&virtax_cdev, virtax_dev_number, 1);
	if (ret < 0) {
		pr_err("VirtAx: Failed to add cdev.\n");
		class_destroy(virtax_class);
		unregister_chrdev_region(virtax_dev_number, 1);
		iounmap(io_reg_base);
		free_page((unsigned long)phys_to_virt(virtax_phys_addr));
		return ret;
	}

	if (IS_ERR(device_create(virtax_class, NULL, virtax_dev_number,
					NULL, "virtax"))) {
		pr_err("VirtAx: Failed to create device node.\n");
		cdev_del(&virtax_cdev);
		class_destroy(virtax_class);
		unregister_chrdev_region(virtax_dev_number, 1);
		iounmap(io_reg_base);
		free_page((unsigned long)phys_to_virt(virtax_phys_addr));
		return -1;
	}
	return 0;
}

static void __exit virtax_exit(void)
{
	device_destroy(virtax_class, virtax_dev_number);
	cdev_del(&virtax_cdev);
	class_destroy(virtax_class);
	unregister_chrdev_region(virtax_dev_number, 1);
	iounmap(io_reg_base);
	free_page((unsigned long)phys_to_virt(virtax_phys_addr));
	pr_info("VirtAx: Module exited.\n");
}

module_init(virtax_init);
module_exit(virtax_exit);
