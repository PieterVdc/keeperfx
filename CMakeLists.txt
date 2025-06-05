cmake_minimum_required(VERSION 3.20)
project(keeperfx C CXX)

set(CMAKE_C_STANDARD    11)
set(CMAKE_CXX_STANDARD  20)

include(FetchContent)
include(ExternalProject)

option(USE_DEPS_PREBUILTS "Use prebuilt dependency archives when available (Windows+MinGW only)" ON)

# Detect platform/arch:
if (WIN32 AND (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang"))
    set(USING_MINGW    ON)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(MINGW_ARCH  "mingw64")
    else()
        set(MINGW_ARCH  "mingw32")
    endif()
else()
    set(USING_MINGW    OFF)
    set(MINGW_ARCH    "")
endif()

# On Linux, we'll try find_package(...) first, then fallback to FetchContent.
if (UNIX AND NOT APPLE AND NOT USING_MINGW)
    set(USING_LINUX  ON)
else()
    set(USING_LINUX  OFF)
endif()

# Common “deps” root folder (only used by MinGW prebuilt path)
set(DEPS_ROOT "${CMAKE_BINARY_DIR}/deps")
file(MAKE_DIRECTORY "${DEPS_ROOT}")

#
# ─── Helper: Try download+extract a MinGW tar.gz ─────────────────────────────────
# Used only when USING_MINGW=ON and USE_DEPS_PREBUILTS=ON.
#
function(try_prebuilt ARCHIVE_PATH URL DEST_DIR RESULT_VAR)
    if (NOT EXISTS "${DEST_DIR}")
        message(STATUS "  [deps] Attempting prebuilt: ${URL}")
        file(DOWNLOAD
            "${URL}"
            "${ARCHIVE_PATH}"
            SHOW_PROGRESS
            TLS_VERIFY ON
            STATUS __dl_status
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
        set(${RESULT_VAR} 0 PARENT_SCOPE)
    endif()
endfunction()

#
# ─────────────────────────────────────────────────────────────────────────────────
# ─── 1) ENet ────────────────────────────────────────────────────────────────────
#
if (USING_MINGW AND USE_DEPS_PREBUILTS)
    # (MinGW prebuilt path)
    set(ENET_TAG       "initial")
    set(ENET_URL       "https://github.com/dkfans/kfx-deps/releases/download/${ENET_TAG}/enet-${MINGW_ARCH}.tar.gz")
    set(ENET_ARCHIVE   "${DEPS_ROOT}/enet-${MINGW_ARCH}.tar.gz")
    set(ENET_DIR       "${DEPS_ROOT}/enet-${MINGW_ARCH}")

    try_prebuilt("${ENET_ARCHIVE}" "${ENET_URL}" "${ENET_DIR}" ENET_DL_OK)
else()
    set(ENET_DL_OK 1)
endif()

if (ENET_DL_OK EQUAL 0)
    #  ─── Imported from MinGW tarball ─────────────────────────
    add_library(enet STATIC IMPORTED)
    set_target_properties(enet PROPERTIES
        IMPORTED_LOCATION            "${ENET_DIR}/lib/libenet.a"
        INTERFACE_INCLUDE_DIRECTORIES "${ENET_DIR}/include"
    )
elseif (USING_LINUX)
    #  ─── Linux: try system package first ───────────────────────
    find_package(ENet)
    if (ENET_FOUND)
        # Upstream CMake usually provides an imported target ENet::ENet or just ENet::enet
        # Alias it to “enet” for consistency:
        if (TARGET ENet::ENet)
            add_library(enet_alias ALIAS ENet::ENet)
            set_property(TARGET enet_alias PROPERTY ALIAS "enet")
        elseif (TARGET enet)
            # if upstream already names it “enet”
        endif()
    else()
        #  ─── Linux fallback: build from source via FetchContent ───
        message(STATUS "  [deps] Falling back to building ENet from source (Linux)")
        FetchContent_Declare(
          enet
          GIT_REPOSITORY  https://github.com/lsalzman/enet.git
          GIT_TAG         v1.3.20
        )
        FetchContent_MakeAvailable(enet)
        # Upstream export might be “enet::enet” or “enet_s”; create alias “enet”:
        if (TARGET enet)
            add_library(enet_alias ALIAS enet)
            set_property(TARGET enet_alias PROPERTY ALIAS "enet")
        elseif (TARGET enet_s)
            add_library(enet_alias ALIAS enet_s)
            set_property(TARGET enet_alias PROPERTY ALIAS "enet")
        endif()
    endif()
else()
    #  ─── Windows+non-prebuilt: build from source ───────────────
    message(STATUS "  [deps] Falling back to building ENet from source (non-Mingw or forced)")
    FetchContent_Declare(
      enet
      GIT_REPOSITORY  https://github.com/lsalzman/enet.git
      GIT_TAG         v1.3.20
    )
    FetchContent_MakeAvailable(enet)
    if (TARGET enet)
        add_library(enet_alias ALIAS enet)
        set_property(TARGET enet_alias PROPERTY ALIAS "enet")
    elseif (TARGET enet_s)
        add_library(enet_alias ALIAS enet_s)
        set_property(TARGET enet_alias PROPERTY ALIAS "enet")
    endif()
endif()


#
# ─── 2) ZLIB & MINIZIP ──────────────────────────────────────────────────────────
#
if (USING_MINGW AND USE_DEPS_PREBUILTS)
    set(ZLIB_TAG       "initial")
    set(ZLIB_URL       "https://github.com/dkfans/kfx-deps/releases/download/${ZLIB_TAG}/zlib-${MINGW_ARCH}.tar.gz")
    set(ZLIB_ARCHIVE   "${DEPS_ROOT}/zlib-${MINGW_ARCH}.tar.gz")
    set(ZLIB_DIR       "${DEPS_ROOT}/zlib-${MINGW_ARCH}")
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
elseif (USING_LINUX)
    find_package(ZLIB REQUIRED)    # typically provides ZLIB::ZLIB
    if (TARGET ZLIB::ZLIB)
        add_library(zlib_alias ALIAS ZLIB::ZLIB)
        set_property(TARGET zlib_alias PROPERTY ALIAS "zlib")
    endif()
    # Minizip: on many Linux distros, “minizip” is part of zlib’s development package
    # and you link with “ZLIB::ZLIB” only.  If you truly need a separate “minizip” target,
    # you can treat it as the same as zlib, or write a small wrapper.
    add_library(minizip_alias ALIAS ZLIB::ZLIB)
    set_property(TARGET minizip_alias PROPERTY ALIAS "minizip")
else()
    message(STATUS "  [deps] Falling back to building zlib (and minizip) from source)")
    FetchContent_Declare(
      zlib
      GIT_REPOSITORY  https://github.com/madler/zlib.git
      GIT_TAG         v1.2.13
      SOURCE_SUBDIR   CMake
    )
    FetchContent_MakeAvailable(zlib)
    if (TARGET zlibstatic)
        add_library(zlib_alias ALIAS zlibstatic)
        set_property(TARGET zlib_alias PROPERTY ALIAS "zlib")
    elseif (TARGET zlib)
        add_library(zlib_alias ALIAS zlib)
        set_property(TARGET zlib_alias PROPERTY ALIAS "zlib")
    endif()
    # For minizip, check if upstream CMake defined it:
    if (TARGET minizip)
        add_library(minizip_alias ALIAS minizip)
        set_property(TARGET minizip_alias PROPERTY ALIAS "minizip")
    else()
        # If not, create a wrapper that points at zlib’s “libminizip.a” in the build folder
        add_library(minizip STATIC IMPORTED)
        set_target_properties(minizip PROPERTIES
            IMPORTED_LOCATION "${zlib_BINARY_DIR}/libminizip.a"
            INTERFACE_INCLUDE_DIRECTORIES "${zlib_SOURCE_DIR}"
        )
    endif()
endif()


#
# ─── 3) SPNG ─────────────────────────────────────────────────────────────────────
#
if (USING_MINGW AND USE_DEPS_PREBUILTS)
    set(SPNG_TAG       "initial")
    set(SPNG_URL       "https://github.com/dkfans/kfx-deps/releases/download/${SPNG_TAG}/spng-${MINGW_ARCH}.tar.gz")
    set(SPNG_ARCHIVE   "${DEPS_ROOT}/spng-${MINGW_ARCH}.tar.gz")
    set(SPNG_DIR       "${DEPS_ROOT}/spng-${MINGW_ARCH}")
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
elseif (USING_LINUX)
    find_package(PNG)             # many distros bundle “libspng” under system libpng, but you might need to install libspng-dev
    if (TARGET spng::spng)
        add_library(spng_alias ALIAS spng::spng)
        set_property(TARGET spng_alias PROPERTY ALIAS "spng")
    else()
        message(STATUS "  [deps] SPNG not found as system package—building from source")
        FetchContent_Declare(
          spng
          GIT_REPOSITORY  https://github.com/randy408/libspng.git
          GIT_TAG         v0.7.6
        )
        FetchContent_MakeAvailable(spng)
        if (TARGET spng)
            add_library(spng_alias ALIAS spng)
            set_property(TARGET spng_alias PROPERTY ALIAS "spng")
        endif()
    endif()
else()
    message(STATUS "  [deps] Falling back to building SPNG from source (non-Mingw)")
    FetchContent_Declare(
      spng
      GIT_REPOSITORY  https://github.com/randy408/libspng.git
      GIT_TAG         v0.7.6
    )
    FetchContent_MakeAvailable(spng)
    if (TARGET spng)
        add_library(spng_alias ALIAS spng)
        set_property(TARGET spng_alias PROPERTY ALIAS "spng")
    endif()
endif()


#
# ─── 4) ASTRONOMY ────────────────────────────────────────────────────────────────
#
if (USING_MINGW AND USE_DEPS_PREBUILTS)
    set(ASTRONOMY_TAG       "astronomy_fix")
    set(ASTRONOMY_URL       "https://github.com/dkfans/kfx-deps/releases/download/${ASTRONOMY_TAG}/astronomy-${MINGW_ARCH}.tar.gz")
    set(ASTRONOMY_ARCHIVE   "${DEPS_ROOT}/astronomy-${MINGW_ARCH}.tar.gz")
    set(ASTRONOMY_DIR       "${DEPS_ROOT}/astronomy-${MINGW_ARCH}")
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
elseif (USING_LINUX)
    # There is usually no system “astronomy” library—build from source.
    message(STATUS "  [deps] Building astronomy from source (Linux)")
    FetchContent_Declare(
      astronomy
      GIT_REPOSITORY  https://github.com/dkfans/astronomy.git
      GIT_TAG         astronomy_fix
    )
    FetchContent_MakeAvailable(astronomy)
    if (TARGET astronomy_lib)
        add_library(astronomy_alias ALIAS astronomy_lib)
        set_property(TARGET astronomy_alias PROPERTY ALIAS "astronomy")
    endif()
else()
    message(STATUS "  [deps] Falling back to building astronomy from source (non-Mingw)")
    FetchContent_Declare(
      astronomy
      GIT_REPOSITORY  https://github.com/dkfans/astronomy.git
      GIT_TAG         astronomy_fix
    )
    FetchContent_MakeAvailable(astronomy)
    if (TARGET astronomy_lib)
        add_library(astronomy_alias ALIAS astronomy_lib)
        set_property(TARGET astronomy_alias PROPERTY ALIAS "astronomy")
    endif()
endif()


#
# ─── 5) CENTIJSON (JSON) ─────────────────────────────────────────────────────────
#
if (USING_MINGW AND USE_DEPS_PREBUILTS)
    set(CENTIJSON_TAG       "initial")
    set(CENTIJSON_URL       "https://github.com/dkfans/kfx-deps/releases/download/${CENTIJSON_TAG}/centijson-${MINGW_ARCH}.tar.gz")
    set(CENTIJSON_ARCHIVE   "${DEPS_ROOT}/centijson-${MINGW_ARCH}.tar.gz")
    set(CENTIJSON_DIR       "${DEPS_ROOT}/centijson-${MINGW_ARCH}")
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
elseif (USING_LINUX)
    # Many distros have a “libjansson” or similar, but centijson is not always packaged.
    find_package(Jansson)  # if you want to substitute Jansson for centijson
    if (Jansson_FOUND)
        add_library(json_alias ALIAS Jansson::Jansson)
        set_property(TARGET json_alias PROPERTY ALIAS "json")
    else()
        message(STATUS "  [deps] Building centijson from source (Linux)")
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
else()
    message(STATUS "  [deps] Falling back to building centijson from source (non-Mingw)")
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


#
# ─── 6) FFMPEG (avformat, avcodec, swresample, avutil) ─────────────────────────
#
if (USING_MINGW AND USE_DEPS_PREBUILTS)
    set(FFMPEG_TAG       "initial")
    set(FFMPEG_URL       "https://github.com/dkfans/kfx-deps/releases/download/${FFMPEG_TAG}/ffmpeg-${MINGW_ARCH}.tar.gz")
    set(FFMPEG_ARCHIVE   "${DEPS_ROOT}/ffmpeg-${MINGW_ARCH}.tar.gz")
    set(FFMPEG_DIR       "${DEPS_ROOT}/ffmpeg-${MINGW_ARCH}")
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

elseif (USING_LINUX)
    # On most Linux distros, you can install libavformat-dev, libavcodec-dev, etc.
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(AVFORMAT   REQUIRED libavformat)
    pkg_check_modules(AVCODEC    REQUIRED libavcodec)
    pkg_check_modules(SWRESAMPLE REQUIRED libswresample)
    pkg_check_modules(AVUTIL     REQUIRED libavutil)

    add_library(avformat_alias STATIC IMPORTED)
    set_target_properties(avformat_alias PROPERTIES
        IMPORTED_LOCATION            "${AVFORMAT_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${AVFORMAT_INCLUDE_DIRS}"
    )
    set_property(TARGET avformat_alias PROPERTY ALIAS "avformat")

    add_library(avcodec_alias STATIC IMPORTED)
    set_target_properties(avcodec_alias PROPERTIES
        IMPORTED_LOCATION            "${AVCODEC_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${AVCODEC_INCLUDE_DIRS}"
    )
    set_property(TARGET avcodec_alias PROPERTY ALIAS "avcodec")

    add_library(swresample_alias STATIC IMPORTED)
    set_target_properties(swresample_alias PROPERTIES
        IMPORTED_LOCATION            "${SWRESAMPLE_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${SWRESAMPLE_INCLUDE_DIRS}"
    )
    set_property(TARGET swresample_alias PROPERTY ALIAS "swresample")

    add_library(avutil_alias STATIC IMPORTED)
    set_target_properties(avutil_alias PROPERTIES
        IMPORTED_LOCATION            "${AVUTIL_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${AVUTIL_INCLUDE_DIRS}"
    )
    set_property(TARGET avutil_alias PROPERTY ALIAS "avutil")

else()
    message(STATUS "  [deps] Falling back to building FFmpeg from source (non-Mingw/Linux)")
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


#
# ─── 7) OPENAL ──────────────────────────────────────────────────────────────────
#
if (USING_MINGW AND USE_DEPS_PREBUILTS)
    set(OPENAL_TAG       "2024-11-14")
    set(OPENAL_URL       "https://github.com/dkfans/kfx-deps/releases/download/${OPENAL_TAG}/openal-${MINGW_ARCH}.tar.gz")
    set(OPENAL_ARCHIVE   "${DEPS_ROOT}/openal-${MINGW_ARCH}.tar.gz")
    set(OPENAL_DIR       "${DEPS_ROOT}/openal-${MINGW_ARCH}")
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
elseif (USING_LINUX)
    find_package(OpenAL REQUIRED)
    if (TARGET OpenAL::OpenAL)
        add_library(OpenAL32_alias ALIAS OpenAL::OpenAL)
        set_property(TARGET OpenAL32_alias PROPERTY ALIAS "OpenAL32")
    endif()
else()
    message(STATUS "  [deps] Falling back to building OpenAL from source (non-Mingw/Linux)")
    FetchContent_Declare(
      openal
      GIT_REPOSITORY  https://github.com/kcat/openal-soft.git
      GIT_TAG         1.23.1
    )
    FetchContent_MakeAvailable(openal)
    if (TARGET OpenAL_Soft)
        add_library(OpenAL32_alias ALIAS OpenAL_Soft)
        set_property(TARGET OpenAL32_alias PROPERTY ALIAS "OpenAL32")
    elseif (TARGET OpenAL)
        add_library(OpenAL32_alias ALIAS OpenAL)
        set_property(TARGET OpenAL32_alias PROPERTY ALIAS "OpenAL32")
    endif()
endif()


#
# ─── 8) LUAJIT ──────────────────────────────────────────────────────────────────
#
if (USING_MINGW AND USE_DEPS_PREBUILTS)
    set(LUAJIT_TAG       "20250418")
    set(LUAJIT_URL       "https://github.com/dkfans/kfx-deps/releases/download/${LUAJIT_TAG}/luajit-${MINGW_ARCH}.tar.gz")
    set(LUAJIT_ARCHIVE   "${DEPS_ROOT}/luajit-${MINGW_ARCH}.tar.gz")
    set(LUAJIT_DIR       "${DEPS_ROOT}/luajit-${MINGW_ARCH}")
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
elseif (USING_LINUX)
    find_package(LuaJIT REQUIRED)
    if (TARGET LuaJIT::LuaJIT)
        add_library(luajit_alias ALIAS LuaJIT::LuaJIT)
        set_property(TARGET luajit_alias PROPERTY ALIAS "luajit")
    else()
        message(STATUS "  [deps] Building LuaJIT from source (Linux)")
        FetchContent_Declare(
          luajit
          GIT_REPOSITORY  https://github.com/LuaJIT/LuaJIT.git
          GIT_TAG         v2.1.0-beta3
        )
        FetchContent_MakeAvailable(luajit)
        if (TARGET LuaJIT)
            add_library(luajit_alias ALIAS LuaJIT)
            set_property(TARGET luajit_alias PROPERTY ALIAS "luajit")
        endif()
    endif()
else()
    message(STATUS "  [deps] Falling back to building LuaJIT from source (non-Mingw/Linux)")
    FetchContent_Declare(
      luajit
      GIT_REPOSITORY  https://github.com/LuaJIT/LuaJIT.git
      GIT_TAG         v2.1.0-beta3
    )
    FetchContent_MakeAvailable(luajit)
    if (TARGET LuaJIT)
        add_library(luajit_alias ALIAS LuaJIT)
        set_property(TARGET luajit_alias PROPERTY ALIAS "luajit")
    endif()
endif()


# ─────────────────────────────────────────────────────────────────────────────────
# At this point, you have a target name “enet”, “zlib”, “minizip”, “spng”, “astronomy”,
# “json”, “avformat”, “avcodec”, “swresample”, “avutil”, “OpenAL32”, “luajit” — either
# as an imported‐from‐tarball (Windows+MinGW), as a system‐package alias (Linux), or
# as a FetchContent/ExternalProject‐built target.

#
# ─── PROJECT EXECUTABLES ──────────────────────────────────────────────────────────

# (Replace these with your actual source‐file lists.)
set(KEEPERFX_C_SOURCES
    # e.g. src/main.c, src/… .c
)
set(KEEPERFX_CXX_SOURCES
    # e.g. src/game.cpp, src/… .cpp
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
