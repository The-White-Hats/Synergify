#include "../ds/fib_heap.h"
#include <stdbool.h>
#define ASSERT(CONDITION) assert(CONDITION, #CONDITION, __FILE__, __LINE__)

static bool assert(bool cond, char* err_msg, char* file_name, int line)
{
    if (!cond)
        fprintf(stderr, 
                "'%s' is not true in file '%s' at line %d.\n", 
                err_msg,
                file_name,
                line);
    
    return cond;
}

static void test_heap_correctness() 
{
    fib_heap_t* heap;
    size_t i;
    
    heap = fib_heap_alloc();
    ASSERT(fib_heap_is_healthy(heap));
    
    for (i = 0; i < 30; ++i) 
    {
        ASSERT(fib_heap_insert(heap, i, 30 - i));
    }
    
    ASSERT(fib_heap_size(heap) == 30);
    
    ASSERT(fib_heap_is_healthy(heap));

    for (i = 29; i != (size_t) -1; --i) 
    {
        ASSERT((size_t) fib_heap_extract_min(heap) == i);
    }

    ASSERT(fib_heap_size(heap) == 0);
    ASSERT(fib_heap_is_healthy(heap));
    
    for (i = 10; i < 100; ++i) 
    {
        ASSERT(fib_heap_insert(heap, i, i));
    }
    
    for (i = 10; i < 50; ++i) 
    {
        ASSERT((size_t) fib_heap_min(heap) == i);
        ASSERT((size_t) fib_heap_extract_min(heap) == i);
    }

    ASSERT(fib_heap_is_healthy(heap));
    
    for (i = 50; i < 100; ++i) 
    {
        ASSERT((size_t) fib_heap_min(heap) == i);
        ASSERT((size_t) fib_heap_extract_min(heap) == i);
    }
    ASSERT(fib_heap_size(heap) == 0);
    
    ASSERT(fib_heap_min(heap) == NULL);
    ASSERT(fib_heap_extract_min(heap) == NULL);
    
    for (i = 20; i < 40; ++i) 
    {
        ASSERT(fib_heap_insert(heap, i, i));
    }
    
    ASSERT(fib_heap_size(heap) == 20);
    
    ASSERT(fib_heap_is_healthy(heap));
    
    fib_heap_free(heap);
}

int main() {
  test_heap_correctness();
  printf("SUCCESS");
  return 0;
}