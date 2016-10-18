/*
* Viola kernel interface
* File: viola_interface.c
*
* Copyright (c) 2016 University of California, Irvine, CA, USA
* All rights reserved.
*
* Authors: Saeed Mirzamohammadi <saeed@uci.edu>
*          Ardalan Amiri Sani <arrdalan@gmail.com>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License version 2 as published by
* the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>

#define IN_KERNEL	1

int reginfer_i2c_main(int iregaccess_p, int oregaccess_p);
int reginfer_gpio_main(int iregaccess_p, int oregaccess_p);
int viola_main(int regaccess_ptr, int master_iomem_ptr, int slave_iomem_ptr);

#ifdef IN_KERNEL
bool iomems_initialized = false;
#else
int iomems_initialized = 0;
#define PRINTK_ERR(f, args...) printf("%s " f, __func__, ##args); 
#endif

char *twl6040_iomem;
char *gn_led_iomem;

static int init_iomem(unsigned int devid)
{
	int i;
	if (devid == 0x4b) { /* twl6040 */
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
	ret = reginfer_gpio_main((int) iregaccess, (int) oregaccess);
	
	return ret;
}

int pass_to_omap_i2c(char *iregaccess, char *oregaccess)
{
	int ret;
	ret = reginfer_i2c_main((int) iregaccess, (int) oregaccess);

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


	if (!iomems_initialized) {
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

	ret = viola_main((int) twl6040_iomem, (int) gn_led_iomem, (int) regaccess);
	
	_devid = (char) oregaccess[2]; //devid
	_regoff = (char) oregaccess2[0]; //reg_off
	_regval = (char) oregaccess[1]; //reg_val
	
	if (!ret) {
		if (_devid == 0x30) {
			gn_led_iomem[_regoff] = (char) _regval;
		}
		if (_devid == 0x4b) { /* twl6040 */
			twl6040_iomem[_regoff] = (char) _regval;
		}
	}

	return ret;

}
EXPORT_SYMBOL(viola_check_reg_access);
