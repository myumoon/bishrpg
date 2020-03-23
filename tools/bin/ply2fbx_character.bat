@echo off
rem =======================================================
rem ?L????????
rem ?f?[?^????????fbx??o??
rem ????„„??????z??
rem 
rem blender??p?X?????????????
rem =======================================================

call project_def.bat

set BLENDER="blender.exe"
set PLY2FBX=%~dp0\ply2fbx.py
set SRC_PLY=%~1
rem set DEST_FBX=%~dp1out\
set DEST_FBX=%ASSET_CHARACTER_DIR%\_out\
rem set WORK_BLEND=%~dp1temp\%~n1.blend
set WORK_BLEND=%ASSET_CHARACTER_DIR%\_temp\
set BASE_BLEND=%~dp0ply2fbx_base_character.blend
set TEX_SIZE=256

echo src: %SRC_PLY:\\=/%
echo dest: %DEST_FBX:\\=/%
echo work: %WORK_BLEND:\\=/%

if not "%SRC_PLY%" == "" (
	%BLENDER% %BASE_BLEND:\\=/% -b -P %PLY2FBX% -- "" %SRC_PLY% %DEST_FBX% %WORK_BLEND% %TEX_SIZE%
)

:pause

exit /b 0
