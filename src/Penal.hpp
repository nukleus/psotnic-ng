#ifndef PENAL_HPP
#define PENAL_HPP 

class penal
{
	private:
	time_t when;
	int penality;

	public:
	operator int();
	void calc();
	penal operator+(int n);
	penal operator+=(int n);
	penal operator-(int n);
	penal operator-=(int n);
	penal operator++(int);
	penal operator--(int);
	int val() { return penality; };

	penal(int n=0);
};

extern penal penalty;

#endif /* PENAL_HPP */
