// GRR20245621 Daniel Wesley Freitas Siqueira
// GRR20245396 Guilherme Vitoriano Santana de Oliveira
// GRR20245567 Ulisses Bastian Machado da Rosa

// PingPongOS - PingPong Operating System


#include <assert.h>

#include "lib/libc.h"
#include "ppos.h"


static struct task_t *p1, *p2, *p3, *c1, *c2;
static struct semaphore_t *s_item, *s_vaga, *s_buffer;
static int item_buffer[5];
static unsigned int buf_start, buf_end;


void producer(void* arg) {
    int item;
    while (true) {
        task_sleep(1000);
        item = randnum() % 99;
        if (sem_down(s_vaga) == ERROR)
            break;
        if (sem_down(s_buffer) == ERROR)
            break;
        item_buffer[buf_end%5] = item;
        buf_end++;
        sem_up(s_buffer);
        sem_up(s_item);
        printk("%s inseriu %d (tem %d)\n", (char *)arg, item, buf_end - buf_start);
    }

    task_exit(0);
}


void consumer(void* arg) {
    int i = 0;
    int item;
    while (i < 10) {
        sem_down(s_item);
        sem_down(s_buffer);
        item = item_buffer[buf_start%5];
        buf_start++;
        sem_up(s_buffer);
        sem_up(s_vaga);

        printk("%s consumiu %d (tem %d)\n", (char *)arg, item, buf_end - buf_start);
        i++;
        task_sleep(1000);
    }

    task_exit(0);
}


void user_main() {
    printf("user: inicio\n");
    // Setting rand seed
    randseed(0);
    int status;

    // Initializing semaphores
    s_buffer = sem_create(1);
    assert(s_buffer);
    s_vaga = sem_create(5);
    assert(s_vaga);
    s_item = sem_create(0);
    assert(s_item);

    // Initializing tasks
    p1 = task_create("p1", producer, "P1");
    assert(p1);
    p2 = task_create("p2", producer, "\tP2");
    assert(p2);
    p3 = task_create("p3", producer, "\t\tP3");
    assert(p3);
    c1 = task_create("c1", consumer, "\t\t\tC1");
    assert(c1);
    c2 = task_create("c2", consumer, "\t\t\t\tC2");
    assert(c2);


    status = task_wait(c1);
    assert(status == NOERROR);
    status = task_wait(c2);
    assert(status == NOERROR);


    printf("user: destruindo semáforos\n");
    status = sem_destroy(s_buffer);
    assert(status == NOERROR);
    status = sem_destroy(s_item);
    assert(status == NOERROR);   
    status = sem_destroy(s_vaga);
    assert(status == NOERROR);


    // Wait for other tasks to end
    status = task_wait(p1);
    assert(status == NOERROR);
    status = task_wait(p2);
    assert(status == NOERROR);
    status = task_wait(p3);
    assert(status == NOERROR);

    // Destroying other tasks
    status = task_destroy(p1);
    assert(status == NOERROR);
    status = task_destroy(p2);
    assert(status == NOERROR);
    status = task_destroy(p3);
    assert(status == NOERROR);
    status = task_destroy(c1);
    assert(status == NOERROR);
    status = task_destroy(c2);
    assert(status == NOERROR);

    printf("user: fim\n");

    task_exit(0);
}
