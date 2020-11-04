catch {load ibtcl011tg.dll}
catch {load ibtcl011t.dll}
catch {load ibtcl011.dll}
catch {load ./ibtcl.so}

set dbn "localhost:c:/FireBird/Data/test.fdb"
set dbh [ib_open $dbn SYSDBA masterkey]
set stm [ib_exec $dbh {select current_timestamp from RDB$DATABASE}]

puts isquery=[ib_isquery $stm]
puts fields=[ib_fields $stm]
puts field0=[ib_fieldname $stm 0]
puts fetch=[ib_fetch -n 1 $stm a]
parray a

ib_free_stmt $stm
ib_close $dbh
