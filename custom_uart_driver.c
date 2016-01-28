//STD ALONE DRV

// uart5: serial@481a8000 : LINUX
//UART4 0x481A_8000 0x481A_8FFF 4KB : TRM


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/blkdev.h>
#include <linux/cdev.h>
#include<linux/kfifo.h>
#include<asm/ioctl.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/err.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
//#include <linux/serial_core.h>
//#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_data/serial-omap.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/device.h>

MODULE_LICENSE("Dual BSD/GPL");

#define PSDEV_BUFFSIZE 100

static int N_devices = 2;
module_param(N_devices, int, S_IRUGO);


//CM_PER_UART4_CLKCTRL Register (offset = 78h) [reset = 30000h]
#define CM_PER_START_ADDR 0x44e00000
#define CM_PER_SIZE       0x400
#define CM_PER_UART4_CLKCTRL 0x78



//UART4 0x481A_8000 0x481A_8FFF 4KB UART4 Registers  - TRM

#define UART4_START     0x481a8000 
#define UART4_LEN       0x2000   


int custom_uart_driver_probe(struct platform_device *pdev)
{
//		uart_am33x_port up;
//		int ret;
//		int irq;
        void __iomem *membase;
		struct resource  *mem_res;
		void __iomem *cm_per;
		unsigned int uart5_clkctrl=0;

       cm_per = ioremap(CM_PER_START_ADDR, CM_PER_SIZE);
	   if(!cm_per)
	   {
			printk (KERN_ERR "HI: ERROR: Failed to remap memory for CM_PER.\n");
			return 0;
	   }
		uart5_clkctrl = ioread32(cm_per+CM_PER_UART4_CLKCTRL); 
		printk("\n uart5_clkctrl=%x\n",uart5_clkctrl);
		iowrite32(0x02,cm_per+CM_PER_UART4_CLKCTRL);    //Enable the clock for UART5 
		uart5_clkctrl = ioread32(cm_per+CM_PER_UART4_CLKCTRL); 
		printk("\n uart5_clkctrl=%x\n",uart5_clkctrl);
	    iounmap(cm_per);

		printk("in probe: of_get_uart_port_info\n");


        mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        if (!mem_res)
     {
                dev_err(&pdev->dev, "no mem resource?\n");
                return -ENODEV;
     }
       if (!devm_request_mem_region(&pdev->dev, mem_res->start, resource_size(mem_res),
                                pdev->dev.driver->name)) {
                dev_err(&pdev->dev, "memory region already claimed\n");
                return -EBUSY;
        }
        membase = devm_ioremap_nocache(&pdev->dev, mem_res->start,resource_size(mem_res));
        if (!membase)
            return -ENODEV;


	printk("LCR=%x\n",ioread16(membase+0X0C));
	iowrite16(0x00BF,membase+0X0C);
	printk("LCR=%x\n",ioread16(membase+0X0C));

	iowrite16( (ioread16(membase+0X0C))^0x01,membase+0X0C);
	printk("LCR=%x\n",ioread16(membase+0X0C));

	{
//	unsigned int reg_val=0;
	printk("MDR1=%x\n",ioread32(membase+0X20));
	iowrite32((ioread32(membase+0X20))^0x01,membase+0X20); // FCR : TX_FIFO_CLEAR,RX_FIFO_CLEAR,FIFO_1byte
	printk("MDR1=%x\n",ioread32(membase+0X20));
	} 

return 0;
}



int custom_uart_driver_remove(struct platform_device *drv)
{
	printk("in remove\n");
	return 0;
}



static const struct of_device_id of_custom_uart_match[] = {
        { .compatible = "uartx", },
        {},
};

struct platform_driver custom_uart_platform_driver = {
		.probe = custom_uart_driver_probe,
        .remove = custom_uart_driver_remove,
        .driver = {
		.name  = "custom_uart",
		.owner = THIS_MODULE, 
                .of_match_table = of_match_ptr(of_custom_uart_match),
	},
};


static dev_t uart_first;
static struct class *uart_class;	

static int __init custom_uart_drv_init(void)
{
	platform_driver_register(&custom_uart_platform_driver);
	return 0;

}

static void __exit  custom_uart_drv_exit(void)
{
	printk(KERN_ALERT "custom_uart_drv_exit\n");
	platform_driver_unregister(&custom_uart_platform_driver);   
	printk(KERN_INFO "custom_uart_drv: unloading.\n");
}
module_init(custom_uart_drv_init);
module_exit(custom_uart_drv_exit);

