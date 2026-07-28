#include "Headers/DeviceKit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>

static const char *_platform_family = NULL;
static uint32_t _platform_cpu_type = 0;

#define CPU_TYPE_ANY         -1
#define CPU_TYPE_M68K        6
#define CPU_TYPE_HPPA        7
#define CPU_TYPE_SPARC       8
#define CPU_TYPE_MIPS        9
#define CPU_TYPE_I386        10
#define CPU_TYPE_MC88000     11
#define CPU_TYPE_POWERPC     12
#define CPU_TYPE_ARM         14
#define CPU_TYPE_SPARC64     15
#define CPU_TYPE_X86_64      (CPU_TYPE_I386 | (63 << 24))

typedef struct {
    const char *name;
    bool available;
} PlatformFeature;

static PlatformFeature _features[] = {
    { "x11",           false },
    { "opengl",       false },
    { "vulkan",       false },
    { "sse",           false },
    { "sse2",          false },
    { "sse3",          false },
    { "sse4",          false },
    { "avx",           false },
    { "neon",          false },
    { "vmx",           false },
    { "gpu_accel",     false },
    { "kms",           false },
    { NULL,            false }
};

static void detect_platform(void)
{
    struct utsname uts;
    
    if (uname(&uts) < 0) {
        _platform_family = "Unknown";
        return;
    }

    if (strcmp(uts.machine, "x86_64") == 0 || 
        strcmp(uts.machine, "amd64") == 0) {
        _platform_family = "x86_64";
        _platform_cpu_type = CPU_TYPE_X86_64;
    } else if (strcmp(uts.machine, "i386") == 0 ||
               strcmp(uts.machine, "i486") == 0 ||
               strcmp(uts.machine, "i586") == 0 ||
               strcmp(uts.machine, "i686") == 0) {
        _platform_family = "i386";
        _platform_cpu_type = CPU_TYPE_I386;
    } else if (strncmp(uts.machine, "arm", 3) == 0) {
        _platform_family = "arm";
        _platform_cpu_type = CPU_TYPE_ARM;
    } else if (strncmp(uts.machine, "mips", 4) == 0) {
        _platform_family = "mips32";
        _platform_cpu_type = CPU_TYPE_MC88000;
    } else if (strncmp(uts.machine, "sparc", 5) == 0) {
        if (strstr(uts.machine, "64")) {
            _platform_family = "sparc64";
            _platform_cpu_type = CPU_TYPE_SPARC64;
        } else {
            _platform_family = "sparc";
            _platform_cpu_type = CPU_TYPE_SPARC;
        }
    } else if (strncmp(uts.machine, "ppc", 3) == 0 ||
               strncmp(uts.machine, "powerpc", 7) == 0) {
        _platform_family = "powerpc";
        _platform_cpu_type = CPU_TYPE_POWERPC;
    } else if (strncmp(uts.machine, "m68k", 4) == 0 ||
               strcmp(uts.machine, "hppa") == 0) {
        _platform_family = uts.machine;
        _platform_cpu_type = (strncmp(uts.machine, "m68k", 4) == 0) ? 
                             CPU_TYPE_M68K : CPU_TYPE_HPPA;
    } else {
        _platform_family = strdup(uts.machine);
    }
}

static void detect_features(void)
{
    if (getenv("DISPLAY") != NULL) {
        _features[0].available = true; 
    }
    
    if (getenv("MXWL_GPU_ACCEL") != NULL) {
        _features[5].available = true;
    }
    
    if (_platform_cpu_type == CPU_TYPE_X86_64) {
        _features[3].available = true;  /* sse */
        _features[4].available = true;  /* sse2 */
        _features[5].available = true;  /* sse3 */
        _features[6].available = true;  /* sse4 */
    }

    if (_platform_cpu_type == CPU_TYPE_ARM) {

        _features[8].available = true;  /* neon (assume true on modern ARM) */
    }
    
    if (_platform_cpu_type == CPU_TYPE_POWERPC) {
        _features[9].available = true;
    }
}

const char *DKitGetPlatformFamily(void)
{
    if (!_platform_family) {
        detect_platform();
    }
    return _platform_family;
}

uint32_t DKitGetPlatformCPUType(void)
{
    if (!_platform_family) {
        detect_platform();
    }
    return _platform_cpu_type;
}

bool DKitPlatformHasFeature(const char *feature)
{
    if (!feature) return false;
    
    int i;
    for (i = 0; _features[i].name != NULL; i++) {
        if (strcmp(_features[i].name, feature) == 0) {
            return _features[i].available;
        }
    }
    return false;
}

void DKitPrintPlatformInfo(void)
{
    struct utsname uts;
    
    printf("DriverKit Platform Information\n");
    printf("==============================\n");
    
    if (uname(&uts) < 0) {
        printf("System: Unknown\n");
    } else {
        printf("System: %s %s\n", uts.sysname, uts.release);
        printf("Machine: %s (%s)\n", uts.machine, uts.node);
    }
    
    printf("Platform Family: %s\n", DKitGetPlatformFamily());
    printf("CPU Type: 0x%08x\n", DKitGetPlatformCPUType());
    
    printf("\nAvailable Features:\n");
    int i;
    for (i = 0; _features[i].name != NULL; i++) {
        printf("  %s: %s\n", _features[i].name, 
               _features[i].available ? "yes" : "no");
    }
}