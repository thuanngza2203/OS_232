/*
* Copyright (C) 2024 pdnguyen of the HCMC University of Technology
*/
/*
* Source Code License Grant: Authors hereby grants to Licensee 
* a personal to use and modify the Licensed Source Code for 
* the sole purpose of studying during attending the course CO2018.
*/
//#ifdef MM_TLB
/*
* Memory physical based TLB Cache
* TLB cache module tlb/tlbcache.c
*
* TLB cache is physically memory phy
* supports random access 
* and runs at high speed
*/

#include <stdlib.h>
#include <stdio.h>
#include "mm.h"

#define init_tlbcache(mp,sz,...) init_memphy(mp, sz, (1, ##__VA_ARGS__))

int tlb_cache_check(struct pcb_t* proc, int pgn, int *fpn) {
   struct memphy_struct* tlb = proc->tlb;
   if (tlb == NULL) {
      printf("No tlb");
      exit(1);
   }

   int TLBpgsize = tlb->maxsz / (int)PAGE_SIZE; // 64
   int TLBpg = pgn % TLBpgsize;

   if (proc->mm->pgd[pgn] == tlb->pgd[TLBpg].pte)
   {
      if (proc->pid == tlb->pgd[TLBpg].pid)
      {
         if (!PAGING_PAGE_PRESENT(tlb->pgd[TLBpg].pte))
         {
            // page fault
            if (pg_getpage(proc->mm, pgn, fpn, proc) != 0)
            {
               printf("ERROR: Can't get page\n");
               return -3000;
            }
            tlb->pgd[TLBpg].pte = proc->mm->pgd[pgn];
         }

         // hit
         uint32_t pte = tlb->pgd[TLBpg].pte;
         *fpn = PAGING_FPN(pte);
         return 0;
      }
      else
      {
         printf("ERROR: Can't access pid\n");
         return -3000;
      }
   }
   // miss
   else
   {
      tlb->pgd[TLBpg].pte = proc->mm->pgd[pgn];
      tlb->pgd[TLBpg].pid = proc->pid;
      return -1;
   }
   return 0;
}

/*
*  tlb_cache_read read TLB cache device
*  @mp: memphy struct
*  @pid: process id
*  @pgnum: page number
*  @value: obtained value
*/
int tlb_cache_read(struct pcb_t *proc, uint32_t vmaddr, BYTE* value)
{
   /* TODO: the identify info is mapped to 
   *      cache line by employing:
   *      direct mapped, associated mapping etc.
   */
   int off = PAGING_OFFST(vmaddr);
   int pgn = PAGING_PGN(vmaddr);
   int fpn;
   struct memphy_struct *tlb = proc->tlb;
   if (tlb == NULL) {
      return -3000;
   }
   
   int result = tlb_cache_check(proc, pgn, &fpn);
   int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;
   TLBMEMPHY_read(tlb, phyaddr, value);

   if (result == 0) {
      return 0;
   }
   else {
      return -1;
   }
   return -1;
}

/*
*  tlb_cache_write write TLB cache device
*  @mp: memphy struct
*  @pid: process id
*  @pgnum: page number
*  @value: obtained value
*/
int tlb_cache_write(struct pcb_t *proc, uint32_t vmaddr, BYTE* value)
{
   /* TODO: the identify info is mapped to
   *      cache line by employing:
   *      direct mapped, associated mapping etc.
   */
   int off = PAGING_OFFST(vmaddr);
   int pgn = PAGING_PGN(vmaddr);
   int fpn;
   struct memphy_struct *tlb = proc->tlb;

   if (tlb == NULL)
      return -1;

   int result = tlb_cache_check(proc, pgn, &fpn);
   int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;
   TLBMEMPHY_write(tlb, phyaddr, *value);

   if (result == 0)
   {
      // hit
      return 0;
   }
   else
   {
      // miss
      return -1;
   }

   return -1;
}


/*
*  TLBMEMPHY_read natively supports MEMPHY device interfaces
*  @mp: memphy struct
*  @addr: address
*  @value: obtained value
*/
int TLBMEMPHY_read(struct memphy_struct * mp, int addr, BYTE *value)
{
   if (mp == NULL)
   return -1;

   /* TLB cached is random access by native */
   *value = mp->storage[addr];

   return 0;
}


/*
*  TLBMEMPHY_write natively supports MEMPHY device interfaces
*  @mp: memphy struct
*  @addr: address
*  @data: written data
*/
int TLBMEMPHY_write(struct memphy_struct * mp, int addr, BYTE data)
{
   if (mp == NULL)
   return -1;

   /* TLB cached is random access by native */
   mp->storage[addr] = data;

   return 0;
}

/*
*  TLBMEMPHY_format natively supports MEMPHY device interfaces
*  @mp: memphy struct
*/


int TLBMEMPHY_dump(struct memphy_struct * mp)
{
   /*TODO dump memphy contnt mp->storage 
   *     for tracing the memory content
   */

   return 0;
}


/*
*  Init TLBMEMPHY struct
*/
int init_tlbmemphy(struct memphy_struct *mp, int max_size)
{
   mp->storage = (BYTE *)malloc(max_size * sizeof(BYTE));
   mp->maxsz = max_size;

   mp->rdmflg = 1;

   int fgnum = DIV_ROUND_UP(max_size, PAGE_SIZE);
   // MEMPHY_format(mp, PAGE_SIZE); // tạo ra 64 frame trống trong free_list
   mp->pgd = malloc(fgnum * sizeof(struct node_pte));
   for (int i = 0; i < fgnum; i++)
   {

      mp->pgd[i].pid = -1;
      mp->pgd[i].pte = -1;
   }
   return 0;
}

//#endif
