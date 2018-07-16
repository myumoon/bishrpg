@echo off
rem =======================================================
rem static�ȃI�u�W�F�N�g��ply��fbx��
rem �����̃I�u�W�F�N�g�����邱�Ƃ��ł���
rem �t�H���_�w�肷�邱�ƂŃt�H���_���ł܂Ƃ߂�
rem 
rem blender�̃p�X��ʂ��Ă�������
rem =======================================================

set BLENDER="blender.exe"
set PLY2FBX=%~dp0\ply2fbx_static_atlas.py
set SRC_PLY_LIST=%*
set BASE_BLEND=%~dp0ply2fbx_base_static.blend

echo src: %SRC_PLY_LIST:\\=/%

if not "%SRC_PLY_LIST%" == "" (
	%BLENDER% %BASE_BLEND:\\=/% -b -P %PLY2FBX% -- ""  --static_x10 %SRC_PLY_LIST%
)

pause
