@echo off

set SCRIPT=%CD%\testprep.sql 
set TCLROOT=C:\P\tcl-32-8.6-m
set IBROOT=D:\Firebird
set IBDATA=D:\FirebirdData
set PATH=%TCLROOT%\bin;%IBROOT%\bin;%PATH%

pushd %IBDATA%

:taskkill /im fbserver.exe

chcp 1251
del ibtcltest.fdb
del ibtclòåñò.fdb
chcp 866

:start "fbserver" %Firebird%\bin\fbserver.exe -a
:pause

echo user ibtcltest/test
gsec -user SYSDBA -password masterkey -delete ibtcltest
gsec -user SYSDBA -password masterkey -add ibtcltest -pw test

echo user ibtcltestcyr/â¥áâ
gsec -user SYSDBA -password masterkey -delete ibtcltestcyr
gsec -user SYSDBA -password masterkey -add ibtcltestcyr -pw â¥áâ

:echo user ibtclâ¥áâ
:gsec -user SYSDBA -password masterkey -delete ibtclâ¥áâ
:gsec -user SYSDBA -password masterkey -add ibtcltâ¥áâ -pw test

echo script %Script%
isql -q -u SYSDBA -p masterkey -ch utf-8 -i %Script%

chcp 1251
echo script ibtclòåñò.fdb
echo create database "ibtclòåñò.fdb"; exit; | isql -q -u SYSDBA -p masterkey -ch WIN1251
chcp 866

:taskkill /im fbserver.exe

popd
