#include "pc.h"

#include <driver/vga.h>
#include <exc.h>
#include <intr.h>
#include <xsu/slab.h>
#include <xsu/syscall.h>
#include <xsu/utils.h>

static inline int __pc_gettimeslots(int level);

void copy_context(context* src, context* dest)
{
    dest->epc = src->epc;
    dest->at = src->at;
    dest->v0 = src->v0;
    dest->v1 = src->v1;
    dest->a0 = src->a0;
    dest->a1 = src->a1;
    dest->a2 = src->a2;
    dest->a3 = src->a3;
    dest->t0 = src->t0;
    dest->t1 = src->t1;
    dest->t2 = src->t2;
    dest->t3 = src->t3;
    dest->t4 = src->t4;
    dest->t5 = src->t5;
    dest->t6 = src->t6;
    dest->t7 = src->t7;
    dest->s0 = src->s0;
    dest->s1 = src->s1;
    dest->s2 = src->s2;
    dest->s3 = src->s3;
    dest->s4 = src->s4;
    dest->s5 = src->s5;
    dest->s6 = src->s6;
    dest->s7 = src->s7;
    dest->t8 = src->t8;
    dest->t9 = src->t9;
    dest->hi = src->hi;
    dest->lo = src->lo;
    dest->gp = src->gp;
    dest->sp = src->sp;
    dest->fp = src->fp;
    dest->ra = src->ra;
}
static task_struct* __getnexttask()
{
    flushsleeplist(); //update sleep list
    task_struct* next = (task_struct*)0;
    int i;
    for (i = PROC_LEVELS - 1; i >= 0; i--) {
        if (!list_empty(&ready_list[i])) {
            next = list_entry(ready_list[i].next, task_struct, ready);
            break;
        }
    }
    if (!next) {
        next = create_kthread("time", PROC_LEVELS-1 ,0);
        // while (1)
        //     ; //panic
    }
}

void flushsleeplist()
{
    task_struct* task;
    struct list_head *pos, *n;
    list_for_each_safe(pos, n, &sleep_list)
    {
        task = list_entry(pos, task_struct, sleep);
        time_u64 now;
        pc_time_get(&now);
        if (pc_time_cmp(&now,&task->sleeptime) >= 0) {
            list_del_init(&task->sleep);
            task->state = PROC_STATE_READY;
            task->level = PROC_LEVELS - 1;
            list_add(&task->ready, &ready_list[task->level]);
        }
    }
}

void pc_schedule(unsigned int status, unsigned int cause, context* pt_context)
{
    //printalltask();
    //printreadylist();
    current->counter--;
    // Check whether the process' timeslots are used out
    if (current->counter > 0) {
        __reset_counter();
    } else // Restore counter and choose the next process
    {
        if(current->level)
            current->level--;

        current->counter = __pc_gettimeslots(current->level);
        current->state = PROC_STATE_READY;
        list_add_tail(&current->ready, &ready_list[current->level]);

        __pc_schedule(status, cause, pt_context);
    }
}
void __pc_schedule(unsigned int status, unsigned int cause, context* pt_context) //find next task and load context
{
    if(current)
    {
        copy_context(pt_context, &current->context);
    }
    // Get next ready task
    current = __getnexttask();
    list_del_init(&current->ready);
    if (current) 
    {
        current->state = PROC_STATE_RUNNING;
    }

    if ((unsigned int)current <= 0x80000000) {
        kernel_printf("Fetched invalid task from ready list:(unsigned)current = %x\n");
    }

    // Load context
    copy_context(&current->context, pt_context);

    __reset_counter();
}
void __reset_counter()
{
    setasid(current->ASID);
    kernel_sp = current->kernel_stack;

    // Reset counter
    asm volatile("mtc0 $zero, $9\n\t");
}

static inline int __pc_gettimeslots(int level)
{
    level = PROC_LEVELS - level;
    return PROC_DEFAULT_TIMESLOTS << level;
}

void pc_time_get(time_u64* dst)
{
    asm volatile(
        "mfc0 %0, $9, 6\n\t"
        "mfc0 %1, $9, 7\n\t"
        : "=r"(dst->lo), "=r"(dst->hi));
}

int pc_time_cmp(time_u64 *large, time_u64 *small)
{
    if(large->hi > small->hi)
    {
        return 1;
    }
    else if (large->hi == small->hi)
    {
        if(large->lo > small->lo)
        {
            return 1;
        }
        else if(large->lo == small->lo)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}

void pc_time_add(time_u64 *dst,unsigned int src)
{
    if(src + dst->lo <= dst->lo)
        dst->hi++;
    dst->lo += src;
}