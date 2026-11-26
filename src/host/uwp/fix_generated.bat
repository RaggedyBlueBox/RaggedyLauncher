@echo off
set FILE=Generated Files\module.g.cpp
if exist %FILE% (
    powershell -Command "(Get-Content '%FILE%') -replace '#include \"winrt/base.h\"', '#include <unknwn.h>`n#include \"winrt/base.h\"' | Set-Content '%FILE%'"
)
