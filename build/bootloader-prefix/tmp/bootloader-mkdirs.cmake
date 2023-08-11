# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/ronald/esp/esp-idf/components/bootloader/subproject"
  "/home/ronald/Desktop/Task/build/bootloader"
  "/home/ronald/Desktop/Task/build/bootloader-prefix"
  "/home/ronald/Desktop/Task/build/bootloader-prefix/tmp"
  "/home/ronald/Desktop/Task/build/bootloader-prefix/src/bootloader-stamp"
  "/home/ronald/Desktop/Task/build/bootloader-prefix/src"
  "/home/ronald/Desktop/Task/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/ronald/Desktop/Task/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/ronald/Desktop/Task/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
