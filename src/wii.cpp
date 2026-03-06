#include "platform.h"
#include <gccore.h>
#include <errno.h>
#include <reent.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

extern "C" {
void* __ppc_main_sp = (void*)0x817FFFE0;
void* __ppc_excpt_sp = (void*)0x817FBFE0;

extern char __Arena1Lo[];
extern char __Arena1Hi[];
extern char __Arena2Lo[];
extern char __Arena2Hi[];
extern void* __malloc_sbrk_base;

void __real_KThreadInit(void);
void __real_KIrqInit(void);

static void wii_init_arenas_early(void)
{
    static int arenas_initialized = 0;
    if (arenas_initialized) {
        return;
    }
    arenas_initialized = 1;

    void* arena1_lo = (void*)(((unsigned int)__Arena1Lo + 31u) & ~31u);
    SYS_SetArena1Lo(arena1_lo);
    SYS_SetArena2Lo((void*)__Arena2Lo);
    __malloc_sbrk_base = arena1_lo;
}

void* __wrap__sbrk_r(struct _reent* r, ptrdiff_t incr)
{
    static char* heap_base = NULL;
    static char* heap_curr = NULL;
    static char* heap_limit = NULL;

    wii_init_arenas_early();

    if (heap_curr == NULL) {
        uintptr_t lo = (uintptr_t)SYS_GetArena1Lo();
        uintptr_t hi = (uintptr_t)SYS_GetArena1Hi();

        lo = (lo + 31u) & ~31u;
        hi = hi & ~31u;

        if (lo < 0x80000000u || hi <= lo) {
            lo = ((uintptr_t)__Arena1Lo + 31u) & ~31u;
            hi = ((uintptr_t)__Arena1Hi) & ~31u;
        }

        heap_base = (char*)lo;
        heap_curr = (char*)lo;
        heap_limit = (char*)hi;
    }

    if (incr == 0) {
        return heap_curr;
    }

    if (incr > 0) {
        if ((uintptr_t)incr > (uintptr_t)(heap_limit - heap_curr)) {
            if (r != NULL) {
                r->_errno = ENOMEM;
            }
            errno = ENOMEM;
            return (void*)-1;
        }
    } else {
        if ((uintptr_t)(-incr) > (uintptr_t)(heap_curr - heap_base)) {
            if (r != NULL) {
                r->_errno = ENOMEM;
            }
            errno = ENOMEM;
            return (void*)-1;
        }
    }

    char* prev = heap_curr;
    heap_curr += incr;
    return prev;
}

void __wrap_KThreadInit(void)
{
    __real_KThreadInit();
}

void __wrap_KIrqInit(void)
{
    __real_KIrqInit();
}

void __wrap_SYS_Init(void)
{
    wii_init_arenas_early();
}

void __wrap_SYS_PreMain(void)
{
}
}

static void wii_bootstrap_banner(void)
{
    VIDEO_Init();
    GXRModeObj* mode = VIDEO_GetPreferredMode(NULL);
    void* framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(mode));

    size_t fb_size = (size_t)mode->fbWidth * (size_t)mode->xfbHeight * VI_DISPLAY_PIX_SZ;
    memset(framebuffer, 0x00, fb_size);

    VIDEO_Configure(mode);
    VIDEO_SetNextFramebuffer(framebuffer);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (mode->viTVMode & VI_NON_INTERLACE) {
        VIDEO_WaitVSync();
    }

    for (int i = 0; i < 180; i++) {
        VIDEO_WaitVSync();
    }
}

extern "C" int wii_kfx_entry(int argc, char *argv[])
{
    SYS_Report("WII_WRAP: wii_kfx_entry begin\n");
    (void)argc;
    (void)argv;
    static char app_name[] = "keeperfx";
    char* safe_argv[] = { app_name, NULL };
    int safe_argc = 1;

    wii_bootstrap_banner();
    int result = kfxmain(safe_argc, safe_argv);
    return result;
}
