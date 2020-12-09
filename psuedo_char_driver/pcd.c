#include<linux/fs.h>
#include<linux/module.h>
#include<linux/cdev.h>
#include<linux/kernel.h>
#include<linux/export.h>
#include<linux/kdev_t.h>
#include<linux/device.h>
#include<linux/uaccess.h>

#define DEV_MEM_SIZE 512
#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__
char device_buffer[DEV_MEM_SIZE];
dev_t device_number;
struct file_operations pcd_fops;
struct cdev pcd_cdev;
struct class *class_pcd;
struct device *device_pcd;

int pcd_open(struct inode *inode, struct file *filp)
{
		pr_info("In pcd_open\n");
		return 0;
}
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
		pr_info("Write requested fo %zu bytes\n",count);
		pr_info("current file position = %lld\n",*f_pos);
		if((*f_pos+count)>DEV_MEM_SIZE)
			count=DEV_MEM_SIZE-*f_pos;

		if(copy_from_user(&device_buffer[*f_pos],buff,count))
			return -EFAULT;
		*f_pos+=count;
		pr_info("number of bytes sucessfully written = %zu bytes\n",count);
		pr_info("updated file position = %lld\n",*f_pos);
		return count;

}
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
		pr_info("Read requested fo %zu bytes\n",count);
		pr_info("current file position = %lld\n",*f_pos);
		if((*f_pos+count)>DEV_MEM_SIZE)
			count=DEV_MEM_SIZE-*f_pos;

		if(copy_to_user(buff,&device_buffer[*f_pos],count))
			return -EFAULT;
		*f_pos+=count;
		pr_info("number of bytes sucessfully read = %zu bytes\n",count);
		pr_info("updated file position = %lld\n",*f_pos);
		return count;

}
loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{
		loff_t temp;
		pr_info("lseek requested\n");
		pr_info("current file position = %lld\n",filp->f_pos);
		
		switch(whence)
		{
			case SEEK_SET:
				if((offset>DEV_MEM_SIZE)|| (offset<0))
					return -EINVAL;
				filp->f_pos = offset;
				break;
			case SEEK_CUR:
				temp=filp->f_pos + offset;
				if((temp>DEV_MEM_SIZE)|| (temp<0))
					return -EINVAL;
				filp->f_pos += offset;
				break;
			case SEEK_END:
				filp->f_pos = DEV_MEM_SIZE+offset;
				break;
			default:
				return -EINVAL;
		}
		pr_info("updated file position = %lld\n",filp->f_pos);
		return filp->f_pos;
}
int pcd_release(struct inode *inode, struct file *filp)
{
		pr_info("pcd release function\n");
		return 0;
}


struct file_operations pcd_fops=
{
	.open = pcd_open,
	.release = pcd_release,
	.read = pcd_read,
	.write = pcd_write,
	.owner = THIS_MODULE
};

int static __init pcd_init(void)
{
	int ret;
	ret=alloc_chrdev_region(&device_number,0,8,"pcd_devices");
	if(ret<0){
		pr_err("Alloc chrdev failed");
		goto out;
	}
	pr_info("%s :Device number <Major>:<Minor> = %d:%d\n",__func__,MAJOR(device_number),MINOR(device_number));

	cdev_init(&pcd_cdev,&pcd_fops);
	pcd_cdev.owner = THIS_MODULE;

	ret=cdev_add(&pcd_cdev,device_number,1);
		if(ret<0){
			pr_err("cdev creation failed\n");
			goto unreg_chrdev;
		}

	class_pcd=class_create(THIS_MODULE,"pcd_class");
	if(IS_ERR(class_pcd))
	{
		pr_err("class creation failed\n");
		ret=PTR_ERR(class_pcd);
		goto cdev_del;
	}
	device_pcd=device_create(class_pcd,NULL,device_number,NULL,"pcd");
	if(IS_ERR(device_pcd))
	{
		pr_err("device creation failed\n");
		ret=PTR_ERR(device_pcd);
		goto class_del;
	}
	pr_info("In pcd_init");
	return 0;
class_del:
	class_destroy(class_pcd);
cdev_del:
	cdev_del(&pcd_cdev);
unreg_chrdev:
	unregister_chrdev_region(device_number,1);
out:
	return ret;
}

void static __exit pcd_exit(void)
{
	printk("In pcd_exit");
	device_destroy(class_pcd,device_number);
	class_destroy(class_pcd);
	cdev_del(&pcd_cdev);
	unregister_chrdev_region(device_number,1);
	pr_info("module unloaded\n");
}

module_init(pcd_init);
module_exit(pcd_exit);

MODULE_AUTHOR("BHARATH");
MODULE_DESCRIPTION("PSEUDO CHARACTER DRIVER");
MODULE_LICENSE("GPL");