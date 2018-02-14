#include "kernel.h"

extern spinlock_t print_lock;

void newproc() {
    __spin_lock(&print_lock);
    printf("newproc()\r\n");
    __spin_unlock(&print_lock); 
}

void idleproc( uint32_t r0, uint32_t r1, uint32_t r2 ) {
    __spin_lock(&print_lock);
    // write(0, "idleproc()\r\n", 12);
    printf("this is a test\r\n");
    __spin_unlock(&print_lock);
    /*
       84bf8:	f0000100 	adrp	x0, a7000 <__extenddftf2+0x28>
    ESR_EL1        0x96000044	2516582468 == 000100 Translation fault, level 0
    
    level 0 not configured... level 1 first configured level
        
    */

    uint32_t pid = 10;
    pid = sysgetpid();
    pid = sysgetpid();
    pid = sysgetpid();

    // syscreate(newproc, 1024);    

    __spin_lock(&print_lock);
    printf("[%d] idleproc(%d, %d, %d)\r\n", pid, r0, r1, r2);
    __spin_unlock(&print_lock); 

    pid = sysgetpid();

    __spin_lock(&print_lock);
    printf("[%d] idleproc(%d, %d, %d)\r\n", pid, r0, r1, r2);
    __spin_unlock(&print_lock); 


    while(true) {
        asm("wfi"); 
    }
}

struct list_head my_list;

void kernel_init( void )
{
    __spin_lock(&print_lock);
    printf("kernel_init()\r\n");
    __spin_unlock(&print_lock); 
    
    // Initialize process, dispatcher, context and device structures.
    process_init();
    dispatcher_init();
    // context_init();
    // devices_init();
    // Create idle and root process

    if (create(idleproc, 4096, PRIORITY_IDLE) < 0) {
        __spin_lock(&print_lock);
        printf("failed to init idle() process\r\n");
        __spin_unlock(&print_lock);            
        return;
    }

    // if (create( &root, 1024, MED) < 0) {
    //     kprintf("failed to init root() process");
    //     return;
    // }
  
    // Dispatch processes and wait for system calls
    dispatch(); 

    printf("[kernel] Exiting... this should not happen\r\n");
    while(true);
}


