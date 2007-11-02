#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "firemake.h"
#include "firestring.h"

void error_handler() {
	fprintf(stderr,"Error handler functions properly\n");
}

int main() {
	char buffer[1024];
	long long i = 1024;
	struct firestring_estr_t out, in;
	struct firestring_conf_t *value;
	int f;

	firestring_estr_alloc(&out,1000);
	firestring_estr_alloc(&in,200);

	firestring_estr_sprintf(&out,"-5551212: '%d'\n",-5551212);
	write(1,out.s,out.l);
	firestring_snprintf(buffer,1024,"8675309: '%d'\n",8675309);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"-5551212: '%d'\n",-5551212);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"3: '%d'\n",3);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"0: '%d'\n",0);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"%%3d (3): '%3d'\n",3);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"%%3d (0): '%3d'\n",0);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"5.9382: '%f'\n",5.9382);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"-5.9382: '%f'\n",-5.9382);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"%%10f (-5.9382): '%10f'\n",-5.9382);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"%%010f (-5.9382): '%010f'\n",-5.9382);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"%%4f (-5.9382): '%4f'\n",-5.9382);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"%%4f (5.9382): '%4f'\n",5.9382);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"38: '%d'\n",38);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"%%03d (1): '%03d'\n",1);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"%%050d (-123456): '%050d'\n",-123456);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"%%010d (-123456): '%010d'\n",-123456);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"%%50d (-123456): '%50d'\n",-123456);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"%%s %%d %%s %%d %%s %%s: %s %d %s %d %s %s\n","foo",2002,"bar",38,"fluffernutter","humbug");
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"the cow says moo: '%s'\n","the cow says moo");
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"(null): '%s'\n",NULL);
	printf("%s",buffer);
	firestring_snprintf(buffer,1024,"long long bah: %g %s\n",i,"bah");
	printf("%s",buffer);
	firestring_snprintf(buffer,5,"%d",1234);
	printf("5,1234: '%s'\n",buffer);
	firestring_snprintf(buffer,5,"%d",12345);
	printf("5,12345: '%s'\n",buffer);
	firestring_snprintf(buffer,6,"%s","foobar");
	printf("6,foobar: '%s'\n",buffer);
	firestring_snprintf(buffer,7,"%s","foobar");
	printf("7,foobar: '%s'\n",buffer);
	printf("'aardvark'/'zebra': %d\n",firestring_strcasecmp("aardvark","zebra"));
	printf("'zebra'/'aardvark': %d\n",firestring_strcasecmp("zebra","aardvark"));
	printf("'zebra'/'zebra': %d\n",firestring_strcasecmp("zebra","zebra"));
	printf("'zebra'/'zebra1': %d\n",firestring_strcasecmp("zebra","zebra1"));
	printf("'zebra1'/'zebra': %d\n",firestring_strcasecmp("zebra1","zebra"));
	printf("'zebra1'/'zebra' [5]: %d\n",firestring_strncasecmp("zebra1","zebra",5));
	printf("'zebra1'/'zebra' [6]: %d\n",firestring_strncasecmp("zebra1","zebra",6));
	printf("'zebra'/'zebra1' [6]: %d\n",firestring_strncasecmp("zebra","zebra1",6));

	firestring_estr_strcpy(&in,"zebra");
	firestring_estr_strcpy(&out,"zebra");
	printf("'zebra' (e) in 'zebra': %ld\n",firestring_estr_estrstr(&in,&out,0));

	firestring_estr_strcpy(&in,"ZeBrA");
	firestring_estr_strcpy(&out,"zebra");
	printf("'zebra' (e) in 'ZeBrA': %ld\n",firestring_estr_estrstr(&in,&out,0));

	firestring_estr_strcpy(&in,"virus.exe");
	firestring_estr_strcpy(&out,".exe");
	printf("'.exe' (e) in 'virus.exe': %ld\n",firestring_estr_estrstr(&in,&out,0));

	firestring_estr_strcpy(&in,"virus.exe.scr");
	firestring_estr_strcpy(&out,".exe");
	printf("'.exe' (e) in 'virus.exe.scr': %ld\n",firestring_estr_estrstr(&in,&out,0));

	firestring_estr_strcpy(&in,"zebra");
	firestring_estr_strcpy(&out,"zebra");
	printf("'zebra' (ei) in 'zebra': %ld\n",firestring_estr_estristr(&in,&out,0));

	firestring_estr_strcpy(&in,"ZeBrA");
	firestring_estr_strcpy(&out,"zebra");
	printf("'zebra' (ei) in 'ZeBrA': %ld\n",firestring_estr_estristr(&in,&out,0));

	firestring_estr_strcpy(&in,"virus.exe");
	firestring_estr_strcpy(&out,".exe");
	printf("'.exe' (ei) in 'virus.exe': %ld\n",firestring_estr_estristr(&in,&out,0));

	firestring_estr_strcpy(&in,"vIrUs.eXe");
	firestring_estr_strcpy(&out,".exe");
	printf("'.exe' (ei) in 'vIrUs.eXe': %ld\n",firestring_estr_estristr(&in,&out,0));

	firestring_estr_strcpy(&in,"virus.exe.scr");
	firestring_estr_strcpy(&out,".exe");
	printf("'.exe' (ei) in 'virus.exe.scr': %ld\n",firestring_estr_estristr(&in,&out,0));

	firestring_estr_strcpy(&in,"ViRuS.ExE.ScR");
	firestring_estr_strcpy(&out,".exe");
	printf("'.exe' (ei) in 'ViRuS.ExE.ScR': %ld\n",firestring_estr_estristr(&in,&out,0));

	firestring_estr_strcpy(&in,"virus.exe");
	printf("'.exe' in 'virus.exe': %ld\n",firestring_estr_strstr(&in,".exe",0));

	firestring_estr_strcpy(&in,"ViRuS.ExE");
	printf("'.exe' (i) in 'ViRuS.ExE': %ld\n",firestring_estr_stristr(&in,".exe",0));

	firestring_estr_strcpy(&in,"ViRuS.ExE");
	firestring_estr_strcpy(&out,".bat");
	printf("'.bat' on the end of 'ViRuS.ExE': %d\n",firestring_estr_eends(&in,&out));

	firestring_estr_strcpy(&in,"ViRuS.ExE");
	firestring_estr_strcpy(&out,".exe");
	printf("'.exe' on the end of 'ViRuS.ExE': %d\n",firestring_estr_eends(&in,&out));

	firestring_estr_strcpy(&in,"ViRuS.ExE");
	firestring_estr_strcpy(&out,"tbear");
	printf("'tbear' on the start of 'ViRuS.ExE': %d\n",firestring_estr_estarts(&in,&out));

	firestring_estr_strcpy(&in,"ViRuS.ExE");
	firestring_estr_strcpy(&out,"virus");
	printf("'virus' on the start of 'ViRuS.ExE': %d\n",firestring_estr_estarts(&in,&out));

	firestring_estr_strcpy(&in,"\r.\r\n");
	firestring_estr_strcpy(&out,".\r\n");
	printf("'.\\r\\n' on the start of '\\r.\\r\\n': %d\n",firestring_estr_eends(&in,&out));

	firestring_estr_strcpy(&in,"testuser1");
	firestring_estr_base64_encode(&out,&in);
	firestring_estr_0(&out);
	printf("base64_encode('testuser1'): %s\n",out.s);

	firestring_estr_strcpy(&in,"testuser12");
	firestring_estr_base64_encode(&out,&in);
	firestring_estr_0(&out);
	printf("base64_encode('testuser12'): %s\n",out.s);

	firestring_estr_strcpy(&in,"testuser123");
	firestring_estr_base64_encode(&out,&in);
	firestring_estr_0(&out);
	printf("base64_encode('testuser123'): %s\n",out.s);

	firestring_estr_strcpy(&in,"dGVzdHVzZXIx");
	firestring_estr_base64_decode(&out,&in);
	firestring_estr_0(&out);
	printf("base64_decode('dGVzdHVzZXIx'): %s\n",out.s); 

	firestring_estr_strcpy(&in,"dGVzdHVzZXIxMg==");
	firestring_estr_base64_decode(&out,&in);
	firestring_estr_0(&out);
	printf("base64_decode('dGVzdHVzZXIxMg=='): %s\n",out.s); 

	firestring_estr_strcpy(&in,"dGVzdHVzZXIxMjM=");
	firestring_estr_base64_decode(&out,&in);
	firestring_estr_0(&out);
	printf("base64_decode('dGVzdHVzZXIxMjM='): %s\n",out.s); 

	firestring_estr_strcpy(&in,"dGVzdHVzZXI\nxMjM=");
	firestring_estr_base64_decode(&in,&in);
	firestring_estr_0(&in);
	printf("base64_decode('dGVzdHVzZXI\\nxMjM=') -- in place: %s\n",in.s);

	firestring_estr_strcpy(&in,"Aladdin:open sesame");
	firestring_estr_base64_encode(&out,&in);
	firestring_estr_0(&out);
	printf("base64_encode('Aladdin:open sesame'): %s\n",out.s);

	firestring_estr_strcpy(&in,"QWxhZGRpbjpvcGVuIHNlc2FtZQ==");
	firestring_estr_base64_decode(&out,&in);
	firestring_estr_0(&out);
	printf("base64_decode('QWxhZGRpbjpvcGVuIHNlc2FtZQ=='): %s\n",out.s);

	firestring_estr_strcpy(&in,"QWxhZGRpbjpvcGVuIHNlc2FtZQ==");
	firestring_estr_base64_decode(&in,&in);
	firestring_estr_0(&in);
	printf("base64_decode('QWxhZGRpbjpvcGVuIHNlc2FtZQ==') -- in place: %s\n",in.s);

	firestring_estr_strcpy(&in,"QWxhZGRpbjpvcGVuIHNlc2FtZQ=\n=");
	firestring_estr_base64_decode(&in,&in);
	firestring_estr_0(&in);
	printf("base64_decode('QWxhZGRpbjpvcGVuIHNlc2FtZQ=\\n=') -- in place: %s\n",in.s);

	firestring_estr_strcpy(&in,"QWxhZGRpbjpvcGVuIHNlc2FtZQ==     ");
	firestring_estr_base64_decode(&in,&in);
	firestring_estr_0(&in);
	printf("base64_decode('QWxhZGRpbjpvcGVuIHNlc2FtZQ==     ') -- in place: %s\n",in.s);

	f = open("/dev/urandom",O_RDONLY);
	if (f == -1) {
		perror("open");
		exit(1);
	}

	if (read(f,in.s,200) != 200) {
		perror("read");
		exit(1);
	}
	in.l = 200;

	firestring_estr_base64_encode(&out,&in);
	firestring_estr_0(&out);
	printf("base64_encode(random data): %s\n",out.s);

	firestring_estr_strcpy(&in,"test of xml decode");
	firestring_estr_xml_decode(&out,&in);
	firestring_estr_0(&out);
	printf("xml_decode('test of xml decode'): %s\n",out.s);

	firestring_estr_strcpy(&in,"test of xml decode &amp; foo");
	firestring_estr_xml_decode(&out,&in);
	firestring_estr_0(&out);
	printf("xml_decode('test of xml decode &amp; foo'): %s\n",out.s);
	
	firestring_estr_strcpy(&in,"test of xml decode &amp; foo");
	firestring_estr_xml_decode(&in,&in);
	firestring_estr_0(&in);
	printf("xml_decode('test of xml decode &amp; foo') -- in place: %s\n",in.s);
	
	firestring_estr_strcpy(&in,"&quot;there is no&nbsp;spoon &amp; the cat's in the cradle&quot;");
	firestring_estr_xml_decode(&out,&in);
	firestring_estr_0(&out);
	printf("xml_decode('&quot;there is no&nbsp;spoon &amp; the cat's in the cradle&quot;'): %s\n",out.s);

	firestring_estr_strcpy(&in,"&#9;&#85;&#160;&#169;");
	firestring_estr_xml_decode(&out,&in);
	firestring_estr_0(&out);
	printf("xml_decode('&#9;&#85;&#160;&#169;'): %s\n",out.s);

	firestring_estr_strcpy(&in,"&quot;there is no&nbsp;spoon &amp; the cat's in the cradle&quot;");
	firestring_estr_xml_decode(&out,&in);
	firestring_estr_0(&out);
	printf("xml_decode('&quot;there is no&nbsp;spoon &amp; the cat's in the cradle&quot;'): %s\n",out.s);

	firestring_estr_strcpy(&in,"&#x09;&#x55;&#xa0;&#xA9;");
	firestring_estr_xml_decode(&out,&in);
	firestring_estr_0(&out);
	printf("xml_decode('&#x09;&#x55;&#xa0;&#xA9;'): %s\n",out.s);

	firestring_estr_strcpy(&in,"\"there is no spoon & the cat's <> in the cradle\"");
	firestring_estr_xml_encode(&out,&in);
	firestring_estr_0(&out);
	printf("xml_encode('\"there is no spoon & the cat's <> in the cradle\"'): %s\n",out.s);

	firestring_estr_strcpy(&in,"\"test of {replaceme} replace function {replaceme}\"");
	firestring_estr_replace(&out,&in,&(struct firestring_estr_t) { "REPLACED", 8, 8 },&(struct firestring_estr_t) { "{replaceme}", 11, 11 });
	firestring_printf("replace('test of {replaceme} replace function {replaceme}\"','REPLACED','{replaceme}'): '%e'\n",&out);

	firestring_estr_strcpy(&in,"\"test of {replaceme} replace function {replaceme}\"");
	firestring_estr_replace(&out,&in,&(struct firestring_estr_t) { "REPLACED123456", 14, 14 },&(struct firestring_estr_t) { "{replaceme}", 11, 11 });
	firestring_printf("replace('test of {replaceme} replace function {replaceme}\"','REPLACED123456','{replaceme}'): '%e'\n",&out);

	value = firestring_conf_parse("/tmp/firestring.conf");
	printf("firestring_conf_parse('/tmp/firestring.conf') -- testvar: %s\n",value == NULL ? "failed" : firestring_conf_find(value,"testvar"));
	printf("firestring_conf_parse('/tmp/firestring.conf') -- testvar2: %s\n",value == NULL ? "failed" : firestring_conf_find(value,"testvar2"));
	{
		char *target_user = NULL;
		while ((target_user = firestring_conf_find_next(value,"target_user",target_user)) != NULL)
			printf("firestring_conf_find_next('target_user'): %s\n",target_user);
	}

	firestring_printf("firestring_printf() test\n");
	firestring_fprintf(stderr,"firestring_fprintf() test\n");

	firestring_estr_strcpy(&in,"this is an estr");
	firestring_printf("test: %e\n",&in);

	firestring_printf("time: %t\n",time(NULL));

	firestring_set_error_handler(error_handler);
	firestring_malloc(-1);

	firestring_estr_free(&in);
	firestring_estr_free(&out);

	return 0;
}
