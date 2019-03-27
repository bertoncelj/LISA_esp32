@echo off

del data_gz\* /F /Q
cd data_rus

rem gzip source files and move them to data_gz folder

FOR %%i IN (*.html,*.htm,*.css,*.jpg,*.ico,*.png) DO (
	"%ProgramFiles%\7-Zip\7z.exe" a "%%~i.gz" "%%i"
	move "%%~i.gz" ../data_gz
)

copy "version.txt" "../data_gz"


pause