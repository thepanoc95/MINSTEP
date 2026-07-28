/*
    NOTE: This source code uses the Macintosh Toolbox to work!
    
    filename: kal_mac68k_memory.c
    author: thepanoc95
    
    */
    
#include "../kal_memory.h"
    
#include <Memory.h>
#include <string.h>
    
void *kal_malloc(size_t size)
{
    return NewPtr((Size)size);
}
    
void *kal_calloc(size_t count, size_t size)
{
    Size total = (Size)(count * size);
    Ptr p = NewPtr(total);
        
    if (p != NULL)
        memset(p, 0, total);  
        return p;    
}
    
void kal_free(void *ptr)
{
    if (ptr != NULL)
        DisposePtr((Ptr)ptr);
}
    
void *kal_realloc(void *ptr, size_t new_size)
{
    Ptr new_ptr;
        
    if (ptr == NULL)
        return NewPtr((Size)new_size);
            
    new_ptr = NewPtr((Size)new_size);
    if (new_ptr == NULL)
        return NULL;
            
    DisposePtr((Ptr)ptr);
    return new_ptr;    
}