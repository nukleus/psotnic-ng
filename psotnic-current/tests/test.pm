#!/usr/bin/perl

use strict;

my ($lflags, $cflags);
my %have;

sub tryCompile
{
    my ($cc, $file, @opts) = @_;
    my ($i, $j, $line);
    
    print "`-> testing $file: ";
    
    if(!system("$cc $file $lflags 2> /dev/null" )) {
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
    $lflags = "";
    $cflags = "";
    undef %have;
   
	#look for gcc
	checkCC('gcc');
	if(!$have{'gcc'})
	{
		die 'please instal gcc to continue...';
	} 
   
	#look for g++ 
    checkCC('g++');
    if(!$have{'g++'})
    {
        die 'please instal g++ to continue...';
    }

	#get endian
	tryCompile('gcc', 'endian.c', '-lsocket', '-lnsl', '-lc');

	print "`-> looking for processor architecture: ";

	my $endian = `./endian`;
	if("$endian" == "LITTLE_ENDIAN")
	{
		$cflags .= "-DHAVE_LITTLE_ENDIAN ";
		print "little endian\n";
	}
	elsif("$endian" == "BIG_ENDIAN")
	{
		$cflags .= "-DHAVE_BIG_ENDIAN ";
		print "big endian\n";
	}
	else
	{
		print "unknown: $endian\n";
		die;
	}

	#determine linker options
    tryCompile('g++', 'ipv4.c', '-lsocket', '-lnsl', '-lc');

    if(!$have{'ipv4.c'})
    {
		die 'woo hoo hoo, your system cannot compile event the simplest code, do you have g++ installed? :)';
    }

    #check for stack protector
    if(!system("g++ empty.c -fno-stack-protector 2> /dev/null" )) {
        print "`-> looking for stack protector: found, disabling\n";
        $lflags .= '-fno-stack-protector ';
    }
    else
    {
        print "`-> looking for stack protector: not found (good)\n";
    }

    tryCompile('g++', 'ipv6.c', '-lsocket', '-lnsl', '-ldl', '-lc');
    tryCompile('g++', 'pthread.c', '-lthread', '-lpthread');
    tryCompile('g++', 'gethostbyname.c', '-lsocket', '-lnsl', '-ldl', '-lc');
    tryCompile('g++', 'gethostbyname2.c', '-lsocket', '-lnsl', '-ldl', '-lc');
    tryCompile('g++', 'gethostbyname2_r.c', '-lsocket', '-lnsl', '-ldl', '-lc');
    tryCompile('g++', 'dlopen.c', '-ldl', '-lc');
    tryCompile('g++', 'resolv.c', '-lresolv');

    #determine compiler options
    if($have{'ipv6.c'})
    {
        $cflags .= "-DHAVE_IPV6 ";
    }
    
    if(!$have{'gethostbyname2.c'})
    {
        $cflags .= "-DNO_6DNS ";
    }

    if($have{'gethostbyname2_r.c'})
    {
	$cflags .= "-DHAVE_ADNS ";
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
