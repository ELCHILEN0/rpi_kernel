#include "kernel.h"


extern spinlock_t print_lock;

void idleproc( void ) {
    __spin_lock(&print_lock);
    printf("idleproc() running from 0x%X\r\n", idleproc);
    __spin_unlock(&print_lock);   

    for ( ;; ) {
        asm("wfi"); 
    }
}

void kernel_init( void )
{
    // Initialize process, dispatcher, context and device structures.
    process_init();
    // disp_init();
    // context_init();
    // devices_init();

    // Create idle and root process
    if (create( &idleproc, 4096) < 0) {
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
    // dispatch(); 

    printf("[kernel] Exiting... this should not happen\r\n");
    while(true);
}


