#!/usr/bin/expect
set timeout 50
spawn ssh pi@192.168.20.19
expect "password:"
send "123\r"
expect "*#"
interact
