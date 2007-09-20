#!/bin/sh

blah()
{
    echo    "/*******************************************************"
    echo    " * Psotnic framework generated by framework.sh"
    echo    " * Copyright (c) 2004-2005 Grzegorz Rusin <grusin@gmail.com>"
    echo -n " * Created: "; date
    echo    " * "
    echo    " * WARNING! All changes made in this file will be lost!"
    echo    " */"
    echo    ""
}

INC="framework/includes";
INT="framework/internal";

add_include()
{
    echo "[+] Generation $INC/$1"
    echo "#include \"$1\"" >> $INC/framework.h
    blah > $INC/$1
    cat $1 >> $INC/$1
}

#############################################################
echo "[+] Generation $INC/includes.h"
blah > $INC/includes.h
cat prots.h | grep '^#include <.*>' >> $INC/includes.h

#############################################################
echo "[+] Generating $INC/framework.h"

blah > $INC/framework.h

echo "#define HAVE_MODULES 1" >> $INC/framework.h
echo "#include \"includes.h\"" >> $INC/framework.h

add_include "iterator.h"
add_include "ptrlist.h"
add_include "fastptrlist.h"
add_include "tiny_ptrlist.h"
add_include "hashlist.h"
add_include "defines.h"
add_include "pstring.h"
add_include "class-ent.h"
add_include "grass.h"
add_include "common.h"
echo "" > $INC/prots.h

echo "" >> $INC/framework.h
echo "typedef int foobar;" >> $INC/framework.h
echo "typedef int* FUNCTION;" >> $INC/framework.h

echo " " >> $INC/framework.h

cat structs.h classes.h | grep -v '^[*]' | grep -v '^/' | gawk '{ if(match($0, "[(][*]") || !match($0, "[()]")) print $0 }' | \
gawk '{ if(length($0) > 0) print $0 }' | sed -e 's/CBlowFish/foobar/g' >> $INC/framework.h

############################################################
echo "[+] Generating $INC/func.h"
blah > $INC/func.h
cat export.functions.cpp | awk '{ if($1 == "//" || length($0) < 1) { print $0 } else { print $1 " (*" $2 ")" substr($0, length($1) + length($2) + 3) ";" } }' >> $INC/func.h

############################################################
echo "[+] Generating $INC/var.h"
blah > $INC/var.h
cat export.variables.cpp | awk '{ if($1 == "//" || length($0) < 1) { print $0 } else { print $1 " *" $2 ";" } }' >> $INC/var.h

############################################################
echo "[+] Generating $INC/func_extern.h"
blah > $INC/func_extern.h
cat export.functions.cpp | grep -v '^//' | awk '{ if(length($0) > 0) { print "extern " $1 " (*" $2 ")" substr($0, length($1) + length($2) + 3) ";" } }' >> $INC/func_extern.h

############################################################
echo "[+] Generating $INC/var_extern.h"
blah > $INC/var_extern.h
cat export.variables.cpp | grep -v '^//' | awk '{ if(length($0) > 0) { print "extern " $1 " *" $2 ";"} }' >> $INC/var_extern.h

############################################################
echo "[+] Generating load_module.h"
blah > register_module.h
echo "void registerAll(int (*_register)(const char *name, DLSYM_FUNCTION address))" >> register_module.h
echo "{" >> register_module.h
cat export.functions.cpp | grep -v '//' | awk '{ if(length($0) > 0) print "\t_register(\"" $2 "\", (DLSYM_FUNCTION) &" $2 ");" }' >> register_module.h
cat export.variables.cpp | grep -v '//' | awk '{ if(length($0) > 0) print "\t_register(\"" $2 "\", (DLSYM_FUNCTION) obj2fun( (DLSYM_OBJECT) &" $2 "));" }' >> register_module.h
echo "}" >> register_module.h
echo "" >> register_module.h

############################################################
echo "[+] Generating $INT/register.cpp"
blah > $INT/register.cpp

echo "#include \"../includes/framework.h\"" >> $INT/register.cpp
echo "#include \"../includes/func_extern.h\"" >> $INT/register.cpp
echo "#include \"../includes/var_extern.h\"" >> $INT/register.cpp
echo "" >> $INT/register.cpp

echo "int _strcmp(char *s1, char *s2)" >> $INT/register.cpp
echo "{" >> $INT/register.cpp
echo "    unsigned char *str1 = (unsigned char *) s1;" >> $INT/register.cpp
echo "    unsigned char *str2 = (unsigned char *) s2;" >> $INT/register.cpp
echo "    int res;" >> $INT/register.cpp
echo ""	 >> $INT/register.cpp    
echo "    while((res = *str1 - *str2) == 0)" >> $INT/register.cpp
echo "    {" >> $INT/register.cpp
printf "        if(*str1 == '\\\0')\n"  >> $INT/register.cpp
echo "            return 0;" >> $INT/register.cpp
echo "        str1++;" >> $INT/register.cpp
echo "        str2++;" >> $INT/register.cpp
echo "    }" >> $INT/register.cpp
echo "    return res;" >> $INT/register.cpp
echo "}" >> $INT/register.cpp
echo "" >> $INT/register.cpp

echo "extern \"C\" int _register(char *name, FUNCTION address);" >> $INT/register.cpp
echo "" >> $INT/register.cpp
echo "int _register(char *name, FUNCTION address)" >> $INT/register.cpp
echo "{" >> $INT/register.cpp
cat export.functions.cpp | grep -v '//' | awk '{ if(length($0) > 0) print "    if(!_strcmp(\"" $2 "\", name))\n    {\n        memcpy(&" $2 ", &address, sizeof(FUNCTION));\n        return 1;\n    }" }' >> $INT/register.cpp
cat export.variables.cpp | grep -v '//' | awk '{ if(length($0) > 0) print "    if(!_strcmp(\"" $2 "\", name))\n    {\n        memcpy(&" $2 ", &address, sizeof(FUNCTION));\n        return 1;\n    }" }' >> $INT/register.cpp
echo "    return 0;" >> $INT/register.cpp
echo "}" >> $INT/register.cpp
echo "" >> $INT/register.cpp

#############################################################
echo "[+] Generating $INC/psotnic.h"
blah > $INC/psotnic.h
echo "#include \"framework.h\"" >> $INC/psotnic.h
echo "#include \"func.h\"" >> $INC/psotnic.h
echo "#include \"var.h\"" >> $INC/psotnic.h
echo "#include \"common.h\"" >> $INC/psotnic.h
