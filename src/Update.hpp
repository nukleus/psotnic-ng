#ifndef UPDATE_HPP
#define UPDATE_HPP 

#include "Inetconn.hpp"

class update
{
	public:
	inetconn child;
	int parent;
	pid_t pid;
	pid_t child_pid;

	update();
	bool forkAndGo(char *site);
	void end();
	bool doUpdate(const char *site);
};

#endif /* UPDATE_HPP */
