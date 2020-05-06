@echo off
rem =======================================================
rem ƒ‚ƒfƒ‹ƒrƒ‹ƒh
rem =======================================================

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

%~dp0../../thirdparty/bin/ninja.exe -C %~dp0ninja -v %*

rem UE4Editor-Cmd.exe "%UE4_PROJECT_DIR%\bishrpg.uproject" -run=CharacterModelImporter -csv=%IMPORT_FILE_LIST% -stdout -UTF8Output

rem pause
