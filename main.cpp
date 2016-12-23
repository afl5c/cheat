
#ifdef _WIN32
#include "process_win.h"
#endif

#ifdef __APPLE__
#include "process_mac.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h> //for getuid()
#endif

//read a line from stdin, while supporting history on posix
string read_line(const string& prompt = "> "){
	string line;
#ifndef __APPLE__
	printf("%s",prompt.c_str()); fflush(stdout);
	while(1){
		char c = fgetc(stdin);
		if(c=='\n') break;
		line += c;
	}
	return line;
#else
	char* p = readline(prompt.c_str());
	if(p){
		line = p;
		add_history(p);
		free(p);
	}
#endif
	return line;
}

vector<size_t> addrs;

int main(int argc, char** argv){

#ifdef __APPLE__
	if(getuid()!=0){
		printf("You need to run this as root (using sudo).\n");
		return 1;
	}
#endif
	
	int pid = 0;
	printf("Type 'find [part of name]' to find a process.\n");
	printf("Type 'help' for help.\n");
	while(1){
		string line = read_line();
		if(line.size()<1) continue;
		int i = line.find(" ");
		string cmd = line.substr(0,i);
		string arg = line.substr(i+1);
		
		if(cmd=="find"){
			pid = find_process(arg);
			if(!pid) printf("No matching process found.\n");
			else printf("pid = %d\n",pid);
			continue;
		}
		
		if(cmd=="help"){
			printf("find [part] => find process with [part] in name\n");
			printf("fc [value] => find char value\n");
			printf("fs [value] => find short value\n");
			printf("fi [value] => find int value\n"); 
			printf("ff [value] => find float value\n");
			printf("fd [value] => find double value\n");
			printf("reset => reset search\n");
			printf("print => print addresses found\n");
			printf("wc [value] => write char value\n");
			printf("ws [value] => write short value\n");
			printf("wi [value] => write int value\n");
			printf("wf [value] => write float value\n");
			printf("wd [value] => write double value\n");
			printf("\n");
			continue;
		}
		
		if(pid==0){
			printf("No pid. Type 'find [part of name]' to find a process.\n");
			continue;
		}
		
		int ibuf;
		short sbuf;
		char cbuf;
		float fbuf;
		double dbuf;
		string s;
		void* data;
		int dlen;
		if(arg.size()>0){
			switch(cmd[1]){
				case 'c': cbuf = atoi(arg.c_str()); data = &cbuf; dlen = 1; break;
				case 's': sbuf = atoi(arg.c_str()); data = &sbuf; dlen = 2; break;
				case 'i': ibuf = atoi(arg.c_str()); data = &ibuf; dlen = 4; break;
				case 'f': fbuf = atof(arg.c_str()); data = &fbuf; dlen = 4; break;
				case 'd': dbuf = atof(arg.c_str()); data = &dbuf; dlen = 8; break;
				default: break;
			}
			switch(cmd[0]){
				case 'f':{
					printf("Looking for: 0x");
					int n;
					for(n=0;n<dlen;n++) printf("%02x",((unsigned char*)data)[n]);
					printf("...\n");
					
					if(addrs.empty()){
						scan_memory(pid,data,dlen,addrs);
						printf("%lu locations found\n",addrs.size());
					}
					else{
						vector<size_t> new_addrs;
						char* buf = new char[dlen];
						int n;
						for(n=0;n<addrs.size();n++){
							read_memory(pid,addrs[n],buf,dlen);
							if(memcmp(buf,data,dlen)==0){
								new_addrs.push_back(addrs[n]);
							}
						}
						delete [] buf;
						printf("%lu locations eliminated\n",addrs.size()-new_addrs.size());
						printf("%lu locations found\n",new_addrs.size());
						addrs = new_addrs;
					}
				} break;
				case 'w':{
					int n;
					for(n=0;n<addrs.size();n++){
						write_memory(pid,addrs[n],data,dlen);
					}
				} break;
				default: break;
			}
		}
		if(cmd=="reset"){
			addrs.clear();
		}
		if(cmd=="print"){
			int n;
			for(n=0;n<addrs.size();n++){
				printf("0x%08lX\n",addrs[n]);
			}
		}
	}
}
