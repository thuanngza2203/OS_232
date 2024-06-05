/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef CPU_TLB
/*
 * CPU TLB
 * TLB module cpu/cpu-tlb.c
 */
 
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

static pthread_mutex_t cache_lock = PTHREAD_MUTEX_INITIALIZER;

int tlb_change_all_page_tables_of(struct pcb_t *proc,  struct memphy_struct * mp)
{
  /* TODO update all page table directory info 
   *      in flush or wipe TLB (if needed)
   */

  return 0;
}

int tlb_flush_tlb_of(struct pcb_t *proc, struct memphy_struct * mp)
{
  /* TODO flush tlb cached*/
  if (mp == NULL) {
    return -1;
  }
  if (mp->pgd == NULL) {
    return -1;
  }

  pthread_mutex_lock(&cache_lock);
  free(mp->pgd);
  pthread_mutex_unlock(&cache_lock);

  return 0;
}

/*tlballoc - CPU TLB-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr, val;

  // bug here, don't know how to fix so I just do this
  struct memphy_struct* temp_tlb = proc->tlb;
  if (proc->tlb->maxsz == 0) {
    init_tlbmemphy(temp_tlb, 65536);
  }

  /* By default using vmaid = 0 */
  val = __alloc(proc, 0, reg_index, size, &addr);

  /* TODO update TLB CACHED frame num of the new allocated page(s)*/
  int fpn = 0;
  int pgn_start = PAGING_PGN(addr);
  int pgn_end = PAGING_PGN((addr + size));

  for (int pgn = pgn_start; pgn <= pgn_end; pgn++)
  {
    tlb_cache_check(proc, pgn, &fpn);
  }
  // printf("alloc region: %d \n", reg_index);

  return val;
}

/*pgfree - CPU TLB-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlbfree_data(struct pcb_t *proc, uint32_t reg_index)
{
  /* TODO update TLB CACHED frame num of freed page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  pthread_mutex_lock(&cache_lock);

  unsigned long rg_start = proc->mm->symrgtbl[reg_index].rg_start;
  unsigned long rg_end = proc->mm->symrgtbl[reg_index].rg_end;
  int pgn_start = PAGING_PGN(rg_start);
  int pgn_end = PAGING_PGN((rg_end));
  struct memphy_struct *tlb = proc->tlb;

  for (int pgn = pgn_start; pgn <= pgn_end; pgn++)
  {
    tlb->pgd[pgn].pte = -1;
    tlb->pgd[pgn].pid = -1;
  }

  __free(proc, 0, reg_index);

  pthread_mutex_unlock(&cache_lock);
  return 0;
}


/*tlbread - CPU TLB-based read a region memory
 *@proc: Process executing the instruction
 *@source: index of source register
 *@offset: source address = [source] + [offset]
 *@destination: destination storage
 */
int tlbread(struct pcb_t * proc, uint32_t source,
            uint32_t offset, 	uint32_t destination) 
{
  BYTE data, frmnum = -1;
	
  /* TODO retrieve TLB CACHED frame num of accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  /* frmnum is return value of tlb_cache_read/write value*/

  unsigned long rg_start = proc->mm->symrgtbl[source].rg_start;
  unsigned long rg_end = proc->mm->symrgtbl[source].rg_end;

  uint32_t vmaddr = rg_start + offset;
  if (vmaddr < rg_start || vmaddr > rg_end) {
    printf("[rg_start; rg_end] = [%lu; %lu], vmaddr = %u\n", rg_start, rg_end, vmaddr);
    printf("Vmaddr out of range or Rg hasn't been allocated yet to read\n");
    return -1;
  }
	
#ifdef IODUMP
  frmnum = tlb_cache_read(proc, vmaddr, &data);
  if (frmnum >= 0)
    printf("TLB hit at read reg = %d + offset %d\n", 
	         source, offset);
  else 
    printf("TLB miss at read reg = %d + offset %d\n", 
	         source, offset);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  int val = __read(proc, 0, source, offset, &data);

  destination = (uint32_t) data;

  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  return val;
}

/*tlbwrite - CPU TLB-based write a region memory
 *@proc: Process executing the instruction
 *@data: data to be wrttien into memory
 *@destination: index of destination register
 *@offset: destination address = [destination] + [offset]
 */
int tlbwrite(struct pcb_t * proc, BYTE data,
             uint32_t destination, uint32_t offset)
{
  int val;
  BYTE frmnum = -1;

  /* TODO retrieve TLB CACHED frame num of accessing page(s))*/
  /* by using tlb_cache_read()/tlb_cache_write()
  frmnum is return value of tlb_cache_read/write value*/

  unsigned long rg_start = proc->mm->symrgtbl[destination].rg_start;
  unsigned long rg_end = proc->mm->symrgtbl[destination].rg_end;
  uint32_t vmaddr = rg_start + offset;

  printf("%lu %u %lu\n", rg_start, vmaddr, rg_end);

  if (vmaddr < rg_start || vmaddr > rg_end) {
    printf("[rg_start; rg_end] = [%lu; %lu], vmaddr = %u\n", rg_start, rg_end, vmaddr);
    printf("Vmaddr out of range or Rg hasn't been allocated yet to write\n");
    return -1;
  }

#ifdef IODUMP
  frmnum = tlb_cache_write(proc, vmaddr, &data);
  if (frmnum >= 0)
    printf("TLB hit at write region=%d offset=%d value=%d\n",
	          destination, offset, data);
	else
    printf("TLB miss at write region=%d offset=%d value=%d\n",
            destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  val = __write(proc, 0, destination, offset, data);

  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  return val;
}

//#endif
