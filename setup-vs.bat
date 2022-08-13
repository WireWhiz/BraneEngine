@echo off
echo Creating Visual Studio Project...
where /q git
IF ERRORLEVEL 1 (
    ECHO Git not found, make sure it is installed and added to your path.
    EXIT /B
)
echo Updating Git Submodules...
git submodule update --init
where /q cmake
IF ERRORLEVEL 1 (
    ECHO Cmake not found, make sure it is installed and added to your path.
    EXIT /B
)

set build_testing=-DBUILD_TESTING=OFF
set /p bt="Build Testing? (y/n defaults to no): "
if "%bt%" == "y" (
    set build_testing=-DBUILD_TESTING=ON
)
mkdir vs-proj
pushd vs-proj
echo on
cmake .. %build_testing%
@echo off
popd
