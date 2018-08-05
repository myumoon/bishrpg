@echo off
rem =======================================================
rem ドラッグで入れたやつをまとめて処理
rem =======================================================

set CMD=%~dp0\ply2fbx_character.bat

for %%I in (%*) do (
	echo %%~xI
	if "%%~xI" == ".ply" (
		echo %CMD% %%I
		rem startだとファイルのロックがかかってしまったのでcallで呼ぶ
		call %CMD% %%I
	)
)

:pause
