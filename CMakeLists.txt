cmake_minimum_required(VERSION 3.0)
project(xmrig-proxy)


include (CheckIncludeFile)


set(HEADERS
    src/3rdparty/align.h
    src/App.h
    src/Console.h
    src/interfaces/IClientListener.h
    src/interfaces/IConsoleListener.h
    src/interfaces/ILogBackend.h
    src/interfaces/IMinerListener.h
    src/interfaces/IServerListener.h
    src/interfaces/ISplitter.h
    src/interfaces/IStrategy.h
    src/interfaces/IStrategyListener.h
    src/log/ConsoleLog.h
    src/log/FileLog.h
    src/log/Log.h
    src/net/Client.h
    src/net/Job.h
    src/net/strategies/FailoverStrategy.h
    src/net/strategies/SinglePoolStrategy.h
    src/net/Url.h
    src/Options.h
    src/proxy/Addr.h
    src/proxy/Counters.h
    src/proxy/Hashrate.h
    src/proxy/JobResult.h
    src/proxy/JobResult.h
    src/proxy/Miner.h
    src/proxy/Miners.h
    src/proxy/Proxy.h
    src/proxy/Server.h
    src/proxy/splitters/NonceMapper.h
    src/proxy/splitters/NonceSplitter.h
    src/proxy/splitters/NonceStorage.h
    src/proxy/Uuid.h
    src/Summary.h
    src/version.h
   )

set(SOURCES
    src/App.cpp
    src/Console.cpp
    src/log/ConsoleLog.cpp
    src/log/FileLog.cpp
    src/log/Log.cpp
    src/net/Client.cpp
    src/net/Job.cpp
    src/net/strategies/FailoverStrategy.cpp
    src/net/strategies/SinglePoolStrategy.cpp
    src/net/Url.cpp
    src/Options.cpp
    src/proxy/Counters.cpp
    src/proxy/Hashrate.cpp
    src/proxy/JobResult.cpp
    src/proxy/LoginRequest.cpp
    src/proxy/Miner.cpp
    src/proxy/Miners.cpp
    src/proxy/Proxy.cpp
    src/proxy/Server.cpp
    src/proxy/splitters/NonceMapper.cpp
    src/proxy/splitters/NonceSplitter.cpp
    src/proxy/splitters/NonceStorage.cpp
    src/Summary.cpp
    src/xmrig.cpp
   )

if (WIN32)
    set(SOURCES_OS
        res/app.rc
        src/3rdparty/winansi.cpp
        src/3rdparty/winansi.h
        src/App_win.cpp
        src/proxy/Proxy_win.cpp
        src/proxy/Uuid_win.cpp
        )

    add_definitions(/DWIN32)
    set(EXTRA_LIBS ws2_32 psapi iphlpapi userenv)
elseif (APPLE)
    set(SOURCES_OS
        src/App_unix.cpp
        src/proxy/Proxy_mac.cpp
        )
else()
    set(SOURCES_OS
        src/App_unix.cpp
        src/proxy/Proxy_unix.cpp
        src/proxy/Uuid_unix.cpp
        )

    set(EXTRA_LIBS pthread uuid)
endif()

add_definitions(/DXMRIG_PROXY_PROJECT)
add_definitions(/DUNICODE)
add_definitions(/DAPP_DEVEL)
#add_definitions(/DAPP_DEBUG)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(UV REQUIRED)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()


# https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html
if (CMAKE_CXX_COMPILER_ID MATCHES GNU)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -maes -Wall -Wno-strict-aliasing")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -maes -Wall -std=c++14 -fno-exceptions -fno-rtti")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -s")

    if (WIN32)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
        add_definitions(/D__STDC_FORMAT_MACROS)
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    endif()

    add_definitions(/D_GNU_SOURCE)

    #set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -gdwarf-2")

elseif (CMAKE_CXX_COMPILER_ID MATCHES MSVC)

    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Ox /Ot /Oi /MT /GL")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /Ot /Oi /MT /GL")

elseif (CMAKE_CXX_COMPILER_ID MATCHES Clang)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -maes -Wall")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -funroll-loops -fmerge-all-constants")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -maes -Wall -std=c++14 -fno-exceptions -fno-rtti")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")

endif()

CHECK_INCLUDE_FILE (syslog.h HAVE_SYSLOG_H)
if (HAVE_SYSLOG_H)
    add_definitions(/DHAVE_SYSLOG_H)
    set(SOURCES_SYSLOG src/log/SysLog.h src/log/SysLog.cpp)
endif()

include_directories(src)
include_directories(src/3rdparty)
include_directories(src/3rdparty/jansson)
include_directories(${UV_INCLUDE_DIR})

add_subdirectory(src/3rdparty/jansson)

add_executable(${CMAKE_PROJECT_NAME} ${HEADERS} ${SOURCES} ${SOURCES_OS} ${SOURCES_SYSLOG})
target_link_libraries(${CMAKE_PROJECT_NAME} jansson ${UV_LIBRARIES} ${EXTRA_LIBS})
