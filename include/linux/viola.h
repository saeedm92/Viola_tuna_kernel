#ifndef _VIOLA_H_
#define _VIOLA_H_

#define VIOLA_SHARED	0
#define VIOLA_MODIFIED	1
#define VIOLA_INVALID	2

int viola_change_page_state(unsigned long local_addr, int state);
int viola_kernel_fault(struct mm_struct *mm, unsigned long addr, unsigned int fsr,
	    	       struct pt_regs *regs);
int viola_check_reg_access(char regoff, char regval);

#endif /* _VIOAL_H_ */
