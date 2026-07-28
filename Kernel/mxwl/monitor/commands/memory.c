#include "../monitor.h"
#include "memory.h"
#include "../../kal/kal.h"

static kern_return_t _memory_handler(int argc, char **argv,
                                      char *out, size_t out_size)
{
    (void)argc; (void)argv;

    size_t page_size = kal_get_page_size();
    uint64_t total = kal_get_total_memory();
    uint64_t avail = kal_get_available_memory();

    size_t pos = 0;
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "Memory Information:\n");
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "  Page Size:    %zu bytes\n", page_size);
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "  Total Memory: %llu KB\n",
                            (unsigned long long)(total / 1024));
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "  Available:    %llu KB\n",
                            (unsigned long long)(avail / 1024));

    return KERN_SUCCESS;
}

mx_cmd_t _memory_cmd = {
    .name       = "memory",
    .help_short = "Display memory usage information",
    .help_long  = "Usage: memory\n"
                  "Shows page size, total physical memory, and available memory.",
    .handler    = _memory_handler,
};
