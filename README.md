# BiSH RPG

BiSHを題材にしたRPGゲー

----

## Overview

無茶ぶりプロデューサーの指令により世界を救うことになったアイドルグループ。  
果たして荒廃した世界で仲間を救うことはできるのか！？  
推しを応援するバトルシステムで敵と戦え！
  
アイドルが世界を救うRPGが今ここに。

## Platform

- iOS(TBE:version)
- Android(TBE:version)

----

## Setup

### Engine

- UnrealEngine4
  - Install version 4.20.3

### Version Control System for Assets

- SVN
  - Install Tortoise-1.13.1  
    https://tortoisesvn.net/downloads.html
  - Checkout https://myumoon.assembla.com/spaces/game/subversion/source/HEAD/trunk 
    Trunk URL is ```https://subversion.assembla.com/svn/myumoon^game/trunk```.

### DCC Tools

- Magica Voxel
  - Install any version.

- Blender
  - Install version blender-2.79b-windows64.  
    **Other version doesn't work.**  
    ```bishrpg/tools/bin/*.bat``` dependences the version.
  - Add "UV: Texture Atlas":
    File > UserPreferences > Add-ons > Check "UV: Texture Atlas"

## Modeling

- MagicaVoxel
  1. Create a model.
  1. Export the model to .ply

## Convert

- Charactes
  1. Drag and drop a model folder to ```bishrpg/tools/bin/ply2fbx_character.bat```.

- Objects
  1. Drag and drop a model folder including *.ply to ```bishrpg/tools/bin/ply2fbx_static_x1.bat```.  
     You can make x10 model if you use ~x10.bat.

## Required assets

The project depend on the following assets:

- POLYGON MINI - Fantasy Pack  
  https://www.unrealengine.com/marketplace/ja/product/polygon-mini-fantasy-pack

- Fog Gradients  
  https://www.unrealengine.com/marketplace/ja/product/fog-gradients

## Lisence

(TBE)

## Acknowledgments

- M+ BITMAP FONTS  
  http://mplus-fonts.osdn.jp/mplus-bitmap-fonts/