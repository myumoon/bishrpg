@echo off
rem =======================================================
rem staticなオブジェクトをply→fbx化
rem 複数のオブジェクトを入れることができる
rem フォルダ指定することでフォルダ名でまとめる
rem 
rem blenderのパスを通してください
rem =======================================================

set BLENDER="blender.exe"
set PLY2FBX=%~dp0\ply2fbx_static_atlas.py
set SRC_PLY_LIST=%*
set BASE_BLEND=%~dp0ply2fbx_base_static.blend

echo src: %SRC_PLY_LIST:\\=/%

if not "%SRC_PLY_LIST%" == "" (
	%BLENDER% %BASE_BLEND:\\=/% -b -P %PLY2FBX% -- ""  --static_x10 %SRC_PLY_LIST%
)

:pause
