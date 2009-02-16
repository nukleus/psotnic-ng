#!/usr/bin/perl

use strict;
use FileHandle;

if( -f "seed.h" )
{
	print "[!] Random seed (seed.h) exists, using existing one.\n";
}
else
{
	my $f = new FileHandle "> seed.h";
	if( defined $f )
	{
		print $f "/*\n";
		print $f " * This file contains magical seeds that will be used during encryption/decryption process.\n";
		print $f " * So make sure that you store it in safe place, if you loose it you wont be able to decode\n";
		print $f " * crypted files.\n";
		print $f " */\n\n";
		print $f "#ifndef SEED_H\n";
		print $f "#define SEED_H 1\n\n";

		my $cfg_seed = "";
		my $ul_seed = "";

		for(my $i=0; $i<16; ++$i) {
			$cfg_seed .= sprintf("\\x%02x", rand(256));
		}

		for(my $i=0; $i<16; ++$i) {
			$ul_seed .= sprintf("\\x%02x", rand(256));
		}

		print $f "static unsigned char cfg_seed[] = \"$cfg_seed\";\n";
		print $f "static unsigned char ul_seed[] = \"$ul_seed\";\n\n";
		print $f "#endif /* SEED_H */\n";
	}
	else
	{
		print "FATAL: Cannot generate seed.h\n";
		die;
	}
	$f->close;
}
