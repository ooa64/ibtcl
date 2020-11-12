SET NAMES UTF8;
set sql dialect 3;

--drop user ibtcltest;
--create user ibtcltest password 'test';

--drop user ibtcltestcyr;
--create user ibtcltestcyr password 'тест';

create database "ibtcltest.fdb";

create table test (i integer, s varchar(10));
commit;
insert into test values (1, 'one');
insert into test values (2, 'two');
insert into test values (3, 'three');
commit;

create table testcyr (i integer, s varchar(10) character set WIN1251);
commit;
insert into testcyr values (1, 'один');
insert into testcyr values (2, 'два');
insert into testcyr values (3, 'три');
commit;

create table testtypes
(
    i integer,
    ih smallint,
    ib bigint,

    n numeric,
    n1 numeric(10),
    n2 numeric(10,2),

    e decimal,
    e1 decimal(10),
    e2 decimal(10,2),

    f float,
    f2 double precision,

    d date,
    t time,
    ts timestamp,

    c char,
    c1 char(10),

    h nchar ,
    h1 nchar(10),

    v varchar(10) character set WIN1251
);
commit;
insert into testtypes values(
    1    /*integer*/,
    2    /*smallint*/,
    3    /*bigint*/,

    4    /*numeric*/,
    5    /*numeric(10)*/,
    6.66 /*numeric(10,2)*/,

    7    /*decimal*/,
    8    /*decimal(10)*/,
    9.99 /*decimal(10,2)*/,

    10.10 /*float*/,
    20.20 /*double precision*/,

    '2001-01-31'               /*date*/,
    '12:34:56.7890'            /*time*/,
    '2016-12-25 12:34:56.7890' /*timestamp*/,

    'c'  /*char*/,
    'c1' /*char(10)*/,

    'h'  /*nchar*/,
    'h1' /*nchar(10)*/,

    'v'  /*varchar(10)*/
);
commit;
insert into testtypes values(
    -2147483648          /*integer*/,
    -32768               /*smallint*/,
    -9223372036854775808 /*bigint*/,

    -32768    /*numeric*/,
    -32768    /*numeric(10)*/,
    -327.68   /*numeric(10,2)*/,

    -99999999    /*decimal*/,
    -999999999    /*decimal(10)*/,
    -9999999.99 /*decimal(10,2)*/,

    -9999.99 /*float*/,
    -9999999999999.99 /*double precision*/,

    '0001-01-01'               /*date*/,
    '00:00:00.0000'            /*time*/,
    '2001-01-01 00:00:00.0000' /*timestamp*/,

    ''  /*char*/,
    ''  /*char(10)*/,

    ''  /*nchar*/,
    ''  /*nchar(10)*/,

    ''  /*varchar(10)*/
);
commit;

insert into testtypes values(
    2147483647          /*integer*/,
    32767               /*smallint*/,
    9223372036854775807 /*bigint*/,

    32767    /*numeric*/,
    32767    /*numeric(10)*/,
    327.67   /*numeric(10,2)*/,

    99999999    /*decimal*/,
    999999999    /*decimal(10)*/,
    9999999.99 /*decimal(10,2)*/,

    9999.99 /*float*/,
    9999999999999.99 /*double precision*/,

    '0001-01-01'               /*date*/,
    '00:00:00.0000'            /*time*/,
    '2001-01-01 00:00:00.0000' /*timestamp*/,

    'X'  /*char*/,
    'XXXXXXXXXX'  /*char(10)*/,

    'x'  /*nchar*/,
    'xxxxxxxxxx'  /*nchar(10)*/,

    'ЙЙЙЙЙЙЙЙЙЙ'  /*varchar(10)*/
);
commit;

grant all on test    to ibtcltest;
grant all on testcyr to ibtcltest;
grant all on test    to ibtcltestcyr;
grant all on testcyr to ibtcltestcyr;
commit;

--create database "ibtclтест.fdb" set names 'win1251' default character set win1251 collation win1251;

--create table "тест" (i integer, s varchar(10) character set WIN1251);
--commit;

--insert into "тест" values (1, 'один');
--insert into "тест" values (2, 'два');
--insert into "тест" values (3, 'три');
--commit;

--grant all on "тест" to ibtcltest;
--grant all on "тест" to ibtcltestcyr;

--commit;
