<!--
 Copyright (C) 2023 wwhai

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
-->

# 一个比较简单的Modbus地址扫描仪
主要用来扫描总线上能用的设备，比较简单。现阶段只有windows版本。
## 编译
```sh
gcc scanner-win32.c -o scanner-win32.exe
```
## 使用教程
直接powershell执行:`scanner-win32.exe [端口号] 波特率`
```sh
scanner-win32.exe COM2 9600
```
