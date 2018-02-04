#include "kernel.h"


extern spinlock_t print_lock;

void idleproc( uint32_t r0, uint32_t r1, uint32_t r2 ) {
    // __spin_lock(&print_lock);
    // printf("idleproc() running from 0x%X (%X, %X, %X) debug here...\r\n", idleproc, r0, r1, r2);
    // __spin_unlock(&print_lock);   
    // // bool debug = false;
    // // while(!debug);

    asm("SVC 0x80");

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
    // disp_init();
    // context_init();
    // devices_init();

    // Create idle and root process
    if (create(idleproc, 4096) < 0) {
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
    // context_switch(get_process(0));

    printf("[kernel] Exiting... this should not happen\r\n");
    while(true);
}


