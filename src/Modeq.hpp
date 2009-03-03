#ifndef MODEQ_HPP
#define MODEQ_HPP 



class chan;

class modeq
{

	public:
	class modeq_ent
	{
		public:
		char mode[3];
		char *arg;
		int expire;
		bool backupmode;
		bool reject;

		modeq_ent(time_t exp, const char *m, const char *a=NULL);
		~modeq_ent();

		int operator==(modeq_ent &c);
		int operator&(modeq_ent &c);
	};

	private:
	chan *ch;
	int validate(modeq_ent *e, int &a, int &n);
	grouplist<modeq_ent> data;

	public:

	modeq_ent *add(time_t exp, const char *m, const char *a=NULL);
	int flush(int prio);

	modeq(chan *channel=NULL);
	void setChannel(chan *channel);
	void removeBackupModesFor(const char *mode, const char *arg);
	modeq_ent *find(const char *mode, const char *arg);
};

#endif /* MODEQ_HPP */
