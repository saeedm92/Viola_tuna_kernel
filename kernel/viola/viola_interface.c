#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/prints.h>
#include <linux/module.h>

#define IN_KERNEL	1

int reginfer_i2c_main(int iregaccess_p, int oregaccess_p);
int reginfer_gpio_main(int iregaccess_p, int oregaccess_p);
int viola_main(int regaccess_ptr, int master_iomem_ptr, int slave_iomem_ptr);

#ifdef IN_KERNEL
bool iomems_initialized = false;
#else
int iomems_initialized = 0;
#define PRINTK0(f, args...) printf("%s " f, __func__, ##args); 
#define PRINTK_ERR(f, args...) printf("%s " f, __func__, ##args); 
#endif
//char *gpio_iomem;
char *twl6040_iomem;
char *gn_led_iomem;

static int init_iomem(unsigned int devid)
{
	int i;

	//if (devid == 0x22) { /* gpio */
	//	//iomem_p = kmalloc(15 * sizeof(*iomem_p), GFP_KERNEL);
	//	//iomem_p = kmalloc(256 * sizeof(*iomem_p), GFP_KERNEL);
	//	gpio_iomem = kmalloc(256 * sizeof(char), GFP_KERNEL);
	//	if (!gpio_iomem) {
	//		PRINTK_ERR("Error: could not allocate gpio_iomem\n");
	//		return -ENOMEM;
	//	}
	//	for (i = 0; i < 256; i++)
	//		gpio_iomem[i] = 0xff;
	//}

	if (devid == 0x4b) { /* twl6040 */
		//iomem_p = kmalloc(47 * sizeof(*iomem_p), GFP_KERNEL);
		//iomem_p = kmalloc(256 * sizeof(*iomem_p), GFP_KERNEL);
#ifdef IN_KERNEL
		twl6040_iomem = kmalloc(256 * sizeof(char), GFP_KERNEL);
#else
		twl6040_iomem = malloc(256 * sizeof(char));
#endif
		if (!twl6040_iomem) {
			PRINTK_ERR("Error: could not allocate twl6040_iomem\n");
#ifdef IN_KERNEL
			return -ENOMEM;
#else
			return -1;
#endif
		}
		for (i = 0; i < 256; i++)
			twl6040_iomem[i] = 0x00;
	}

	if (devid == 0x30) { /* twl6040 */
		//iomem_p = kmalloc(47 * sizeof(*iomem_p), GFP_KERNEL);
		//iomem_p = kmalloc(256 * sizeof(*iomem_p), GFP_KERNEL);
#ifdef IN_KERNEL
		gn_led_iomem = kmalloc(256 * sizeof(char), GFP_KERNEL);
#else
		gn_led_iomem = malloc(256 * sizeof(char));
#endif
		if (!twl6040_iomem) {
			PRINTK_ERR("Error: could not allocate gn_led_iomem\n");
#ifdef IN_KERNEL
			return -ENOMEM;
#else
			return -1;
#endif
		}
		for (i = 0; i < 256; i++)
			gn_led_iomem[i] = 0x00;
		gn_led_iomem[2] = 0x40; 
	}

	return 0;
}

int pass_to_omap_gpio(char *iregaccess, char *oregaccess)
{
	int ret;

	//PRINTK0("iregaccess[0] = %#x, iregaccess[1] = %#x\n", iregaccess[0], iregaccess[1]);
	ret = reginfer_gpio_main((int) iregaccess, (int) oregaccess);
	//PRINTK0("ret = %d\n", ret);
	//if (!ret)
	//	PRINTK0("oregaccess[0] = %#x, oregaccess[1] = %#x, oregaccess[2] = %#x, oregaccess[3] = %#x\n\n",
	//		  oregaccess[0], oregaccess[1], oregaccess[2], oregaccess[3]);

	return ret;
}

int pass_to_omap_i2c(char *iregaccess, char *oregaccess)
{
	int ret;

	//PRINTK0("iregaccess[0] = %#x, iregaccess[1] = %#x\n", iregaccess[0], iregaccess[1]);
	ret = reginfer_i2c_main((int) iregaccess, (int) oregaccess);
	//PRINTK0("ret = %d\n", ret);
        //if (!ret)
	//	PRINTK0("oregaccess[0] = %#x, oregaccess[1] = %#x, oregaccess[2] = %#x, oregaccess[3] = %#x\n",
	//			  oregaccess[0], oregaccess[1], oregaccess[2], oregaccess[3]);

	return ret;
}

int check_bus_reg_access(char regoff, char regval, char *iregaccess,
						   char *oregaccess, char *oregaccess2)
{
	int ret = -1;

	iregaccess[0] = (char) regoff; iregaccess[1] = (char) regval;
	ret = pass_to_omap_i2c(iregaccess, oregaccess);
	if (!ret)
		ret = pass_to_omap_gpio(oregaccess, oregaccess2);

	return ret;
}

/* I2C */
char iregaccess[2];
char oregaccess[4];
char oregaccess2[4];

int viola_check_reg_access(char regoff, char regval)
{
	char regaccess[3], _devid, _regoff, _regval;
	int ret = 0, ret2 = -1;

	//PRINTK0("[1]: devid = %#x, reg = %#x, val = %#x\n", devid, reg, (unsigned int) val);

	//if (devid > 255) {
	//	PRINTK_ERR("Error: devid is too large (%#x)\n", devid);
	//	return -EINVAL;
	//}

	//if (reg > 255) {
	//	PRINTK_ERR("Error: reg is too large (%#x)\n", reg);
	//	return -EINVAL;
	//}

	if (!iomems_initialized) {
		//init_iomem(0x22); //omap5432 led
		init_iomem(0x4b); //twl6040
		init_iomem(0x30); //galaxy nexus led
		//initialize to zero
		oregaccess[0] = (char) 0x00;
		oregaccess[1] = (char) 0x00;
		oregaccess[2] = (char) 0x00;
		oregaccess[3] = (char) 0x00;
		oregaccess2[0] = (char) 0x00;
		oregaccess2[1] = (char) 0x00;
		oregaccess2[2] = (char) 0x00;
		oregaccess2[3] = (char) 0x00;
#ifdef IN_KERNEL
		iomems_initialized = true;
#else
		iomems_initialized = 1;
#endif
	}


	ret2 = check_bus_reg_access(regoff, regval, iregaccess, oregaccess, oregaccess2);

	if (ret2)
		return 0; //allow
	
	//register access
	regaccess[0] = (char) oregaccess[2]; //devid
	regaccess[1] = (char) oregaccess2[0]; //reg_off
	regaccess[2] = (char) oregaccess[1]; //reg_val

	//PRINTK0("[2]: twl6040_iomem[reg] = %#x, gpio_iomem[reg] = %#x\n",
	//			(unsigned int) twl6040_iomem[reg],
	//			(unsigned int) gpio_iomem[reg]);

	ret = viola_main((int) twl6040_iomem, (int) gn_led_iomem, (int) regaccess);
	
	PRINTK0("[3] viola_main returned %d\n", ret);
	_devid = (char) oregaccess[2]; //devid
	_regoff = (char) oregaccess2[0]; //reg_off
	_regval = (char) oregaccess[1]; //reg_val
	PRINTK0("[3.1]: _devid = %#x, _regoff = %#x, _regval = %#x\n",
					(unsigned int) _devid,
					(unsigned int) _regoff,
					(unsigned int) _regval);
	//if (_devid == 0x4b) {
	//	int ii = 0;
	//	for (ii = 0; ii < 10; ii++)
	//		PRINTK0("gn_led_iomem[%d] = %#x\n", ii, (unsigned int) gn_led_iomem[ii]);
	//	ii = 0;
	//	for (ii = 8; ii < 13; ii++)
	//		PRINTK0("twl6040_iomem[%d] = %#x\n", ii, (unsigned int) twl6040_iomem[ii]);
	//}
	
	if (!ret) {
		if (_devid == 0x30) {
			gn_led_iomem[_regoff] = (char) _regval;
			PRINTK0("[4]\n");
		}
		if (_devid == 0x4b) { /* twl6040 */
			twl6040_iomem[_regoff] = (char) _regval;
			PRINTK0("[5]\n");
		}
		//PRINTK0("[6]: twl6040_iomem[reg] = %#x, gpio_iomem[reg] = %#x\n",
		//		(unsigned int) twl6040_iomem[reg],
		//		(unsigned int) gpio_iomem[reg]);
	}

	return ret;

}
EXPORT_SYMBOL(viola_check_reg_access);
