@echo off
rem =======================================================
rem ドラッグで入れたやつをまとめて処理
rem =======================================================

call %~dp0project_def.bat

set BLENDER="blender.exe"
set SRC_FILE_OR_DIR=%*
rem set DEST_FBX=%~dp1out\
set DEST_FBX=%ASSET_CHARACTER_DIR%\_out
rem set WORK_BLEND=%~dp1temp\%~n1.blend
set WORK_DIR=%ASSET_CHARACTER_DIR%\_temp
set BASE_BLEND=%~dp0ply2fbx_base_character.blend
set TEX_SIZE=256


python %~dp0ply2fbx_recursive.py %SRC_FILE_OR_DIR% --destDir %DEST_FBX% --workDir %WORK_DIR% --texSize %TEX_SIZE%

rem set CMD=%~dp0\ply2fbx_character.bat

rem for %%I in (%*) do (
rem 	echo %%~xI
rem 	if "%%~xI" == ".ply" (
rem 		echo %CMD% %%I
rem 		rem startだとファイルのロックがかかってしまったのでcallで呼ぶ
rem 		call %CMD% %%I
rem 	)
rem )

rem pause
