#!/usr/bin/env python3

import random

ones = ["", "one ","two ","three ","four ", "five ", "six ","seven ","eight ","nine ","ten ","eleven ","twelve ", "thirteen ", "fourteen ", "fifteen ","sixteen ","seventeen ", "eighteen ","nineteen "]
twenties = ["","","twenty ","thirty ","forty ", "fifty ","sixty ","seventy ","eighty ","ninety "]
thousands = ["","thousand ","million ", "billion ", "trillion ", "quadrillion ", "quintillion ", "sextillion ", "septillion ","octillion ", "nonillion ", "decillion ", "undecillion ", "duodecillion ", "tredecillion ", "quattuordecillion ", "quindecillion", "sexdecillion ", "septendecillion ", "octodecillion ", "novemdecillion ", "vigintillion "]

def num999(n):
    c = n % 10 # singles digit
    b = ((n % 100) - c) / 10 # tens digit
    a = ((n % 1000) - (b * 10) - c) / 100 # hundreds digit
    t = ""
    h = ""

    if a != 0 and b == 0 and c == 0:
        t = ones[int(a)] + "hundred "
    elif a != 0:
        t = ones[int(a)] + "hundred and "
    if b <= 1:
        h = ones[int(n) % 100]
    elif b > 1:
        h = twenties[int(b)] + ones[int(c)]

    st = t + h
    return st

def num2word(num):
    if num == 0:
        return 'zero'

    i = 3
    n = str(num)
    word = ""
    k = 0

    while (i == 3):
        nw = n[-i:]
        n = n[:-i]
        if int(nw) == 0:
            word = num999(int(nw)) + thousands[int(nw)] + word
        else:
            word = num999(int(nw)) + thousands[k] + word
        if n == '':
            i += 1
        k += 1

    return word[:-1]

print('generating test1.sql...')
with open('test1.sql', 'w') as f:
    f.write('DROP TABLE IF EXISTS t1;\n')
    f.write('CREATE TABLE t1(a INTEGER, b INTEGER, c VARCHAR(100));\n')
    for i in range(1000):
        r = random.randint(1, 1000000)
        f.write('INSERT INTO t1 VALUES(%d, %d, \'%s\');\n' % (i, r, num2word(r)))

print('generating test2.sql...')
with open('test2.sql', 'w') as f:
    f.write('DROP TABLE IF EXISTS t2;\n')
    f.write('BEGIN;\n')
    f.write('CREATE TABLE t2(a INTEGER, b INTEGER, c VARCHAR(100));\n')
    for i in range(100000):
        r = random.randint(1, 1000000)
        f.write('INSERT INTO t2 VALUES(%d, %d, \'%s\');\n' % (i, r, num2word(r)))
    f.write('COMMIT;\n')

print('generating test3.sql...')
with open('test3.sql', 'w') as f:
    f.write('DROP TABLE IF EXISTS t3;\n')
    f.write('BEGIN;\n')
    f.write('CREATE TABLE t3(a INTEGER, b INTEGER, c VARCHAR(100));\n')
    f.write('CREATE INDEX i3 ON t3(c);\n')
    for i in range(100000):
        r = random.randint(1, 1000000)
        f.write('INSERT INTO t3 VALUES(%d, %d, \'%s\');\n' % (i, r, num2word(r)))
    f.write('COMMIT;\n')

print('generating test4.sql...')
with open('test4.sql', 'w') as f:
    f.write('BEGIN;\n')
    for i in range(100):
        f.write('SELECT count(*), avg(b) FROM t2 WHERE b >= %d AND b < %d;\n' % (i * 100, (i * 100) + 900))
    f.write('COMMIT;\n')

print('generating test5.sql...')
with open('test5.sql', 'w') as f:
    f.write('BEGIN;\n')
    for i in range(100):
        f.write('SELECT count(*), avg(b) FROM t2 WHERE c LIKE \'%' + num2word(i) + '%\';\n')
    f.write('COMMIT;\n')

print('generating test6.sql...')
with open('test6.sql', 'w') as f:
    f.write('CREATE INDEX i2a ON t2(a);\n')
    f.write('CREATE INDEX i2b ON t2(b);\n')

print('generating test7.sql...')
with open('test7.sql', 'w') as f:
    for i in range(5000):
        f.write('SELECT count(*), avg(b) FROM t2 WHERE b >= %d AND b < %d;\n' % (i * 100, (i * 100) + 900))

print('generating test8.sql...')
with open('test8.sql', 'w') as f:
    f.write('BEGIN;\n')
    for i in range(1000):
        f.write('UPDATE t1 SET b=b*2 WHERE a>=%d AND a<%d;\n' % (i * 10, (i + 10) * 10))
    f.write('COMMIT;\n')

print('generating test9.sql...')
with open('test9.sql', 'w') as f:
    f.write('BEGIN;\n')
    for i in range(25000):
        r = random.randint(1, 1000000)
        f.write('UPDATE t2 SET b=%d WHERE a=%d;\n' % (r, i+1))
    f.write('COMMIT;\n')

print('generating test10.sql...')
with open('test10.sql', 'w') as f:
    f.write('BEGIN;\n')
    for i in range(25000):
        r = random.randint(1, 1000000)
        f.write('UPDATE t2 SET c=\'%s\' WHERE a=%d;\n' % (num2word(r), i+1))
    f.write('COMMIT;\n')

print('generating test11.sql...')
with open('test11.sql', 'w') as f:
    f.write('BEGIN;\n')
    f.write('INSERT INTO t1 SELECT b,a,c FROM t2;\n')
    f.write('INSERT INTO t2 SELECT b,a,c FROM t1;\n')
    f.write('COMMIT;\n')

print('generating test12.sql...')
with open('test12.sql', 'w') as f:
    f.write('DELETE FROM t2 WHERE c LIKE \'%fifty%\';\n')

print('generating test13.sql...')
with open('test13.sql', 'w') as f:
    f.write('DELETE FROM t2 WHERE a > 10 AND a < 20000;\n')

print('generating test14.sql...')
with open('test14.sql', 'w') as f:
    f.write('INSERT INTO t2 SELECT * FROM t1;\n')

print('generating test15.sql...')
with open('test15.sql', 'w') as f:
    f.write('BEGIN;\n')
    f.write('DELETE FROM t1;\n')
    for i in range(12000):
        r = random.randint(1, 1000000)
        f.write('INSERT INTO t1 VALUES(%d, %d, \'%s\');\n' % (i, r, num2word(r)))
    f.write('COMMIT;\n')

print('generating test16.sql...')
with open('test16.sql', 'w') as f:
    f.write('DROP TABLE t1;\n')
    f.write('DROP TABLE t2;\n')
    f.write('DROP TABLE t3;\n')
