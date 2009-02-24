/***************************************************************************
 *   Copyright (C) 2003-2005 by Grzegorz Rusin                             *
 *   grusin@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "prots.h"
#include "global-var.h"

void randstr(char *str, int n)
{
	int i;
	srand();
	for(i=0; i<n; ++i) str[i] = rand() % 100 + 32;
}

int getRandomItems(chanuser **ret, ptrlist<chanuser>::iterator p, int interval, int num, int ex)
{
	if(!p) return 0;
	int r, i, k, *arr = new int[num];

	num = getRandomNumbers(interval, arr, num);

	r=k=0;

	//printf("getRandomItems(ret=%p, start=%p, interval=%d, num=%d, ex=%x)\n",
	//	ret, start, interval, num, ex);

	while(p)
	{

		if(!ex || !(p->flags & ex))
		{
			for(i=0; i<num; ++i)
			{
				if(arr[i] == k)
				{
					*(ret++) = p;
					++r;
					break;
				}
			}
			++k;
		}
		p++;
	}

	delete [] arr;
	return r;
}

int getRandomNumbers_safe(int top, int *ret, int num)
{
	int tmp, n, i, *array = new int[top];

	for(i=0; i<top; ++i)
		array[i] = i;

	for(i=0; i<top; ++i)
	{
		n = rand() % top;
		tmp = array[n];
		array[n] = array[i];
		array[i] = tmp;
	}

	num = MIN(num, top);

	memcpy(ret, array, sizeof(int)*num);
	delete [] array;

	return num;

}

int getRandomNumbers(int top, int *ret, int num)
{
	return getRandomNumbers_safe(top, ret, num);
}

XSRand xsrand;

void srand(int a, int b, int c)
{
	xsrand.srand(a ^ b ? a ^ b : nanotime());
}

int rand()
{

	return abs(xsrand.rand());
}

/**
 * xorShif RNG
 */
unsigned int XSRand::a = 13;
unsigned int XSRand::b = 17;
unsigned int XSRand::c = 5;

void XSRand::srand(unsigned int seed)
{
	x = seed;
	rand();
}

unsigned int XSRand::rand()
{
	x ^= x << a;
	x ^= x >> b;
	x ^= x << c;
	return x;
}
