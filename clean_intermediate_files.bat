:: Copyright LifeEXE. All Rights Reserved.
@echo off

set dirsToRemove=Intermediate Binaries

FOR %%f in (%dirsToRemove%) do (
    rmdir /s /q %%f
)