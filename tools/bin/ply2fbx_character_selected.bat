@echo off
rem =======================================================
rem �h���b�O�œ��ꂽ����܂Ƃ߂ď���
rem =======================================================

set CMD=%~dp0\ply2fbx_character.bat

for %%I in (%*) do (
	echo %%~xI
	if "%%~xI" == ".ply" (
		echo %CMD% %%I
		rem start���ƃt�@�C���̃��b�N���������Ă��܂����̂�call�ŌĂ�
		call %CMD% %%I
	)
)

::pause
