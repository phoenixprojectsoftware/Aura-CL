Aura Client
======================
[![CI](https://github.com/phoenixprojectsoftware/Aura-CL/actions/workflows/CI.yml/badge.svg?branch=master&event=push)](https://github.com/phoenixprojectsoftware/Aura-CL/actions/workflows/CI.yml)
[![Build Status](https://travis-ci.com/phoenixprojectsoftware/Aura-CL.svg?branch=master)](https://travis-ci.com/phoenixprojectsoftware/Aura-CL)

**Aura** is the all new multiplayer game engine from The Phoenix Project Software, and a modification of Adrenaline Gamer, built specifically for *Half-Life: Zombies Ate My Neighbours.*

# Building
## Windows
### Visual Studio 2019
1. Install [Visual Studio 2019](https://my.visualstudio.com/Downloads?q=Visual%20Studio%20Community%202019). In the Visual Studio Installer, select Desktop Development for C++.
1. Open Visual Studio.
1. On the starting screen, click "Clone or check out code".
1. Enter `https://github.com/phoenixsoftware/Aura-CL.git` and press the Clone button. Wait for the process to finish.
1. You can build the project using Build→Build All. To find the built client.dll, go to Project→CMake Cache (x86-Debug Only)→Open in Explorer.

### Visual Studio 2017
1. Install [Visual Studio 2017](https://my.visualstudio.com/Downloads?q=Visual%20Studio%20Community%202017). In the Visual Studio Installer, select Desktop Development for C++.
1. Open Visual Studio.
1. Go to File→Open→Open from Source Control.
1. Click Clone:

    ![Screenshot of the clone button.](https://user-images.githubusercontent.com/1794388/77243489-8e90ea00-6c1b-11ea-8000-09d576266162.png)

    Enter `https://github.com/phoenixsoftware/Aura-CL.git` and press the Clone button. Wait for the process to finish.
1. You can build the project using CMake→Build All. To find the built client.dll, go to CMake→Cache (x86-Debug Only)→Open Cache Folder→OpenAG.

### Visual Studio 2022 Preview
1. Install the [Visual Studio 2022 preview.](https://visualstudio.microsoft.com/vs/preview/vs2022/#download-preview). In the Visual Studio Installer, select **Desktop Development for C++.**
2. Open Visual Studio.
3. On the starting screen, click "Clone or check out code."
4. Enter `https://github.com/YaLTeR/OpenAG.git` and press the Clone button. Wait for the process to finish.
5. You can build the project using Build→Build All. To find the built client.dll, go to Project→CMake Cache (x86-Debug Only)→Open in Explorer.

### Manually using Git and CMake
1. Install Visual Studio 2017 or above, Git and CMake. Make sure to add them to PATH.
1. Clone the repository.
1. Open Git Bash in the OpenAG folder.
1. `git submodule update --init`
1. `cmake -A Win32 -B build`
1. `cmake --build build --config Release`

## macOS
1. Install Xcode.
2. Install CMake via Homebrew.
3. `git submodule update --init`
4. `mkdir build`
5. `cd build`
6. `cmake ..`
7. `cmake --build . --config Release`

## Linux
### GNOME Builder
1. Set up Flathub by following the [guide](https://flatpak.org/setup/) for your distribution.
1. Install [GNOME Builder](https://flathub.org/apps/details/org.gnome.Builder).
1. Open GNOME Builder.
1. Press the Clone Repository button, enter `https://github.com/phoenixsoftware/Aura-CL.git` and press Clone Project. Wait until it finishes.

   The cloning window should close, and a new window with the OpenAG project should open.
1. If Builder prompts you to install missing SDKs, press Install and wait for the process to finish. It will take a while. You can monitor the progress by pressing the circle in the top-right.
1. Click on the bar at the top-center which says OpenAG, and click the Build button.
1. Once the build finishes, in the same bar menu click the Export Bundle button. The file manager will open in a path that looks like `gnome-builder/projects/OpenAG/flatpak/staging/x86_64-master`. Navigate up to the `OpenAG` folder, then down to `builds/pro.openag.OpenAG.json-...` where you will find the built `client.so`.
1. Now you can make some changes to the code and press Build, then grab `client.so` from the same folder.

### Manually via Terminal
1. Get a 32-bit/multilib **gcc** (6 and above) or **clang** (3.9 and above) build environment set up, as well as CMake.
2. `git submodule update --init`
3. `mkdir build`
4. `cd build`
5. `cmake ..`
6. `cmake --build . --config Release`

### WSL 2 (Windows 10 & 11)
Side-note: if you're using WSL, it's recommended you use Ubuntu 18.04 from the Microsoft Store.

Open the Windows Terminal and select the Linux distro's profile (e.g. **Ubuntu 18.04**). Be aware that if you haven't already, you will need to add the `ninja` compiler to the PATH of your distro. It is included with this repository.

1. `sudo apt install gcc`
2. `sudo apt install clang`
3. `sudo apt install cmake`
4. `sudo apt install git`
5. Assuming you have `cd`'d to your Aura-CL folder; `git submodule update --init`
6. `mkdir Linux-Build`
7. `cmake ..` — wait for the process to finish
8. `cmake --build . --config Release`


# LICENSE
```
Half Life 1 SDK LICENSE
======================

Half Life 1 SDK Copyright© Valve Corp.  

THIS DOCUMENT DESCRIBES A CONTRACT BETWEEN YOU AND VALVE CORPORATION (“Valve”).  PLEASE READ IT BEFORE DOWNLOADING OR USING THE HALF LIFE 1 SDK (“SDK”). BY DOWNLOADING AND/OR USING THE SOURCE ENGINE SDK YOU ACCEPT THIS LICENSE. IF YOU DO NOT AGREE TO THE TERMS OF THIS LICENSE PLEASE DON’T DOWNLOAD OR USE THE SDK.

You may, free of charge, download and use the SDK to develop a modified Valve game running on the Half-Life engine.  You may distribute your modified Valve game in source and object code form, but only for free. Terms of use for Valve games are found in the Steam Subscriber Agreement located here: http://store.steampowered.com/subscriber_agreement/ 

You may copy, modify, and distribute the SDK and any modifications you make to the SDK in source and object code form, but only for free.  Any distribution of this SDK must include this license.txt and third_party_licenses.txt.  
 
Any distribution of the SDK or a substantial portion of the SDK must include the above copyright notice and the following: 

DISCLAIMER OF WARRANTIES.  THE SOURCE SDK AND ANY OTHER MATERIAL DOWNLOADED BY LICENSEE IS PROVIDED “AS IS”.  VALVE AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES WITH RESPECT TO THE SDK, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, TITLE AND FITNESS FOR A PARTICULAR PURPOSE.  

LIMITATION OF LIABILITY.  IN NO EVENT SHALL VALVE OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THE ENGINE AND/OR THE SDK, EVEN IF VALVE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  
 
 
If you would like to use the SDK for a commercial purpose, please contact Valve at sourceengine@valvesoftware.com.
```


