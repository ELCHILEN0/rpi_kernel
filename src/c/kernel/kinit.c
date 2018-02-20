#include "kernel.h"

extern spinlock_t print_lock;

void newproc() {
    __spin_lock(&print_lock);
    printf("[newproc] started...\r\n");
    __spin_unlock(&print_lock); 

    uint32_t pid = sysgetpid();;

    __spin_lock(&print_lock);
    printf("[newproc] pid = %d\r\n", pid);
    __spin_unlock(&print_lock);

    while(true);
}

void idleproc( uint32_t r0, uint32_t r1, uint32_t r2 ) {
    __spin_lock(&print_lock);
    printf("[idleproc] started...\r\n");
    __spin_unlock(&print_lock);

    uint32_t pid = sysgetpid();;

    __spin_lock(&print_lock);
    printf("[idleproc] pid = %d\r\n", pid);
    __spin_unlock(&print_lock); 

    pid = syscreate(newproc, 1024);    

    __spin_lock(&print_lock);
    printf("[idleproc] created %d\r\n", pid);
    __spin_unlock(&print_lock); 

    sysyield();    

    while(true) {
        asm("wfi"); 
    }
}

void kernel_init( void )
{
    __spin_lock(&print_lock);
    printf("kernel_init()\r\n");
    __spin_unlock(&print_lock); 
    
    process_init();
    dispatcher_init();

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
  
    switch_to(next());
    asm("MSR SPSel, #0");
    __load_context();

    printf("[kernel] Exiting... this should not happen\r\n");
    while(true);
}


