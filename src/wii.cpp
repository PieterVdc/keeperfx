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
#include <ogc/ios.h>
#include <cstdlib>

#include <ogc/disc_io.h>

#include <tuxedo/ppc/exception.h>

#include <ogc/system.h>

/**************** */

#include <unwind.h>
#include <ogc/system.h>
#include <stdint.h>

struct BacktraceData
{
    int depth;
};

static _Unwind_Reason_Code trace_callback(
    struct _Unwind_Context *context,
    void *arg)
{
    BacktraceData *data = (BacktraceData *)arg;

    uintptr_t ip = _Unwind_GetIP(context);

    if (ip)
    {
        SYS_Report("#%-2d PC=%08x\n",
            data->depth,
            (unsigned int)ip);

        data->depth++;
    }

    if (data->depth >= 32)
        return _URC_END_OF_STACK;

    return _URC_NO_REASON;
}

void dump_backtrace()
{
    BacktraceData data = {};
    SYS_Report("Backtrace:\n");

    _Unwind_Backtrace(trace_callback, &data);
}

static void my_panic_handler(unsigned exid, PPCContext* ctx)
{
    SYS_Report("\n=== KeeperFX exception ===\n");
    SYS_Report("Exception: %u\n", exid);
    SYS_Report("PC: %08x\n", ctx->pc);
    SYS_Report("LR: %08x\n", ctx->lr);

    dump_backtrace();

    while (1)
    {
        // wait forever
    }


}

void install_exception_handler(void)
{
    PPCExcptCurPanicFn = my_panic_handler;
}

/***********/

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

}

static void wii_init_filesystem(void)
{
    __IOS_LoadStartupIOS();
    
    extern DISC_INTERFACE __io_wiisd;

    SYS_Report("startup=%d\n", __io_wiisd.startup());
    SYS_Report("isInserted=%d\n", __io_wiisd.isInserted());

    SYS_Report("WII_FS: wii_init_filesystem entered (build %s)\n", __DATE__ " " __TIME__);

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
            return;
        }
    }

    // Dump what we can see from current dir regardless
    SYS_Report("WII_FS: all chdir candidates failed, listing '.':\n");
}

extern "C" int main(int argc, char *argv[])
{
    SYS_Report("WII_WRAP: wii_kfx_entry begin\n");
    (void)argc;
    (void)argv;
    static char app_path_bin[] = "bin/keeperfx_wii.dol";
    char* safe_argv[] = { app_path_bin, NULL };
    int safe_argc = 1;

    install_exception_handler();
    wii_init_filesystem();
    wii_bootstrap_banner();

    int result = kfxmain(safe_argc, safe_argv);
    SYS_Report("WII_WRAP: end\n");
    return result;
}
