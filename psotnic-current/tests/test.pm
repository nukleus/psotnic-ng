#!/usr/bin/perl

use strict;

my ($lflags, $cflags);
my %have;

our ($antiptrace, $noircbacktrace, $ssl, $little_endian, $big_endian, $cc_prefix, $cc_options, $ld_options, $disable_adns, $firedns);

$lflags = "$ld_options ";
$cflags = "$cc_options ";

sub tryCompile
{
    my ($cc, $file, @opts) = @_;
    my ($i, $j, $line);
    
    print "`-> testing $file: ";
    
    if(!system("$cc -o a.out $file $lflags 2> /dev/null" )) {
        print "ok\n";
        $have{$file} = 1;
        return;
    }
    
    for($i=0, ; $i<$#opts+1; $i++)
    {
        for($j=$i, $line=""; $j <$#opts+1; $j++)
	{
	    $line .= $opts[$j] . " ";
	    
	    if(!system("$cc $file $line $lflags 2> /dev/null")) {
		print "ok, with: $line\n";
		$lflags .= $line . " ";
		$have{$file} = 1;
		return;
	    }
	}
    }
    
    print "failed\n";
}

sub checkCC
{
	my $cc = shift;
	print "`-> looking for $cc: ";

	if(!system("$cc -v 2>/dev/null"))
	{
		print "found\n";
		$have{$cc} = 1;
		return;
	}
	
	print "not found\n";	
}

sub getGccOptions
{
    undef %have;
   
	#look for gcc
	checkCC("${cc_prefix}gcc");
	if(!$have{"${cc_prefix}gcc"})
	{
		die "please instal ${cc_prefix}gcc to continue...";
	} 
   
	#look for g++ 
    checkCC("${cc_prefix}g++");
    if(!$have{"${cc_prefix}g++"})
    {
        die "please instal ${cc_prefix}g++ to continue...";
    }

	my $endian;
	if($big_endian) {
		$endian = "BIG_ENDIAN\n";
	}
	elsif($little_endian) {
		$endian = "LITTLE_ENDIAN\n";
	}
	else {
		tryCompile("${cc_prefix}gcc", 'endian.c', '-lsocket', '-lnsl', '-lc', '-lm');
		$endian = `./a.out`;
	}

	print "`-> looking for processor architecture: ";

	if("$endian" eq "LITTLE_ENDIAN\n")
	{
		$cflags .= "-DHAVE_LITTLE_ENDIAN ";
		print "little endian\n";
	}
	elsif("$endian" eq "BIG_ENDIAN\n")
	{
		$cflags .= "-DHAVE_BIG_ENDIAN ";
		print "big endian\n";
	}
	else
	{
		print "unknown: $endian\n";
		die;
	}

	tryCompile("${cc_prefix}gcc", 'select.c', '-lsocket', '-lnsl', '-lc', '-lm');
	my $fd_setsize = `./a.out`;

	print "`-> determining select set size: ";

	if($fd_setsize < 1024)
	{
		print "too small ($fd_setsize), redefining to 1024\n";
		$cflags .= "-DFD_SETSIZE=1024 ";
	}
	else
	{
		print "ok ($fd_setsize)\n";
	}

	#determine linker options
    tryCompile("${cc_prefix}g++", 'ipv4.c', '-lsocket', '-lnsl', '-lc', '-lm');

    if(!$have{'ipv4.c'})
    {
		die 'woo hoo hoo, your system cannot compile event the simplest code, do you have g++ installed? :)';
    }

    #check for stack protector
    if(!system("${cc_prefix}g++ empty.c -fno-stack-protector 2> /dev/null" )) {
        print "`-> looking for stack protector: found, disabling\n";
        $lflags .= '-fno-stack-protector ';
    }
    else
    {
        print "`-> looking for stack protector: not found (good)\n";
    }

	tryCompile("${cc_prefix}g++", 'math.c', '-lsocket', '-lnsl', '-ldl', '-lc', '-lm');
    tryCompile("${cc_prefix}g++", 'ipv6.c', '-lsocket', '-lnsl', '-ldl', '-lc', '-lm');
    if(!$disable_adns) {
		tryCompile("${cc_prefix}g++", 'pthread.c', '-lthread', '-lpthread', '-lm');
	}
    tryCompile("${cc_prefix}g++", 'gethostbyname.c', '-lsocket', '-lnsl', '-ldl', '-lc', '-lm');
    tryCompile("${cc_prefix}g++", 'gethostbyname2.c', '-lsocket', '-lnsl', '-ldl', '-lc', '-lm');
    tryCompile("${cc_prefix}g++", 'gethostbyname2_r.c', '-lsocket', '-lnsl', '-ldl', '-lc', '-lm');
    tryCompile("${cc_prefix}g++", 'dlopen.c', '-ldl', '-lc', '-lm');
    tryCompile("${cc_prefix}g++", 'resolv.c', '-lresolv', '-lm');

    #determine compiler options
    if($have{'ipv6.c'})
    {
        $cflags .= "-DHAVE_IPV6 ";
    }
    
    if(!$have{'gethostbyname2.c'})
    {
        $cflags .= "-DNO_6DNS ";
    }

	if(!$disable_adns)
	{
		if($firedns)
		{
			$cflags .= "-DHAVE_ADNS -DHAVE_ADNS_FIREDNS ";
		}
	    elsif($have{'gethostbyname2_r.c'})
    	{
			$cflags .= "-DHAVE_ADNS -DHAVE_ADNS_PTHREAD ";
	    }
	}

    if(!$have{'dlopen.c'})
    {
        $cflags .= "-DHAVE_STATIC ";
	
    }
	
	return ($cflags, $lflags);
}

#getGccOptions();
#getGccOptions();

1;
