#ifndef RANDOM_HPP
#define RANDOM_HPP 

#include "classes.h"
#include "ptrlist.h"
namespace Psotnic
{
	int rand();
	void srand(int a=0, int b=0, int c=0);
}

int getRandomNumbers(int top, int *ret, int num);
int getRandomItems(chanuser **ret, ptrlist<chanuser>::iterator start, int interval, int num, int ex=0);

#endif /* RANDOM_HPP */
