@echo off
SetLocal EnableDelayedExpansion
(set PATH=C:\Qt\5.11.1\mingw-w64\qtbase\bin;!PATH!)
if defined QT_PLUGIN_PATH (
    set QT_PLUGIN_PATH=C:\Qt\5.11.1\mingw-w64\qtbase\plugins;!QT_PLUGIN_PATH!
) else (
    set QT_PLUGIN_PATH=C:\Qt\5.11.1\mingw-w64\qtbase\plugins
)
C:\Qt\5.11.1\mingw-w64\qtbase\bin\uic.exe %*
EndLocal
