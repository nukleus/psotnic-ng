#ifndef OP_HPP
#define OP_HPP 

class Op : public Module
{
	public:
	Op(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir);

	virtual bool onLoad(string &msg);
	virtual void onPrivmsg(const char *from, const char *to, const char *msg);
};

#endif /* OP_HPP */
