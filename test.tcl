package require tcltest
namespace import tcltest::*

puts system:[encoding system]
puts stdout:[fconfigure stdout -encoding]
puts test:тест

set ibtclDbh ""
set ibtclLib $env(Ibtcldll)
set fbserverConn [file join $env(FirebirdData) ibtcltest.fdb]
set fbserverConnCyr [file join $env(FirebirdData) ibtclтест.fdb]
set isqlName [auto_execok isql]

if {$tcl_platform(platform) eq "windows"} {
    set ibtclEnc cp1251
} else {
    set ibtclEnc utf-8
}

testConstraint isql [expr {$isqlName ne ""}]

test 0.1 "load library" {
    load $ibtclLib
    testConstraint ibtcl01 [expr {[package require ibtcl] eq "0.1"}]
    testConstraint ibtcl 1
} {1}

test 1.0 "connect as sysdba" {
    set ibtclDbh [ib_open $fbserverConn SYSDBA masterkey {} $ibtclEnc]
    testConstraint fbconnected 1
} {1}

test 1.1 "connect nonexisting user" -constraints fbconnected -body {
    ib_close [ib_open $fbserverConn ibtclnotexists {}]
} -match glob -result {Your user name and password are not defined.*} -returnCodes 1

test 1.2 "connect existing user" fbconnected {
    ib_close [ib_open $fbserverConn ibtcltest test]
} {}

test 1.3.1 "connect user with nonascii password (hack)" {fbconnected ibtcl01} {
    ib_close [ib_open $fbserverConn ibtcltestcyr [encoding convertfrom identity [encoding convertto $ibtclEnc тест]] {} $ibtclEnc]
} {}

test 1.3.2 "connect user with nonascii password" {fbconnected} {
    ib_close [ib_open $fbserverConn ibtcltestcyr тест {} $ibtclEnc]
} {}

test 1.4 "connect nonascii user with nonascii password" {fbconnected BUG} {
    ib_close [ib_open $fbserverConn ibtclтест тест {} $ibtclEnc]
} {}

test 1.5.0 "check nonascii db file" {
   file exists $fbserverConnCyr
} {1}

test 1.5.1 "connect nonascii db file (hack)" {fbconnected ibtcl01} {
   ib_close [ib_open [encoding convertfrom identity [encoding convertto $ibtclEnc $fbserverConnCyr]] ibtcltest test]
} {}

test 1.5.2 "connect nonascii db file" fbconnected {
   ib_close [ib_open $fbserverConnCyr ibtcltest test]
} {}

test 1.6 "select nonascii table" -constraints {fbconnected BUG} -setup {
    set d [ib_open $fbserverConnCyr ibtcltest test]
    set s [ib_exec $d "select * from \"тест\""]
} -body {
    list [ib_fetch -n 3 $s a] [array get a]
} -cleanup {
    catch {ib_close $d}
    catch {ib_free_stmt $s}
    unset -nocomplain s
} -match glob -result {{} {0,0 1 cols 2 1,0 2 0,1 один 2,0 3 1,1 два rows 3 2,1 три}}

test 2.1 "simple query" -constraints fbconnected -setup {
    set s [ib_exec $ibtclDbh "select 'X' X from rdb\$database"]
} -body {
    list [ib_isquery $s] [ib_fields $s] [ib_fieldname $s 0] [ib_fetch -n 1 $s a] [array get a]
} -cleanup {
    catch {ib_free_stmt $s}
    unset -nocomplain s
} -match glob -result {1 1 X {} {0,0 X cols 1 rows 1}}

test 2.2 "simple query timestamp" -constraints fbconnected -setup {
    set s [ib_exec $ibtclDbh "select current_timestamp from rdb\$database"]
} -body {
    list [ib_fetch -n 1 $s a] [array get a]
} -cleanup {
    catch {ib_free_stmt $s}
    unset -nocomplain s
} -match glob -result {{} {0,0 {20??-??-?? ??:??:??} cols 1 rows 1}}

test 2.3 "simple query text" -constraints fbconnected -setup {
    set s [ib_exec $ibtclDbh "select * from test"]
} -body {
    list [ib_fetch -n 3 $s a] [array get a]
} -cleanup {
    catch {ib_free_stmt $s}
    unset -nocomplain s
} -match glob -result {{} {0,0 1 cols 2 1,0 2 0,1 one 2,0 3 1,1 two rows 3 2,1 three}}

test 2.4 "simple query nonacii text" -constraints fbconnected -setup {
    set s [ib_exec $ibtclDbh "select * from testcyr"]
} -body {
    list [ib_fetch -n 3 $s a] [array get a]
} -cleanup {
    catch {ib_free_stmt $s}
    unset -nocomplain s
} -match glob -result {{} {0,0 1 cols 2 1,0 2 0,1 один 2,0 3 1,1 два rows 3 2,1 три}}

test 9.99 "disconnect" fbcreated {
    ib_close $ibtclDbh
} {}

cleanupTests
