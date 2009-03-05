#ifndef CLONE_HPP
#define CLONE_HPP 

class Chanuser;

class clone_host
{
	public:
	chanuser *user;
	time_t cr;
	int type;

	clone_host(chanuser *u, int t);
	time_t creation();

	int operator==(const clone_host &c);
	int operator&(const clone_host &c);
};

class clone_ident
{
	public:
	chanuser *user;
	time_t cr;

	clone_ident(chanuser *u);
	time_t creation();

	int operator==(const clone_ident &c);
	int operator&(const clone_ident &c);

};

class clone_proxy
{
	public:
	chanuser *user;
	time_t cr;

	clone_proxy(chanuser *u);
	time_t creation();

	int operator==(const clone_proxy &c);
	int operator&(const clone_proxy &c);
};

#endif /* CLONE_HPP */
