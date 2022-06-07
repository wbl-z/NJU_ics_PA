#include "common.h"
#include "memory.h"
#include "string.h"

#include <elf.h>

#ifdef HAS_DEVICE_IDE
#define ELF_OFFSET_IN_DISK 0
#endif

#define STACK_SIZE (1 << 20)

void ide_read(uint8_t *, uint32_t, uint32_t);
void create_video_mapping();
uint32_t get_ucr3();

uint32_t loader()
{
	Elf32_Ehdr *elf;
	Elf32_Phdr *ph, *eph;

#ifdef HAS_DEVICE_IDE
	uint8_t buf[4096];
	ide_read(buf, ELF_OFFSET_IN_DISK, 4096);
	elf = (void *)buf;
	Log("ELF loading from hard disk.");
#else
	elf = (void *)0x0;
	Log("ELF loading from ram disk.");
#endif

	/* Load each program segment */
	
	ph = (void *)elf + elf->e_phoff;
	eph = ph + elf->e_phnum;
	for (; ph < eph; ph++)
	{
		if (ph->p_type == PT_LOAD)
		{
		    /* 3-3TODO: 
		    1. 按照`program header`中的`p_memsz`属性, 为这一段segment分配一段不小于`p_memsz`的物理内存；
            2. 根据虚拟地址`p_vaddr`和分配到的物理地址正确填写用户进程的页目录和页表；
            3. 把ELF文件中这一段segment的内容加载到这段物理内存；*/

		    uint32_t paddr = mm_malloc(ph->p_vaddr, ph->p_memsz);// uint32_t mm_malloc(uint32_t va, int len)返回物理内存区间的首地址
		    Log("%x,%x",ph->p_vaddr,paddr);
#ifdef HAS_DEVICE_IDE
            // 4-2TODO:use ide
            ide_read((uint8_t *)paddr, ph->p_offset, ph->p_filesz);
#else
		    memcpy((void *)paddr, (void *)elf + ph->p_offset, ph->p_filesz);
		    memset((void *)(paddr + ph->p_filesz), 0x0, ph->p_memsz - ph->p_filesz);
#endif
/* 2-2TODO: copy the segment from the ELF file to its proper memory area */

/* 2-2TODO: zeror the memory area [vaddr + file_sz, vaddr + mem_sz) */

#ifdef IA32_PAGE
			/* Record the program break for future use */
			extern uint32_t brk;
			uint32_t new_brk = ph->p_vaddr + ph->p_memsz - 1;
			if (brk < new_brk)
			{
				brk = new_brk;
			}
#endif
		}
	}

	volatile uint32_t entry = elf->e_entry;

#ifdef IA32_PAGE
	mm_malloc(KOFFSET - STACK_SIZE, STACK_SIZE);
#ifdef HAS_DEVICE_VGA
	create_video_mapping();
#endif
	write_cr3(get_ucr3());
#endif
	return entry;
}
