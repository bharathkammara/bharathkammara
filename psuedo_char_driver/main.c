#include<linux/module.h>
#include<linux/fs.h>
#define MEM_SIZE 512

//pseudo device memory
char device_buffer[MEM_SIZE];
dev_t device_number;

static int __init pcd_driver_init(void)
{
	//1.dynamically allocate a device number
	alloc_chedev_region(&device_number,0,1,"pcd");

	return 0;
}

static void __exit pcd_driver_cleanup(void)
{
	
}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);
