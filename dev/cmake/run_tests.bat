setlocal
set PATH=%PATH%;${CMAKE_INSTALL_PREFIX}\bin
cmd /c ctest --force-new-ctest-process
if %errorlevel% neq 0 exit /b 1
endlocal
