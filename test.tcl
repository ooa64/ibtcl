#!/usr/bin/env tclsh -encoding utf-8 test.tcl
encoding system utf-8

package require tcltest
namespace import tcltest::*

puts system:[encoding system]
puts stdout:[fconfigure stdout -encoding]
puts test:тест

set ibtclDbh ""
set ibtclLib $env(Ibtcldll)
set ibtclEnc cp1251
set fbserverConn [file join $env(FirebirdData) ibtcltest.fdb]
set fbserverConnCyr [file join $env(FirebirdData) ibtclтест.fdb]
set isqlName [auto_execok isql]

testConstraint isql [expr {$isqlName ne ""}]
testConstraint memory [expr {[testConstraint memory] && [info commands memory] eq "memory"}]

proc a2r {avar} {
    upvar $avar a
    set t {}
    for {set r 0} {$r < $a(rows)} {incr r} {
        set l {}
        for {set c 0} {$c < $a(cols)} {incr c} {
            lappend l $a($r,$c)
        }
	lappend t $l
    }
    return $t
}

test 0.1 "memory debug" memory {
    memory init on
    memory validate on
#   memory trace on
} {}

test 0.2 "load library" {
    load $ibtclLib
    testConstraint ibtcl01 [expr {[package require ibtcl] eq "0.1"}]
    testConstraint ibdebug [expr {[info commands ib_test] eq "ib_test"}]
    testConstraint ibtcl 1
} {1}

test 1.0 "connect as sysdba" {
    set ibtclDbh [ib_open $fbserverConn SYSDBA masterkey {} $ibtclEnc WIN1251]
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
#   ib_close [ib_open $fbserverConn ibtcltestcyr тест {} $ibtclEnc]
    ib_close [ib_open $fbserverConn ibtcltestcyr тест]
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
    list [ib_fetch -n 3 $s a] [a2r a]
} -cleanup {
    catch {ib_close $d}
    catch {ib_free_stmt $s}
    unset -nocomplain s
} -match glob -result {{} {0,0 1 cols 2 1,0 2 0,1 один 2,0 3 1,1 два rows 3 2,1 три}}

test 2.0 "simple query error" -constraints fbconnected -body {
    set s [ib_exec $ibtclDbh "xxx"]
} -cleanup {
    catch {ib_free_stmt $s}
    unset -nocomplain s
} -match glob -result {Dynamic SQL Error*Token unknown*} -returnCodes 1

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
    list [ib_fetch -n 3 $s a] [a2r a]
} -cleanup {
    catch {ib_free_stmt $s}
    unset -nocomplain s
} -result {{} {{1 one} {2 two} {3 three}}}

test 2.4 "simple query nonacii text" -constraints fbconnected -setup {
    set s [ib_exec $ibtclDbh "select * from testcyr"]
} -body {
    list [ib_fetch -n 3 $s a] [a2r a]
} -cleanup {
    catch {ib_free_stmt $s}
    unset -nocomplain s
} -result {{} {{1 один} {2 два} {3 три}}}

test 2.5 "simple query all types" -constraints fbconnected -setup {
    set s [ib_exec $ibtclDbh "select * from testtypes"]
} -body {
    list [ib_fetch -n 1 $s a] [a2r a]
} -cleanup {
    catch {ib_free_stmt $s}
    unset -nocomplain s
} -result {{} {{1 2 3 4 5 6.66 7 8 9.99 10.100000 20.200000 2001-01-31 12:34:56 {2016-12-25 12:34:56} c {c1        } h {h1        } v}}}

test 2.6 "simple query all types min values" -constraints fbconnected -setup {
    set s [ib_exec $ibtclDbh "select * from testtypes order by i"]
} -body {
    list [ib_fetch -n 1 $s a] [a2r a]
} -cleanup {
    catch {ib_free_stmt $s}
    unset -nocomplain s
} -match glob -result {{} {{-2147483648 -32768 -9223372036854775808 -32768 -32768 -327.68 -99999999 -999999999 -9999999.99 -9999.99* -9999999999999.99* 0001-01-01 00:00:00 {2001-01-01 00:00:00} { } {          } { } {          } {}}}}

test 2.7 "simple query all types max values" -constraints fbconnected -setup {
    set s [ib_exec $ibtclDbh "select * from testtypes order by i desc"]
} -body {
    list [ib_fetch -n 1 $s a] [a2r a]
} -cleanup {
    catch {ib_free_stmt $s}
    unset -nocomplain s
} -match glob -result {{} {{2147483647 32767 9223372036854775807 32767 32767 327.67 99999999 999999999 9999999.99 9999.99* 9999999999999.99* 0001-01-01 00:00:00 {2001-01-01 00:00:00} X XXXXXXXXXX x xxxxxxxxxx ЙЙЙЙЙЙЙЙЙЙ}}}

test 9.99 "disconnect" fbconnected {
    ib_close $ibtclDbh
} {}

cleanupTests
