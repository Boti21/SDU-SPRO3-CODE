# How to build the project

There is a few things that is required for get the project to build. The below
this is for Visual studio code, if you want the manual installation consult the
ESP-IDF official website.

1. Install the ESP-IDF extension in VS-Code
   1. Choose express installation and keep the option to default.
   2. For ESP-IDF version choose the news available
   3. The installation should take 3-5 min
2. Clone down the code repository
   1. Login with your github account in vs-code
   2. Open the command line by pressing ```Ctrl+Shift+P```
   3. Write ```git:clone```
   4. Select github and select the SDU-SPRO3-CODE repository
   5. A file explore window should popup, choose your desired location of the
      local repository. 
3. Open the repository in VS-CODE
   1. Go to File->Open folder or ```Ctrl+K```
   2. Find the repository folder SDU-SPRO3-CODE
   3. Inside the SDU-SPRO3-CODE folder select the SPOR3-Firmware folder
   4. To make sure it is the right folder, it should contain the file
      ```CMakeLists.txt```. If not then the wrong folder was selected.
   5. It can be that ```CMakeLists.txt``` is hidden in the VS-CODE explore, if
      this the case use you OS explore.