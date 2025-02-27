@echo off
rem =======================================================
rem ドラッグで入れたやつをまとめて処理
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

set IMPORT_DIRS="[D:\prog\0_myprogram\bishrpg_resources\models\characters\_out\Lower,D:\prog\0_myprogram\bishrpg_resources\models\characters\_out\Upper,D:\prog\0_myprogram\bishrpg_resources\models\characters\_out\Hair,D:\prog\0_myprogram\bishrpg_resources\models\characters\_out\Face,D:\prog\0_myprogram\bishrpg_resources\models\characters\_out\Accessory]"
set FBX_PATH="D:\prog\0_myprogram\bishrpg_resources\models\characters\_out\Lower\Meshes\Lower_p999_01.fbx"
set TEX_PATH="D:\prog\0_myprogram\bishrpg_resources\models\characters\_out\Lower\Textures\Lower_p999_01.png"
set PARTS_NAME="Lower"
set OUT_NAME="Lower_p999_02"

set IMPORT_FILE_LIST=%~dp0import_files.txt


python %~dp0model_build\ninja_model_build_config_writer.py

pause
