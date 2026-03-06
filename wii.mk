include version.mk

BUILD_NUMBER ?= $(VER_BUILD)
VER_SUFFIX ?= Prototype
VER_STRING = $(VER_MAJOR).$(VER_MINOR).$(VER_RELEASE).$(BUILD_NUMBER) $(VER_SUFFIX)

ifeq ($(strip $(DEVKITPRO)),)
ifneq ($(wildcard D:/devkitPro),)
DEVKITPRO := D:/devkitPro
else ifneq ($(wildcard C:/devkitPro),)
DEVKITPRO := C:/devkitPro
endif
endif

ifeq ($(strip $(DEVKITPRO)),)
$(error DEVKITPRO is not set. Install devkitPro and set DEVKITPRO, or install to D:/devkitPro)
endif

DEVKITPPC ?= $(DEVKITPRO)/devkitPPC
ifeq ($(wildcard $(DEVKITPPC)/wii_rules),)
$(error Missing $(DEVKITPPC)/wii_rules. Install devkitPPC + libogc (dkp-pacman -S wii-dev))
endif

include $(DEVKITPPC)/wii_rules

ifeq (,$(findstring Windows,$(OS)))
HOST_EXEEXT :=
else
HOST_EXEEXT := .exe
endif

MKDIR ?= mkdir -p
ECHO ?= echo
MV ?= mv -f
ELF2DOL ?= $(DEVKITPRO)/tools/bin/elf2dol$(HOST_EXEEXT)

KFX_SOURCES := \
	$(wildcard src/*.c) \
	$(wildcard src/*.cpp) \
	$(wildcard src/kfx/lense/*.cpp)

KFX_SOURCES := $(filter-out \
	src/linux.cpp \
	src/windows.cpp \
	src/steam_api.cpp \
	src/bflib_enet.cpp \
	src/bflib_fmvids.cpp \
	src/bflib_input_joyst.cpp \
	src/net_portforward.cpp \
	src/api.c \
	src/bflib_dialog.c \
	src/lua_api.c \
	src/lua_api_lens.c \
	src/lua_api_player.c \
	src/lua_api_room.c \
	src/lua_api_slabs.c \
	src/lua_api_things.c \
	src/lua_base.c \
	src/lua_cfg_funcs.c \
	src/lua_params.c \
	src/lua_triggers.c \
	src/lua_utils.c \
	src/kfx/lense/LuaLensEffect.cpp \
	src/scrcapt.c \
	src/net_checksums.c \
	src/net_resync.cpp, \
	$(KFX_SOURCES))

KFX_C_SOURCES = $(filter %.c,$(KFX_SOURCES))
KFX_CXX_SOURCES = $(filter %.cpp,$(KFX_SOURCES))
KFX_C_OBJECTS = $(patsubst src/%.c,obj/%.o,$(KFX_C_SOURCES))
KFX_CXX_OBJECTS = $(patsubst src/%.cpp,obj/%.o,$(KFX_CXX_SOURCES))
KFX_OBJECTS = $(KFX_C_OBJECTS) $(KFX_CXX_OBJECTS)

TOML_SOURCES = \
	deps/centitoml/toml_api.c
TOML_OBJECTS = $(patsubst deps/centitoml/%.c,obj/centitoml/%.o,$(TOML_SOURCES))

KFX_INCLUDES = \
	-Ideps/centijson/include \
	-Ideps/centitoml \
	-Ideps/astronomy/include \
	-Ideps/enet6/include \
	-Ideps/spng/include \
	-Ideps/zlib/include

KFX_COMMON_FLAGS = $(MACHDEP) -DGEKKO -DHW_RVL -DPLATFORM_WII -D__WII__ -DDEBUG -DBFDEBUG_LEVEL=0 -O2 -fsigned-char $(KFX_INCLUDES) -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -Wno-format-truncation -Wno-sign-compare
KFX_CFLAGS += $(KFX_COMMON_FLAGS)
KFX_CXXFLAGS += $(KFX_COMMON_FLAGS)

TOML_CFLAGS += $(MACHDEP) -O2 -fsigned-char -Ideps/centijson/include -Wall -Wextra -Wno-unused-parameter

KFX_LDFLAGS += \
	$(MACHDEP) \
	-Wl,-Map,bin/keeperfx_wii.map \
	-L$(DEVKITPRO)/libogc/lib/wii \
	-L$(DEVKITPRO)/portlibs/wii/lib \
	-logc \
	-lm

ifeq ($(ENABLE_LTO), 1)
KFX_CFLAGS += -flto
KFX_CXXFLAGS += -flto
KFX_LDFLAGS += -flto=auto
TOML_CFLAGS += -flto
endif

all: bin/keeperfx_wii.dol

clean:
	rm -rf obj bin src/ver_defs.h

.PHONY: all clean

bin/keeperfx_wii.elf: $(KFX_OBJECTS) $(TOML_OBJECTS) | bin
	$(CXX) -o $@ $(KFX_OBJECTS) $(TOML_OBJECTS) $(KFX_LDFLAGS)

bin/keeperfx_wii.dol: bin/keeperfx_wii.elf | bin
	$(ELF2DOL) $< $@

$(KFX_C_OBJECTS): obj/%.o: src/%.c src/ver_defs.h | obj
	$(MKDIR) $(dir $@)
	$(CC) $(KFX_CFLAGS) -c $< -o $@

$(KFX_CXX_OBJECTS): obj/%.o: src/%.cpp src/ver_defs.h | obj
	$(MKDIR) $(dir $@)
	$(CXX) $(KFX_CXXFLAGS) -c $< -o $@

$(TOML_OBJECTS): obj/centitoml/%.o: deps/centitoml/%.c wii.mk | obj/centitoml
	$(CC) $(TOML_CFLAGS) -c $< -o $@

bin obj obj/centitoml:
	$(MKDIR) $@

src/ver_defs.h: version.mk
	$(ECHO) "#define VER_MAJOR   $(VER_MAJOR)" > $@.swp
	$(ECHO) "#define VER_MINOR   $(VER_MINOR)" >> $@.swp
	$(ECHO) "#define VER_RELEASE $(VER_RELEASE)" >> $@.swp
	$(ECHO) "#define VER_BUILD   $(BUILD_NUMBER)" >> $@.swp
	$(ECHO) "#define VER_STRING  \"$(VER_STRING)\"" >> $@.swp
	$(ECHO) "#define PACKAGE_SUFFIX  \"$(VER_SUFFIX)\"" >> $@.swp
	$(ECHO) "#define GIT_REVISION  \"$(shell git describe --always)\"" >> $@.swp
	$(MV) $@.swp $@
