@echo off
echo:

rem ************************************************
rem *      Watcom C++ 11.0 PTC example build       *
rem * -------------------------------------------- *
rem * Syntax: wc11 [example] (platform) (library)  *
rem *                                              *
rem * [example]  = example source name             *
rem * (platform) = "dos" or "win32"                *
rem * (library)  = "debug" or "release"            *
rem *                                              *                                           
rem * NOTE: to compile the examples for the win32  *
rem * platform it is necessary to use the /binnt   *
rem * watcom binaries. this is because the dos     *
rem * binaries have a limit on the size of command *
rem * line arguments.                              *
rem ************************************************

if not exist "%1.cpp" goto done

if "%2" == "pmodew" goto pmodew
if "%2" == "dos"    goto dos
if "%2" == "win32"  goto win32




rem ****************
rem * Dos platform *
rem ****************
:pmodew
echo Building "%1.cpp" : Watcom C++ 11.0 dos PMODEW release
echo:
wpp386 %1.cpp -i=..\source -bt=pmodew -oneatxhl+ -5r -fp5 -fpi87
wlink name %1.exe file %1.obj library ..\library\dos\wc11.0\release.lib system pmodew option stack=256k
goto done


:dos
if "%3" == "debug" goto dos-debug

echo Building "%1.cpp" : Watcom C++ 11.0 dos release
echo:
wpp386 %1.cpp -i=..\source -bt=dos -oneatxhl+ -5r -fp5 -fpi87
wlink name %1.exe file %1.obj library ..\library\dos\wc11.0\release.lib system dos4g option stack=256k
goto done

:dos-debug
echo Building "%1.cpp" : Watcom C++ 11.0 dos debug
echo:
wpp386 %1.cpp -i=..\source -bt=dos
wlink name %1.exe file %1.obj library ..\library\dos\wc11.0\debug.lib system dos4g option stack=256k
goto done

:done
