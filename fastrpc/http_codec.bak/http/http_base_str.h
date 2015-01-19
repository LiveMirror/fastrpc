#ifndef _HTTP_SVR_STR_H_
#define _HTTP_SVR_STR_H_

#include <string>
#include <sstream>
#include <sys/time.h>
#include <stdlib.h>




template<typename T> std::string to_str(const T& t)
{
	std::ostringstream s;
	s << t;
	return s.str();
}

template<typename T> T from_str(const std::string& s)
{
	std::istringstream is(s);
	T t;
	is >> t;
	return t;
}

inline void sran()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);
}

inline unsigned int ran(unsigned range)
{
	unsigned r = rand() | (rand() << 16);
	r = r%range;
	//r = (unsigned) ( (0.0+range) * (r+0.0) / (ULONG_MAX+1.0));
	//cerr << "ran " << r << endl;
	return r;
}


#endif

