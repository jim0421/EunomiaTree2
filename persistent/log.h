#ifndef LOG_H
#define LOG_H

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define CHUNCKSIZE 1024*1024 //1MB
#define PAGESIZE 4*1024 //4KB

class Log {


	struct Buffer{
		char* start;
		char* cur;
		char* end;
	};
	
	const char* path;
	int fd;
	int length;
	
	bool sync;
	
	Buffer buf;
	
public:
		
	Log(const char* path, bool sync = false);

	~Log();
	
	void writeLog(char *data, int len);

	void enlarge(int inc_size);
	
	void closelog();
 	
}; 

#endif
