@echo off
rem =======================================================
rem キャラを変換
rem データの種類ごとにfbxを出力
rem 髪の毛は上にある想定
rem 
rem blenderのパスを通してください
rem =======================================================

set BLENDER="blender.exe"
set PLY2FBX=%~dp0\ply2fbx.py
set SRC_PLY=%~1
set DEST_FBX=%~dp1%~n1.fbx
set WORK_BLEND=%~dp1%~n1.blend
set BASE_BLEND=%~dp0ply2fbx_base_character.blend

echo src: %SRC_PLY:\\=/%
echo dest: %DEST_FBX:\\=/%
echo work: %WORK_BLEND:\\=/%

if not "%SRC_PLY%" == "" (
	%BLENDER% %BASE_BLEND:\\=/% -b -P %PLY2FBX% -- "" %SRC_PLY% %DEST_FBX% %WORK_BLEND% --hair_origin
)

:pause
