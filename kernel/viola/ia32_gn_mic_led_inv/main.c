#include "stdio.h"


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

int main()
{


	/* Galaxy Nexus trace */
	// set imax
	viola_check_reg_access((char) 0xa,  (char) 0x0);
	viola_check_reg_access((char) 0xd,  (char) 0x9);
	viola_check_reg_access((char) 0xe,  (char) 0x9);
	viola_check_reg_access((char) 0xf,  (char) 0x3);
	viola_check_reg_access((char) 0x6,  (char) 0x4747);
	viola_check_reg_access((char) 0x9,  (char) 0x215);
	viola_check_reg_access((char) 0x4,  (char) 0x636f);
	viola_check_reg_access((char) 0xa,  (char) 0x8000);
	viola_check_reg_access((char) 0x17, (char) 0x6fff);
	viola_check_reg_access((char) 0x16, (char) 0x601f);
	viola_check_reg_access((char) 0xc,  (char) 0x30);
	viola_check_reg_access((char) 0x7,  (char) 0x2);
	viola_check_reg_access((char) 0x6,  (char) 0x4747);
	viola_check_reg_access((char) 0xa,  (char) 0x8603);
	viola_check_reg_access((char) 0x2,  (char) 0x0);
	viola_check_reg_access((char) 0x8,  (char) 0x2);
	viola_check_reg_access((char) 0x8,  (char) 0xc0);
	viola_check_reg_access((char) 0x2,  (char) 0x4000);
	viola_check_reg_access((char) 0x2,  (char) 0x4);
	viola_check_reg_access((char) 0x2,  (char) 0x4);
	viola_check_reg_access((char) 0x17, (char) 0x6fff);
	
	// write all
	viola_check_reg_access((char) 0xa, (char) 0x0);
	viola_check_reg_access((char) 0xd, (char) 0x9);
	viola_check_reg_access((char) 0xe, (char) 0x9);
	viola_check_reg_access((char) 0xf, (char) 0x3);
	viola_check_reg_access((char) 0x6, (char) 0x4747);
	viola_check_reg_access((char) 0x9, (char) 0x215);
	viola_check_reg_access((char) 0x4, (char) 0x636f);
	viola_check_reg_access((char) 0xa, (char) 0x8000);
	viola_check_reg_access((char) 0x17,(char)  0x6fff);
	viola_check_reg_access((char) 0x16,(char)  0x601f);
	viola_check_reg_access((char) 0xc, (char) 0x30);
	viola_check_reg_access((char) 0x7, (char) 0x14);
	viola_check_reg_access((char) 0x6, (char) 0x4747);
	viola_check_reg_access((char) 0xa, (char) 0x8603);
	viola_check_reg_access((char) 0x2, (char) 0x0);
	viola_check_reg_access((char) 0x8, (char) 0x82);
	viola_check_reg_access((char) 0x8, (char) 0xc0);
	viola_check_reg_access((char) 0x8, (char) 0xff);
	viola_check_reg_access((char) 0x8, (char) 0x0);
	viola_check_reg_access((char) 0x8, (char) 0x0);
	viola_check_reg_access((char) 0x8, (char) 0x88);
	viola_check_reg_access((char) 0x8, (char) 0x88);
	viola_check_reg_access((char) 0x8, (char) 0x88);
	viola_check_reg_access((char) 0x2, (char) 0x10);
	viola_check_reg_access((char) 0x2, (char) 0x1400);
	viola_check_reg_access((char) 0x8, (char) 0xf8);
	viola_check_reg_access((char) 0x8, (char) 0x0);
	viola_check_reg_access((char) 0x8, (char) 0x88);
	viola_check_reg_access((char) 0x8, (char) 0x88);
	viola_check_reg_access((char) 0x8, (char) 0xf8);
	viola_check_reg_access((char) 0x8, (char) 0x0);
	viola_check_reg_access((char) 0x8, (char) 0x88);
	viola_check_reg_access((char) 0x8, (char) 0x88);
	viola_check_reg_access((char) 0x2, (char) 0x10);
	viola_check_reg_access((char) 0x2, (char) 0x1400);
	viola_check_reg_access((char) 0x8, (char) 0xf8);
	viola_check_reg_access((char) 0x8, (char) 0x0);
	viola_check_reg_access((char) 0x8, (char) 0x88);
	viola_check_reg_access((char) 0x8, (char) 0x88);
	viola_check_reg_access((char) 0x2, (char) 0x4000);
	viola_check_reg_access((char) 0x2, (char) 0x4);
	viola_check_reg_access((char) 0x2, (char) 0x4);
	viola_check_reg_access((char) 0x17,(char)  0x6fff);
	viola_check_reg_access((char) 0xa, (char) 0x0);
	viola_check_reg_access((char) 0xd, (char) 0x9);
	viola_check_reg_access((char) 0xe, (char) 0x9);
	viola_check_reg_access((char) 0xf, (char) 0x3);
	viola_check_reg_access((char) 0x6, (char) 0x4747);
	viola_check_reg_access((char) 0x9, (char) 0x215);
	viola_check_reg_access((char) 0x4, (char) 0x636f);
	viola_check_reg_access((char) 0xa, (char) 0x8000);
	viola_check_reg_access((char) 0x17,(char)  0x6fff);
	viola_check_reg_access((char) 0x16,(char)  0x601f);
	viola_check_reg_access((char) 0xc, (char) 0x30);
	viola_check_reg_access((char) 0x7, (char) 0x2);
	viola_check_reg_access((char) 0x6, (char) 0x4747);
	viola_check_reg_access((char) 0xa, (char) 0x8603);
	viola_check_reg_access((char) 0x2, (char) 0x0);
	viola_check_reg_access((char) 0x8, (char) 0x1);
	viola_check_reg_access((char) 0x8, (char) 0x7);
	viola_check_reg_access((char) 0x2, (char) 0x4000);
	viola_check_reg_access((char) 0x2, (char) 0x4);
	viola_check_reg_access((char) 0x2, (char) 0x4);
	viola_check_reg_access((char) 0x17,(char)  0x6fff);


	viola_check_reg_access((char) 0xc, (char) 0x4b);
	viola_check_reg_access((char) 0x8, (char) 0xa);
	viola_check_reg_access((char) 0x8, (char) 0x1);


	/* omap5432 trace
	//omap_i2c_write_reg: [1]: reg = 0x1, val = 0x601f
	main_core((char) 0x1, (char) 0x601f);

	//omap_i2c_write_reg: [1]: reg = 0xc, val = 0x22
	main_core((char) 0xc, (char) 0x22);

	//omap_i2c_write_reg: [1]: reg = 0x7, val = 0x3
	main_core((char) 0x7, (char) 0x3);

	//omap_i2c_write_reg: [1]: reg = 0x6, val = 0x4747
	main_core((char) 0x6, (char) 0x4747);

	//omap_i2c_write_reg: [1]: reg = 0xa, val = 0x8603
	main_core((char) 0xa, (char) 0x8603);

	//omap_i2c_write_reg: [1]: reg = 0x2, val = 0x1000
	main_core((char) 0x2, (char) 0x1000);

	//omap_i2c_write_reg: [1]: reg = 0x8, val = 0x84
	main_core((char) 0x8, (char) 0x84);

	//omap_i2c_write_reg: [1]: reg = 0x8, val = 0x83
	main_core((char) 0x8, (char) 0x83);

	//omap_i2c_write_reg: [1]: reg = 0x8, val = 0xff
	main_core((char) 0x8, (char) 0xff);

	//omap_i2c_write_reg: [1]: reg = 0x2, val = 0x4000
	main_core((char) 0x2, (char) 0x4000);

	//omap_i2c_write_reg: [1]: reg = 0x2, val = 0x4
	main_core((char) 0x2, (char) 0x4);

	//omap_i2c_write_reg: [1]: reg = 0x2, val = 0x4
	main_core((char) 0x2, (char) 0x4);

	//omap_i2c_write_reg: [1]: reg = 0x1, val = 0x0
	main_core((char) 0x1, (char) 0x0);

	//omap_i2c_write_reg: [1]: reg = 0x2, val = 0x601f
	main_core((char) 0x2, (char) 0x601f);
	*/
}
