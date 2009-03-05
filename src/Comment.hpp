#ifndef COMMENT_HPP
#define COMMENT_HPP 

#include "ptrlist.h"

class Comment
{
	public:
	class entry
	{
		public:
		char *key;
		char *value;

		entry(const char *k, const char *v);
		~entry();

		int operator==(const entry &ent) const;
		int operator<(const entry &e) const;
	};

	ptrlist<entry> data;

	int add(char *key, char *value);
	int del(char *key);
	char *get(char *key);
	Comment();
};

#endif /* COMMENT_HPP */
