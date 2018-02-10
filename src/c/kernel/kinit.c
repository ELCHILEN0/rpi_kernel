#include "kernel.h"


extern spinlock_t print_lock;

void newproc() {
    __spin_lock(&print_lock);
    printf("newproc()\r\n");
    __spin_unlock(&print_lock); 
}

void idleproc( uint32_t r0, uint32_t r1, uint32_t r2 ) {
    // __spin_lock(&print_lock);
    // printf("idleproc(%d, %d, %d)\r\n", r0, r1, r2);
    // __spin_unlock(&print_lock); 

    syscreate(newproc, 1024);

    while(true) {
        asm("wfi"); 
    }
}

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


