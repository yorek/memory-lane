@echo off
echo:

rem ****************
rem * Dos platform *
rem ****************

echo Building "ff.cpp" : Watcom C++ 11.0 dos PMODEW release
echo:
wpp386 ff.cpp -i=..\source -bt=pmodew -oneatxhl+ -5r -fp5 -fpi87

echo:
echo Building "fract.cpp" : Watcom C++ 11.0 dos PMODEW release
echo:
wpp386 fract.cpp -i=..\source -bt=pmodew -oneatxhl+ -5r -fp5 -fpi87

echo:
echo Linking...
echo:
wlink name ff.exe file ff.obj file fract.obj library ..\library\dos\wc11.0\release.lib system pmodew option stack=256k
goto done

:done
