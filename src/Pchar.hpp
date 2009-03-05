#ifndef PCHAR_HPP
#define PCHAR_HPP 

class Pchar
{
	public:
	char *data;
	int step;
	int alloced;
	int len;

	Pchar(int s=4096);
	char *push(const char *str, int l=-1);
	char *push(const char c);
	~Pchar();
	void clean();
};

#endif /* PCHAR_HPP */
