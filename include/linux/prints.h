#define PRINTK0(fmt, args...) printk("%s: " fmt, __func__, ##args)
#define PRINTKM0(fmt, args...) printk("%s: " fmt, __func__, ##args)
#define PRINTK_ERR(fmt, args...) printk("%s: " fmt, __func__, ##args)
