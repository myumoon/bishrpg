@echo off
rem =======================================================
rem プロジェクト情報
rem =======================================================

set UE4_DIR="D:\Program Files\Epic Games\UE_4.24\Engine\Binaries\Win64"

set PROJECT_ROOT_DIR=%~dp0\..\..\
set UE4_PROJECT_DIR=%PROJECT_ROOT_DIR%\bishrpg\

set ASSET_DIR=D:\prog\0_myprogram\bishrpg_resources

set ASSET_MODEL_DIR=%ASSET_DIR%\models
set ASSET_CHARACTER_DIR=%ASSET_MODEL_DIR%\characters

set ASSET_OBJECT_DIR=%ASSET_MODEL_DIR%\objects

exit /b
