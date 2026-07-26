#include "platform.h"
#include <gccore.h>
#include <fat.h>
#include <errno.h>
#include <reent.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

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
    void __real_SYS_Init(void);
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

static void wii_debug_list_dir(const char *path)
{
    DIR *dir = opendir(path);
    if (dir == NULL) {
        SYS_Report("WII_FS: opendir('%s') failed errno=%d\n", path, errno);
        return;
    }
    SYS_Report("WII_FS: opendir('%s') ok:\n", path);
    struct dirent *ent;
    int n = 0;
    while (n < 30 && (ent = readdir(dir)) != NULL) {
        SYS_Report("WII_FS:   %s\n", ent->d_name);
        n++;
    }
    closedir(dir);
}

static void wii_init_filesystem(void)
{
    SYS_Report("WII_FS: wii_init_filesystem entered (build %s)\n", __DATE__ " " __TIME__);
    // version check to detect stale binary
    SYS_Report("WII_FS: VERSION_TAG=KEEPERFX_WII_2026_03_06\n");

    SYS_Report("WII_FS: calling fatInitDefault...\n");
    const int fat_ok = fatInitDefault();
    SYS_Report("WII_FS: fatInitDefault=%d\n", fat_ok);
    if (!fat_ok) {
        SYS_Report("WII_FS: FAT mount failed - need SD/USB in Dolphin\n");
    }

    // Try to chdir to a location with KeeperFX files
    static const char *cwd_candidates[] = {
        "sd:/apps/keeperfx",
        "sd:/keeperfx",
        "sd:/",
        "usb:/apps/keeperfx",
        "usb:/keeperfx",
        "usb:/",
    };

    for (unsigned long i = 0; i < sizeof(cwd_candidates) / sizeof(cwd_candidates[0]); i++) {
        const char *candidate = cwd_candidates[i];
        int rc = chdir(candidate);
        SYS_Report("WII_FS: chdir('%s') = %d (errno=%d)\n", candidate, rc, errno);
        if (rc == 0) {
            wii_debug_list_dir(".");
            return;
        }
    }

    // Dump what we can see from current dir regardless
    SYS_Report("WII_FS: all chdir candidates failed, listing '.':\n");
    wii_debug_list_dir(".");
}

extern "C" int wii_kfx_entry(int argc, char *argv[])
{
    SYS_Report("WII_WRAP: wii_kfx_entry begin\n");
    (void)argc;
    (void)argv;
    static char app_path_bin[] = "bin/keeperfx_wii.dol";
    char* safe_argv[] = { app_path_bin, NULL };
    int safe_argc = 1;

    wii_init_filesystem();
    wii_bootstrap_banner();
    int result = kfxmain(safe_argc, safe_argv);
    return result;
}
