#include "platform.h"

extern "C" {
void* __ppc_main_sp = (void*)0x817FFFE0;
void* __ppc_excpt_sp = (void*)0x817FBFE0;
}

extern "C" int main(int argc, char *argv[])
{
    return kfxmain(argc, argv);
}
