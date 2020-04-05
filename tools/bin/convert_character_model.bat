@echo off
rem =======================================================
rem ÉhÉâÉbÉOÇ≈ì¸ÇÍÇΩÇ‚Ç¬ÇÇ‹Ç∆ÇﬂÇƒèàóù
rem =======================================================

call %~dp0project_def.bat

set BLENDER="E:\\installer\\blender-2.76b-windows64\\blender.exe"
set SRC_FILE_OR_DIR=%*
rem set DEST_FBX=%~dp1out\
set DEST_FBX=%ASSET_CHARACTER_DIR%\_out
rem set WORK_BLEND=%~dp1temp\%~n1.blend
set WORK_DIR=%ASSET_CHARACTER_DIR%\_temp
set BASE_BLEND=%~dp0ply2fbx_base_character.blend
set TEX_SIZE=256


rem python %~dp0ply2fbx_recursive.py %SRC_FILE_OR_DIR% --destDir %DEST_FBX% --workDir %WORK_DIR% --texSize %TEX_SIZE%
echo %UE4_DIR%\UE4Editor-Cmd.exe
echo %UE4_PROJECT_DIR%\bishrpg.uproject
cd %UE4_DIR%
UE4Editor-Cmd.exe "%UE4_PROJECT_DIR%\bishrpg.uproject" -run=CharacterModelImporter -param0=hogehoge -stdout -UTF8Output

pause
