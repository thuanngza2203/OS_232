#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
    if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
    /* TODO: put a new process to queue [q] */
    if (!q) {
        perror("Queue is null");
        exit(1);
    }
    if (q->size == MAX_QUEUE_SIZE) {
        perror("Queue is full");
        exit(1);
    }

    q->proc[q->size] = proc;
    q->size++;

    // printf("queue.c (%d): Added process %d\n", q->size, proc->pid);
}

struct pcb_t * dequeue(struct queue_t * q) {
    /* TODO: return a pcb whose prioprity is the highest
    * in the queue [q] and remember to remove it from q
    * */
    if (!q) {
        return NULL;
    }
    if (q->size == 0) {
        return NULL;
    }
    
    struct pcb_t* item = q->proc[0];
    int length = q->size;

    // get item with lowest prio
    int index = 0;
    for (int i = 0; i < length; ++i) {
        if (q->proc[i]->priority < item->priority) {
            item = q->proc[i];
            index = i;
        }
    }

    // swap last with lowest prio and remove last
    q->proc[index] = q->proc[length - 1];
    q->proc[length - 1] = NULL;
    q->size--;

    // printf("queue.c: Dequeue process %d\n", item->pid);

    return item;
}

