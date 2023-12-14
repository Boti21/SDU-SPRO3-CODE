# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/f-bon/esp/esp-idf/components/bootloader/subproject"
  "D:/AAStudium/Study/Semester_3/MechatronicsSemesterProject3/CodePrototyping/softAP/build/bootloader"
  "D:/AAStudium/Study/Semester_3/MechatronicsSemesterProject3/CodePrototyping/softAP/build/bootloader-prefix"
  "D:/AAStudium/Study/Semester_3/MechatronicsSemesterProject3/CodePrototyping/softAP/build/bootloader-prefix/tmp"
  "D:/AAStudium/Study/Semester_3/MechatronicsSemesterProject3/CodePrototyping/softAP/build/bootloader-prefix/src/bootloader-stamp"
  "D:/AAStudium/Study/Semester_3/MechatronicsSemesterProject3/CodePrototyping/softAP/build/bootloader-prefix/src"
  "D:/AAStudium/Study/Semester_3/MechatronicsSemesterProject3/CodePrototyping/softAP/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/AAStudium/Study/Semester_3/MechatronicsSemesterProject3/CodePrototyping/softAP/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/AAStudium/Study/Semester_3/MechatronicsSemesterProject3/CodePrototyping/softAP/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
