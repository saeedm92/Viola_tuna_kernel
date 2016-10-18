/*
* Viola kernel fault handler
* File: viola_fault.c
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

#include <linux/module.h>
#include <net/sock.h>
#include <linux/proc_fs.h>
#include <asm/cacheflush.h> /* clflush_cache_range() */
#include <asm/tlbflush.h> /* flush_tlb_all() */
#include <asm/highmem.h>
#include <linux/vmalloc.h>
#include <linux/viola.h>

#ifdef CONFIG_ARM
PTE_BIT_FUNC(exprotect, |= L_PTE_XN);
PTE_BIT_FUNC(mkpresent, |= L_PTE_PRESENT);
PTE_BIT_FUNC(mknotpresent, &= ~L_PTE_PRESENT); // use for invalidation (to signal NO permissions)
#define dfv_pte_present pte_present

#define LPRINTK0(fmt, args...)

pte_t *viola_get_pte(struct mm_struct *mm, unsigned long addr)
{
	pgd_t *pgd;

	if (!mm)
		mm = &init_mm;

	pgd = pgd_offset(mm, addr);

	do {
		pud_t *pud;
		pmd_t *pmd;
		pte_t *pte;

		if (pgd_none(*pgd))
			break;

		pud = pud_offset(pgd, addr);
	
		if (pud_none(*pud))
			break;

		pmd = pmd_offset(pud, addr);

		if (pmd_none(*pmd))
			break;

		/* We must not map this if we have highmem enabled */
		if (PageHighMem(pfn_to_page(pmd_val(*pmd) >> PAGE_SHIFT)))
			break;

		pte = pte_offset_map(pmd, addr);
		return pte;
	} while(0);

	return NULL;
}
#endif /* CONFIG_ARM */

int viola_change_page_state(unsigned long local_addr, int state)
{
	pte_t *ptep = viola_get_pte(NULL, local_addr);
	
	if (!ptep)
		goto error_no_pte;
			
	switch (state) {
		
	case VIOLA_SHARED:
		/* grant read-only permissions to the PTE, aka SHARED state */
		set_pte_ext(ptep, pte_mkpresent(*ptep), 0);
		set_pte_ext(ptep, pte_wrprotect(*ptep), 0);
		break;
		
	case VIOLA_MODIFIED:
		set_pte_ext(ptep, pte_mkpresent(*ptep), 0);
		set_pte_ext(ptep, pte_mkwrite(*ptep), 0);
		break;		
		
	case VIOLA_INVALID:
	      set_pte_ext(ptep, pte_mknotpresent(*ptep), 0);
	      set_pte_ext(ptep, pte_wrprotect(*ptep), 0);
		break;
		
	default:
		break;
	}

	flush_tlb_all();
		
	pte_unmap(ptep);
	return 0;
	
error_no_pte:
	return -EFAULT;
}

int counter = 0;

int viola_kernel_fault(struct mm_struct *mm, unsigned long addr, unsigned int fsr,
	    	       struct pt_regs *regs)
{
	if (!(((addr >= 0xfa070000) & (addr < 0xfa071000)) ||
	      ((addr >= 0xfa350000) & (addr < 0xfa351000))))
		return -ENOMEM;

	char pc_1_val = *((char *) (regs->ARM_pc + 1));
	int src_reg_index = pc_1_val & 0xf0; 
	src_reg_index = src_reg_index >> 4;
	char reg = addr & 0xff;
	char char_val = (char) regs->uregs[src_reg_index];
	if (viola_check_reg_access(reg, char_val)) {
		return -ENOMEM;
	}
	unsigned long dst_addr = addr + 0x500000;
	__raw_writew(regs->uregs[src_reg_index], dst_addr);

	regs->ARM_pc += 4;
	return 0;
}
