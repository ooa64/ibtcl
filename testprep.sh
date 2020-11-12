
Script=$(pwd)/testprep.sql
Firebird=/opt/firebird
FirebirdData=/tmp
export PATH=$Firebird/bin:$PATH
export LD_LIBRARY_PATH=$Firebird/lib

sudo -u firebird $Firebird/bin/fbserver -z
sudo -u firebird killall fbserver

pushd $FirebirdData

sudo -u firebird rm ibtcltest.fdb
sudo -u firebird rm ibtclтест.fdb

sudo -u firebird $Firebird/bin/fbserver -a &
sleep 1

gsec -user SYSDBA -password masterkey -delete ibtcltest
gsec -user SYSDBA -password masterkey -add ibtcltest -pw test

gsec -user SYSDBA -password masterkey -delete ibtcltestcyr
gsec -user SYSDBA -password masterkey -add ibtcltestcyr -pw тест

gsec -user SYSDBA -password masterkey -delete ibtclтест
gsec -user SYSDBA -password masterkey -add ibtcltтест -pw тест

isql -q -u SYSDBA -p masterkey -ch win1251 -i $Script 

echo script ibtclтест.fdb
echo "create database \"ibtclтест.fdb\"; exit;" | isql -q -u SYSDBA -p masterkey -ch WIN1251

popd



