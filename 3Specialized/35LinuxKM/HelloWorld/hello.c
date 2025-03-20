#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

int init_module(void) 
{ 
    int x = 0;
    pr_info("Hello world %d\n", x); 
 
    /* A non 0 return means init_module failed; module can't be loaded. */ 
    return 0; 
} 

void cleanup_module(void) 
{ 
    pr_info("Goodbye world.\n"); 
} 
 
MODULE_LICENSE("GPL");