@echo off
set BUILD_TYPE=Debug
set CPP_STD=17
set OUTPUT_FOLDER=build

echo Running Conan install...
conan install . --output-folder=%OUTPUT_FOLDER% --build=missing -s build_type=%BUILD_TYPE% -s compiler.cppstd=%CPP_STD%

echo Conan install completed!
pause