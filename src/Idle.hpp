#ifndef IDLE_HPP
#define IDLE_HPP 

class idle
{
	public:
	char *away;
	time_t nextStatus;
	time_t lastStatus;
	time_t nextMsg;
	time_t lastMsg;

	const char **awayReasons;
	const char **backReasons;
	const char **awayAdd;
	const char **backAdd;

	idle();
	int spread(int x);
	void calcNextStatus();
	void calcNextMsg();
	void sendMsg();
	void eval();
	void togleStatus();
	void init();
	int getIdle();
	int getET();
	int getRT();
	void load();
	const char *getRandAwayMsg();
	const char *getRandBackMsg();
	const char *getRandAwayAdd();
	const char *getRandBackAdd();
};

extern idle antiidle;

#endif /* IDLE_HPP */
