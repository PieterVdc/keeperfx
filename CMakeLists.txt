cmake_minimum_required(VERSION 3.20)
project(keeperfx C CXX)

# ─── Project-wide settings ─────────────────────────────────────────────────────
set(CMAKE_C_STANDARD    11)
set(CMAKE_CXX_STANDARD  20)

# Enable FetchContent and ExternalProject for fallback builds
include(FetchContent)
include(ExternalProject)

# ─── OPTION: Allow disabling prebuilt usage ─────────────────────────────────────
# If ON (the default), try to download & import prebuilt deps/*.tar.gz. If that fails,
# fall back to building from source. If OFF, always build from source.
option(USE_DEPS_PREBUILTS "Use prebuilt dependency archives when available" ON)

# ─── ARCH DETECTION (MinGW32 vs MinGW64) ───────────────────────────────────────
# Only attempt prebuilt-archive logic when on Windows+MinGW (GCC or Clang).
if (WIN32 AND (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang"))
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(MINGW_ARCH "mingw64")
    else()
        set(MINGW_ARCH "mingw32")
    endif()
else()
    # On non-MinGW or non-Windows, skip the prebuilt-archive path.
    set(MINGW_ARCH "")
endif()

# ─── DEPS ROOT ───────────────────────────────────────────────────────────────────
set(DEPS_ROOT "${CMAKE_BINARY_DIR}/deps")
file(MAKE_DIRECTORY "${DEPS_ROOT}")

# ─── HELPER: TRY DOWNLOAD + EXTRACT ─────────────────────────────────────────────
# Tries to download ${URL} → ${ARCHIVE_PATH} and extract into ${DEST_DIR}.
# On success: sets ${RESULT_VAR} to 0. On any failure: sets ${RESULT_VAR} to 1.
function(try_prebuilt ARCHIVE_PATH URL DEST_DIR RESULT_VAR)
    if (NOT EXISTS "${DEST_DIR}")
        message(STATUS "  [deps] Attempting prebuilt: ${URL}")
        file(DOWNLOAD
            "${URL}"
            "${ARCHIVE_PATH}"
            SHOW_PROGRESS
            TLS_VERIFY ON
            STATUS __dl_status
            LOG    log_download
        )
        list(GET __dl_status 0 download_status)
        if (download_status STREQUAL "0")
            execute_process(
                COMMAND "${CMAKE_COMMAND}" -E tar xzf "${ARCHIVE_PATH}"
                WORKING_DIRECTORY "${DEPS_ROOT}"
                RESULT_VARIABLE _tar_status
            )
            if (_tar_status EQUAL 0)
                set(${RESULT_VAR} 0 PARENT_SCOPE)
            else()
                message(WARNING "  [deps] Extraction failed for ${ARCHIVE_PATH}")
                set(${RESULT_VAR} 1 PARENT_SCOPE)
            endif()
        else()
            message(STATUS "  [deps] Prebuilt not found or download failed (status=${download_status})")
            set(${RESULT_VAR} 1 PARENT_SCOPE)
        endif()
    else()
        # Already extracted → success
        set(${RESULT_VAR} 0 PARENT_SCOPE)
    endif()
endfunction()

# ─────────────────────────────────────────────────────────────────────────────────
# ─── 1) DEPENDENCIES: ENet ───────────────────────────────────────────────────────
set(ENET_TAG "initial")
if (USE_DEPS_PREBUILTS AND MINGW_ARCH)
    set(ENET_URL     "https://github.com/dkfans/kfx-deps/releases/download/${ENET_TAG}/enet-${MINGW_ARCH}.tar.gz")
    set(ENET_ARCHIVE "${DEPS_ROOT}/enet-${MINGW_ARCH}.tar.gz")
    set(ENET_DIR     "${DEPS_ROOT}/enet-${MINGW_ARCH}")
    try_prebuilt("${ENET_ARCHIVE}" "${ENET_URL}" "${ENET_DIR}" ENET_DL_OK)
else()
    set(ENET_DL_OK 1)
endif()

if (ENET_DL_OK EQUAL 0)
    add_library(enet STATIC IMPORTED)
    set_target_properties(enet PROPERTIES
        IMPORTED_LOCATION            "${ENET_DIR}/lib/libenet.a"
        INTERFACE_INCLUDE_DIRECTORIES "${ENET_DIR}/include"
    )
else()
    message(STATUS "  [deps] Falling back to building ENet from source")
    FetchContent_Declare(
      enet
      GIT_REPOSITORY  https://github.com/lsalzman/enet.git
      GIT_TAG         v1.3.20
    )
    FetchContent_MakeAvailable(enet)
    # Upstream’s CMake usually defines a target named “enet” or “enet_s”.
    # Create a consistent alias “enet” if needed:
    if (TARGET enet)
        add_library(enet_alias ALIAS enet)
        set_property(TARGET enet_alias PROPERTY ALIAS "enet")
    elseif (TARGET enet_s)
        add_library(enet_alias ALIAS enet_s)
        set_property(TARGET enet_alias PROPERTY ALIAS "enet")
    endif()
endif()

# ─── 2) ZLIB & MINIZIP ──────────────────────────────────────────────────────────
set(ZLIB_TAG "initial")
if (USE_DEPS_PREBUILTS AND MINGW_ARCH)
    set(ZLIB_URL     "https://github.com/dkfans/kfx-deps/releases/download/${ZLIB_TAG}/zlib-${MINGW_ARCH}.tar.gz")
    set(ZLIB_ARCHIVE "${DEPS_ROOT}/zlib-${MINGW_ARCH}.tar.gz")
    set(ZLIB_DIR     "${DEPS_ROOT}/zlib-${MINGW_ARCH}")
    try_prebuilt("${ZLIB_ARCHIVE}" "${ZLIB_URL}" "${ZLIB_DIR}" ZLIB_DL_OK)
else()
    set(ZLIB_DL_OK 1)
endif()

if (ZLIB_DL_OK EQUAL 0)
    add_library(zlib STATIC IMPORTED)
    set_target_properties(zlib PROPERTIES
        IMPORTED_LOCATION            "${ZLIB_DIR}/libz.a"
        INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_DIR}/include"
    )

    add_library(minizip STATIC IMPORTED)
    set_target_properties(minizip PROPERTIES
        IMPORTED_LOCATION            "${ZLIB_DIR}/libminizip.a"
        INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_DIR}/include"
    )
else()
    message(STATUS "  [deps] Falling back to building zlib (and minizip) from source")
    FetchContent_Declare(
      zlib
      GIT_REPOSITORY  https://github.com/madler/zlib.git
      GIT_TAG         v1.2.13
      SOURCE_SUBDIR   CMake
    )
    FetchContent_MakeAvailable(zlib)
    # zlib’s CMake creates zlib::zlib (and/or zlibstatic::zlib). We also need minizip:
    if (TARGET minizip)
        # Upstream provided “minizip” target.
    else()
        # If upstream didn’t define “minizip,” create an imported wrapper:
        add_library(minizip STATIC IMPORTED)
        set_target_properties(minizip PROPERTIES
            IMPORTED_LOCATION            "${zlib_BINARY_DIR}/libminizip.a"
            INTERFACE_INCLUDE_DIRECTORIES "${zlib_SOURCE_DIR}"
        )
    endif()
    # Create an alias “zlib” if necessary:
    if (TARGET zlibstatic)
        add_library(zlib_alias ALIAS zlibstatic)
        set_property(TARGET zlib_alias PROPERTY ALIAS "zlib")
    elseif (TARGET zlib)
        add_library(zlib_alias ALIAS zlib)
        set_property(TARGET zlib_alias PROPERTY ALIAS "zlib")
    endif()
endif()

# ─── 3) SPNG ─────────────────────────────────────────────────────────────────────
set(SPNG_TAG "initial")
if (USE_DEPS_PREBUILTS AND MINGW_ARCH)
    set(SPNG_URL     "https://github.com/dkfans/kfx-deps/releases/download/${SPNG_TAG}/spng-${MINGW_ARCH}.tar.gz")
    set(SPNG_ARCHIVE "${DEPS_ROOT}/spng-${MINGW_ARCH}.tar.gz")
    set(SPNG_DIR     "${DEPS_ROOT}/spng-${MINGW_ARCH}")
    try_prebuilt("${SPNG_ARCHIVE}" "${SPNG_URL}" "${SPNG_DIR}" SPNG_DL_OK)
else()
    set(SPNG_DL_OK 1)
endif()

if (SPNG_DL_OK EQUAL 0)
    add_library(spng STATIC IMPORTED)
    set_target_properties(spng PROPERTIES
        IMPORTED_LOCATION            "${SPNG_DIR}/libspng.a"
        INTERFACE_INCLUDE_DIRECTORIES "${SPNG_DIR}/include"
    )
else()
    message(STATUS "  [deps] Falling back to building SPNG from source")
    FetchContent_Declare(
      spng
      GIT_REPOSITORY  https://github.com/randy408/libspng.git
      GIT_TAG         v0.7.6
    )
    FetchContent_MakeAvailable(spng)
    # Upstream defines spng::spng; create alias “spng” if needed:
    if (TARGET spng_lib)
        add_library(spng_alias ALIAS spng_lib)
        set_property(TARGET spng_alias PROPERTY ALIAS "spng")
    endif()
endif()

# ─── 4) ASTRONOMY ────────────────────────────────────────────────────────────────
set(ASTRONOMY_TAG "astronomy_fix")
if (USE_DEPS_PREBUILTS AND MINGW_ARCH)
    set(ASTRONOMY_URL     "https://github.com/dkfans/kfx-deps/releases/download/${ASTRONOMY_TAG}/astronomy-${MINGW_ARCH}.tar.gz")
    set(ASTRONOMY_ARCHIVE "${DEPS_ROOT}/astronomy-${MINGW_ARCH}.tar.gz")
    set(ASTRONOMY_DIR     "${DEPS_ROOT}/astronomy-${MINGW_ARCH}")
    try_prebuilt("${ASTRONOMY_ARCHIVE}" "${ASTRONOMY_URL}" "${ASTRONOMY_DIR}" ASTRONOMY_DL_OK)
else()
    set(ASTRONOMY_DL_OK 1)
endif()

if (ASTRONOMY_DL_OK EQUAL 0)
    add_library(astronomy STATIC IMPORTED)
    set_target_properties(astronomy PROPERTIES
        IMPORTED_LOCATION            "${ASTRONOMY_DIR}/libastronomy.a"
        INTERFACE_INCLUDE_DIRECTORIES "${ASTRONOMY_DIR}/include"
    )
else()
    message(STATUS "  [deps] Falling back to building astronomy from source")
    FetchContent_Declare(
      astronomy
      GIT_REPOSITORY  https://github.com/dkfans/astronomy.git
      GIT_TAG         astronomy_fix
    )
    FetchContent_MakeAvailable(astronomy)
    # Upstream defines a target (e.g. astronomy::astronomy). Alias it to “astronomy” if needed.
    if (TARGET astronomy_lib)
        add_library(astronomy_alias ALIAS astronomy_lib)
        set_property(TARGET astronomy_alias PROPERTY ALIAS "astronomy")
    endif()
endif()

# ─── 5) CENTIJSON (JSON) ─────────────────────────────────────────────────────────
set(CENTIJSON_TAG "initial")
if (USE_DEPS_PREBUILTS AND MINGW_ARCH)
    set(CENTIJSON_URL     "https://github.com/dkfans/kfx-deps/releases/download/${CENTIJSON_TAG}/centijson-${MINGW_ARCH}.tar.gz")
    set(CENTIJSON_ARCHIVE "${DEPS_ROOT}/centijson-${MINGW_ARCH}.tar.gz")
    set(CENTIJSON_DIR     "${DEPS_ROOT}/centijson-${MINGW_ARCH}")
    try_prebuilt("${CENTIJSON_ARCHIVE}" "${CENTIJSON_URL}" "${CENTIJSON_DIR}" CENTIJSON_DL_OK)
else()
    set(CENTIJSON_DL_OK 1)
endif()

if (CENTIJSON_DL_OK EQUAL 0)
    add_library(json STATIC IMPORTED)
    set_target_properties(json PROPERTIES
        IMPORTED_LOCATION            "${CENTIJSON_DIR}/libjson.a"
        INTERFACE_INCLUDE_DIRECTORIES "${CENTIJSON_DIR}/include"
    )
else()
    message(STATUS "  [deps] Falling back to building centijson from source")
    FetchContent_Declare(
      centijson
      GIT_REPOSITORY  https://github.com/kazuho/centijson.git
      GIT_TAG         v1.3.1
    )
    FetchContent_MakeAvailable(centijson)
    if (TARGET centijson::json)
        add_library(json_alias ALIAS centijson::json)
        set_property(TARGET json_alias PROPERTY ALIAS "json")
    endif()
endif()

# ─── 6) FFMPEG (avformat, avcodec, swresample, avutil) ─────────────────────────
set(FFMPEG_TAG "initial")
if (USE_DEPS_PREBUILTS AND MINGW_ARCH)
    set(FFMPEG_URL      "https://github.com/dkfans/kfx-deps/releases/download/${FFMPEG_TAG}/ffmpeg-${MINGW_ARCH}.tar.gz")
    set(FFMPEG_ARCHIVE  "${DEPS_ROOT}/ffmpeg-${MINGW_ARCH}.tar.gz")
    set(FFMPEG_DIR      "${DEPS_ROOT}/ffmpeg-${MINGW_ARCH}")
    try_prebuilt("${FFMPEG_ARCHIVE}" "${FFMPEG_URL}" "${FFMPEG_DIR}" FFMPEG_DL_OK)
else()
    set(FFMPEG_DL_OK 1)
endif()

if (FFMPEG_DL_OK EQUAL 0)
    foreach(comp IN ITEMS avformat avcodec swresample avutil)
        add_library(${comp} STATIC IMPORTED)
        set_target_properties(${comp} PROPERTIES
            IMPORTED_LOCATION            "${FFMPEG_DIR}/${comp}/lib${comp}.a"
            INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_DIR}/${comp}"
        )
    endforeach()
else()
    message(STATUS "  [deps] Falling back to building FFmpeg from source (ExternalProject)")
    ExternalProject_Add(
      ffmpeg_ep
      GIT_REPOSITORY   https://github.com/FFmpeg/FFmpeg.git
      GIT_TAG          n5.0.1
      PREFIX           "${CMAKE_BINARY_DIR}/_deps/ffmpeg-build"
      CONFIGURE_COMMAND 
        ./configure 
          --disable-programs 
          --disable-doc 
          --disable-swscale 
          --enable-static=yes 
          --disable-shared 
          --prefix=<INSTALL_DIR>
      BUILD_COMMAND    make -j8
      INSTALL_COMMAND  make install
      BUILD_IN_SOURCE  1
    )
    set(FFMPEG_INSTALL_DIR "${CMAKE_BINARY_DIR}/_deps/ffmpeg-build/ffmpeg_ep-prefix/src/ffmpeg_ep-install")
    foreach(comp IN ITEMS avformat avcodec swresample avutil)
        add_library(${comp} STATIC IMPORTED)
        set_target_properties(${comp} PROPERTIES
            IMPORTED_LOCATION            "${FFMPEG_INSTALL_DIR}/lib/lib${comp}.a"
            INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INSTALL_DIR}/include"
        )
        add_dependencies(${comp} ffmpeg_ep)
    endforeach()
endif()

# ─── 7) OPENAL ──────────────────────────────────────────────────────────────────
set(OPENAL_TAG "2024-11-14")
if (USE_DEPS_PREBUILTS AND MINGW_ARCH)
    set(OPENAL_URL     "https://github.com/dkfans/kfx-deps/releases/download/${OPENAL_TAG}/openal-${MINGW_ARCH}.tar.gz")
    set(OPENAL_ARCHIVE "${DEPS_ROOT}/openal-${MINGW_ARCH}.tar.gz")
    set(OPENAL_DIR     "${DEPS_ROOT}/openal-${MINGW_ARCH}")
    try_prebuilt("${OPENAL_ARCHIVE}" "${OPENAL_URL}" "${OPENAL_DIR}" OPENAL_DL_OK)
else()
    set(OPENAL_DL_OK 1)
endif()

if (OPENAL_DL_OK EQUAL 0)
    add_library(OpenAL32 STATIC IMPORTED)
    set_target_properties(OpenAL32 PROPERTIES
        IMPORTED_LOCATION            "${OPENAL_DIR}/libOpenAL32.a"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_DIR}"
    )
else()
    message(STATUS "  [deps] Falling back to building OpenAL from source")
    FetchContent_Declare(
      openal
      GIT_REPOSITORY  https://github.com/kcat/openal-soft.git
      GIT_TAG         1.23.1
    )
    FetchContent_MakeAvailable(openal)
    # Upstream defines something like OpenAL_Soft::OpenAL_Soft or OpenAL::OpenAL
    if (TARGET OpenAL_Soft)
        add_library(OpenAL32_alias ALIAS OpenAL_Soft)
        set_property(TARGET OpenAL32_alias PROPERTY ALIAS "OpenAL32")
    elseif (TARGET OpenAL)
        add_library(OpenAL32_alias ALIAS OpenAL)
        set_property(TARGET OpenAL32_alias PROPERTY ALIAS "OpenAL32")
    endif()
endif()

# ─── 8) LUAJIT ──────────────────────────────────────────────────────────────────
set(LUAJIT_TAG "20250418")
if (USE_DEPS_PREBUILTS AND MINGW_ARCH)
    set(LUAJIT_URL     "https://github.com/dkfans/kfx-deps/releases/download/${LUAJIT_TAG}/luajit-${MINGW_ARCH}.tar.gz")
    set(LUAJIT_ARCHIVE "${DEPS_ROOT}/luajit-${MINGW_ARCH}.tar.gz")
    set(LUAJIT_DIR     "${DEPS_ROOT}/luajit-${MINGW_ARCH}")
    try_prebuilt("${LUAJIT_ARCHIVE}" "${LUAJIT_URL}" "${LUAJIT_DIR}" LUAJIT_DL_OK)
else()
    set(LUAJIT_DL_OK 1)
endif()

if (LUAJIT_DL_OK EQUAL 0)
    add_library(luajit STATIC IMPORTED)
    set_target_properties(luajit PROPERTIES
        IMPORTED_LOCATION            "${LUAJIT_DIR}/lib/libluajit.a"
        INTERFACE_INCLUDE_DIRECTORIES "${LUAJIT_DIR}/include"
    )
else()
    message(STATUS "  [deps] Falling back to building LuaJIT from source")
    FetchContent_Declare(
      luajit
      GIT_REPOSITORY  https://github.com/LuaJIT/LuaJIT.git
      GIT_TAG         v2.1.0-beta3
    )
    FetchContent_MakeAvailable(luajit)
    # Upstream defines LuaJIT::LuaJIT or just LuaJIT
    if (TARGET LuaJIT)
        add_library(luajit_alias ALIAS LuaJIT)
        set_property(TARGET luajit_alias PROPERTY ALIAS "luajit")
    endif()
endif()

# ─────────────────────────────────────────────────────────────────────────────────
# ─── 9) PROJECT SOURCES & EXECUTABLES ────────────────────────────────────────────

# Replace the following lists with your actual source files:
# e.g.: file(GLOB_RECURSE KEEPERFX_C_SOURCES src/*.c)
#       file(GLOB_RECURSE KEEPERFX_CXX_SOURCES src/*.cpp)
set(KEEPERFX_C_SOURCES
    # List your .c files here
)
set(KEEPERFX_CXX_SOURCES
    # List your .cpp/.cxx files here
)

add_executable(keeperfx
    ${KEEPERFX_C_SOURCES}
    ${KEEPERFX_CXX_SOURCES}
)

target_link_libraries(keeperfx
    PRIVATE
        SDL2::SDL2
        SDL2::SDL2main
        SDL2_net::SDL2_net
        SDL2_image::SDL2_image

        enet          # from prebuilt or FetchContent alias
        zlib          # ditto
        minizip       # ditto
        spng          # ditto
        astronomy     # ditto
        json          # ditto
        avformat      # ditto
        avcodec       # ditto
        swresample    # ditto
        avutil        # ditto
        OpenAL32      # ditto
        luajit        # ditto
)

add_executable(keeperfx_hvlog
    ${KEEPERFX_C_SOURCES}
    ${KEEPERFX_CXX_SOURCES}
)

target_link_libraries(keeperfx_hvlog
    PRIVATE
        SDL2::SDL2
        SDL2::SDL2main
        SDL2_net::SDL2_net
        SDL2_image::SDL2_image

        enet
        zlib
        minizip
        spng
        astronomy
        json
        avformat
        avcodec
        swresample
        avutil
        OpenAL32
        luajit
)
