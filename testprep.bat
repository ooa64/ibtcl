@echo off

set Script=%CD%\testprep.sql
set Firebird=D:\Firebird
set FirebirdData=D:\FirebirdData
set path=%Firebird%\bin;%path%

pushd %FirebirdData%

:taskkill /im fbserver.exe

chcp 1251
del %FirebirdData%\ibtcltest.fdb
del %FirebirdData%\ibtcl����.fdb
chcp 866

:start "fbserver" %Firebird%\bin\fbserver.exe -a
:pause

echo user ibtcltest/test
gsec -user SYSDBA -password masterkey -delete ibtcltest
gsec -user SYSDBA -password masterkey -add ibtcltest -pw test

echo user ibtcltestcyr/���
gsec -user SYSDBA -password masterkey -delete ibtcltestcyr
gsec -user SYSDBA -password masterkey -add ibtcltestcyr -pw ���

:echo user ibtcl���
:gsec -user SYSDBA -password masterkey -delete ibtcl���
:gsec -user SYSDBA -password masterkey -add ibtclt��� -pw test

echo script %Script%
isql -q -u SYSDBA -p masterkey -ch utf-8 -i %Script%

chcp 1251
echo script ibtcl����.fdb
echo create database "ibtcl����.fdb"; exit; | isql -q -u SYSDBA -p masterkey -ch WIN1251
chcp 866

:taskkill /im fbserver.exe

popd
