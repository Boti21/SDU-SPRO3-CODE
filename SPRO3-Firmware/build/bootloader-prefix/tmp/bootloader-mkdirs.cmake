# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/botib/esp/esp-idf/components/bootloader/subproject"
  "C:/Users/botib/SDU/SemesterProjects/SDU-SPRO3/SDU-SPRO3-CODE/SPRO3-Firmware/build/bootloader"
  "C:/Users/botib/SDU/SemesterProjects/SDU-SPRO3/SDU-SPRO3-CODE/SPRO3-Firmware/build/bootloader-prefix"
  "C:/Users/botib/SDU/SemesterProjects/SDU-SPRO3/SDU-SPRO3-CODE/SPRO3-Firmware/build/bootloader-prefix/tmp"
  "C:/Users/botib/SDU/SemesterProjects/SDU-SPRO3/SDU-SPRO3-CODE/SPRO3-Firmware/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/botib/SDU/SemesterProjects/SDU-SPRO3/SDU-SPRO3-CODE/SPRO3-Firmware/build/bootloader-prefix/src"
  "C:/Users/botib/SDU/SemesterProjects/SDU-SPRO3/SDU-SPRO3-CODE/SPRO3-Firmware/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/botib/SDU/SemesterProjects/SDU-SPRO3/SDU-SPRO3-CODE/SPRO3-Firmware/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/botib/SDU/SemesterProjects/SDU-SPRO3/SDU-SPRO3-CODE/SPRO3-Firmware/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
