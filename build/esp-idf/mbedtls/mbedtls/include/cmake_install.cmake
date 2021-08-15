# Install script for directory: /Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Users/Lucas/.espressif/tools/xtensa-esp32s2-elf/esp-2021r1-8.4.0/xtensa-esp32s2-elf/bin/xtensa-esp32s2-elf-objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mbedtls" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aes.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aesni.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/arc4.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aria.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/asn1.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/asn1write.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/base64.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/bignum.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/blowfish.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/bn_mul.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/camellia.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ccm.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/certs.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/chacha20.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/chachapoly.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/check_config.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cipher.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cipher_internal.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cmac.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/compat-1.3.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/config.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ctr_drbg.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/debug.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/des.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/dhm.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecdh.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecdsa.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecjpake.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecp.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecp_internal.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/entropy.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/entropy_poll.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/error.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/gcm.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/havege.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/hkdf.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/hmac_drbg.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md2.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md4.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md5.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md_internal.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/memory_buffer_alloc.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/net.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/net_sockets.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/nist_kw.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/oid.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/padlock.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pem.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pk.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pk_internal.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs11.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs12.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs5.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform_time.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform_util.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/poly1305.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ripemd160.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/rsa.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/rsa_internal.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha1.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha256.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha512.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_cache.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_ciphersuites.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_cookie.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_internal.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_ticket.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/threading.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/timing.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/version.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_crl.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_crt.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_csr.h"
    "/Users/Lucas/Documents/Documents-Lucas_Moiseyev_MacBook_Pro/Dev/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/xtea.h"
    )
endif()

