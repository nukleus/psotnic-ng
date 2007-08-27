###################################################
#  This file is a part of psotnic irc bot code
#  Copyright 2003 Grzegorz Rusin <pks@irc.pl>
#
#  PRIVATE CODE - DO NOT DISTRIBUTE
#
#

all: clean op spam control google

op:
	@echo [*] Making module op.so
	@g++ -o op.so op.cpp internal/register.cpp -shared -g

spam:
	@echo [*] Making module spam.so
	@g++ -o spam.so spam.cpp internal/register.cpp -shared -g

control:
	@echo [*] Making module control.so
	@g++ -o control.so control.cpp internal/register.cpp -shared -g

google:
	@echo [*] Making module google.so
	@g++ -o google.so google.cpp internal/register.cpp -shared -g

clean:
	@echo [*] Cleaning up
	@rm -f *.so