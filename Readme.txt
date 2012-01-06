First Name: Ashwin Kumar

Last Name : Gopi Valliammal

Email: gopivall@usc.edu

Project : CS 551 Warmup Project 1 Fall 2009

Date : Sep-11-2009

Files Submitted
----------------
1.common.cc
2.common.h
3.server.h
4.server.cc
5.client.h
6.client.cc
7.Makefile
8.Readme.txt


Some Design assumptions
------------------------

1. The server can queue up only 25 connection in the listening socket. If you want to change this you have change the number 25 the following line "#define CON_MAX 25" in server.h to any number tht you want
2. If bad offset argument is given(eg: -ve numbers for offset) no request is sent to server.
3. If the srting in query from the client is more than 512 bytes long no request is sent to the server.
4. Offset will always be zero in server reply messages 
5. if "-m" option is not specified for the client or for the server program will not any print message to stdout.

Bugs
-----
None

Deviation from Spec
-------------------
None
