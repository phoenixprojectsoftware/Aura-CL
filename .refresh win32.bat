@echo off
cls
title Generating Build Files
cmake -A Win32 -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5
echo Look above in case anything went wrong. Otherwise,
pause