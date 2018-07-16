@echo off
rem =======================================================
rem staticなplyオブジェクトをfbxに変換する
rem スケーリングをx10する
rem 同時に複数インポートされた場合は同じアトラスにする
rem 
rem blenderをPATHに追加すること
rem =======================================================

set BLENDER="blender.exe"
set PLY2FBX=%~dp0\ply2fbx_static_atlas.py
set SRC_PLY_LIST=%*
set BASE_BLEND=%~dp0ply2fbx_base_static.blend
set OUT_DIRNAME=out
set TEMP_DIRNAME=temp

echo src: %SRC_PLY_LIST:\\=/%

if not "%SRC_PLY_LIST%" == "" (
	%BLENDER% %BASE_BLEND:\\=/% -b -P %PLY2FBX% -- ""  --static_x10 %OUT_DIRNAME% %TEMP_DIRNAME% %SRC_PLY_LIST%
)

pause
