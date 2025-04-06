# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/espressif/esp/v5.0.2/esp-idf/components/bootloader/subproject"
  "D:/finalna/4_fresh_start_squareline/build/bootloader"
  "D:/finalna/4_fresh_start_squareline/build/bootloader-prefix"
  "D:/finalna/4_fresh_start_squareline/build/bootloader-prefix/tmp"
  "D:/finalna/4_fresh_start_squareline/build/bootloader-prefix/src/bootloader-stamp"
  "D:/finalna/4_fresh_start_squareline/build/bootloader-prefix/src"
  "D:/finalna/4_fresh_start_squareline/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/finalna/4_fresh_start_squareline/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/finalna/4_fresh_start_squareline/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
