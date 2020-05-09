@echo off
rem =======================================================
rem モデルビルド
rem =======================================================

call %~dp0..\worktime.bat START

for /f "usebackq" %%A in (`time /t`) do set CURRENT_TIME=%%A
echo start time : %CURRENT_TIME%

call %~dp0..\project_def.bat

set NINJA_FILE_DIR=%~dp0ninja
set NINJA_CONFIG_DEST=%NINJA_FILE_DIR%\config.ninja
set NINJA_RULE_DEST=%NINJA_FILE_DIR%\rule.ninja
set NINJA_BUID_DEST=%NINJA_FILE_DIR%\build.ninja

python %~dp0scripts/ninja_model_config_writer.py --out %NINJA_CONFIG_DEST%
python %~dp0scripts/ninja_model_rule_writer.py --out %NINJA_RULE_DEST%
python %~dp0scripts/ninja_model_build_writer.py --out %NINJA_BUID_DEST%

rem ninja buildを実行
rem   -jはスレッド数、スレッド数を増やすと　bpy.ops.object.parent_set(type='ARMATURE_AUTO') の
rem   処理時間が1モデル当たり5分くらいかかるようになったのでスレッド数を1に設定している。
%~dp0../../thirdparty/bin/ninja.exe -C %~dp0ninja -j 1 -v %*

call %~dp0..\worktime.bat STOP

for /f "usebackq" %%A in (`time /t`) do set CURRENT_TIME=%%A
echo All done. current=%CURRENT_TIME% elapsed=%DPS_STAMP%


rem pause
