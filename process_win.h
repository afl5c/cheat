
#include <windows.h>
#include <psapi.h>
#pragma comment(lib,"psapi.lib")
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

string process_name(int pid){
	HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pid);
	if(!h) return "";
	
	char buf[MAX_PATH] = {0};
	DWORD len = MAX_PATH;
	GetModuleFileNameEx(h,0,buf,len);
	CloseHandle(h);
	return buf;
}

int find_process(const string& _part){
	string part = _part;
	transform(part.begin(),part.end(),part.begin(),tolower);

	DWORD pids[4096];
	DWORD len;
	EnumProcesses(pids,sizeof(pids),&len);
	
	len /= sizeof(DWORD);
	int n;
	for(n=0;n<len;n++){
		string name = process_name(pids[n]);
		transform(name.begin(),name.end(),name.begin(),tolower);
		if(name.find(part)!=string::npos){
			return pids[n];
		}
	}
	return 0;
}

bool scan_memory(int pid, void* bytes, int len, vector<size_t>& results){
	
	//open process for reading
	HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pid);
	if(!h){
		printf("can't open!\n");
		return false;
	}
	
	//get min/max process addresses
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	//printf("min/max = %x/%x\n",si.lpMinimumApplicationAddress,si.lpMaximumApplicationAddress);
	
	//read mem
	SIZE_T n;
	MEMORY_BASIC_INFORMATION mbi;
	for(n=(SIZE_T)si.lpMinimumApplicationAddress;
		n<(SIZE_T)si.lpMaximumApplicationAddress;
		n=(SIZE_T)mbi.BaseAddress+mbi.RegionSize){
		
		//query region info
		VirtualQueryEx(h,(LPVOID)n,&mbi,sizeof(mbi));
		
		//read in memory one page at a time
		SIZE_T i;
		for(i=(SIZE_T)mbi.BaseAddress;i<(SIZE_T)mbi.BaseAddress+mbi.RegionSize;i+=4096){
		
			//zero our buffer so we know when we fail
			char buf[4096];
			memset(buf,0,4096);
			
			//unprotect the memory
			DWORD prev;
			VirtualProtectEx(h,(LPVOID)i,4096,PAGE_EXECUTE_READ,&prev);
			
			//read it
			SIZE_T count;
			ReadProcessMemory(h,(LPVOID)i,(void*)buf,4096,&count);
			
			//put back protection
			VirtualProtectEx(h,(LPVOID)i,4096,prev,0);
			
			//look for a match, shifting over one *byte* at a time
			SIZE_T j;
			for(j=len;j<count;j++){
				if(memcmp(buf+j-len,bytes,len)==0){
					results.push_back(i+j-len);
				}
			}
			
			//just in case
			if(count<4096){
				break;
			}
		}
	}
	
	CloseHandle(h);
	return true;
}

bool write_memory(int pid, SIZE_T addr, void* bytes, int len){
	
	//open process for writing
	HANDLE h = OpenProcess(PROCESS_VM_WRITE|PROCESS_VM_OPERATION,FALSE,pid);
	if(!h){
		printf("can't open!\n");
		return false;
	}
	
	//unprotect the memory
	DWORD prev;
	BOOL b = VirtualProtectEx(h,(LPVOID)addr,len,PAGE_READWRITE,&prev);
	if(!b){
		printf("can't unprotect! %d\n",GetLastError());
		return false;
	}
	//^check for failure because sometimes we can crash the process if we proceed
	
	//write it
	SIZE_T count;
	if(!WriteProcessMemory(h,(LPVOID)addr,bytes,len,&count)){
		printf("can't write! %d\n",GetLastError());
	}
	
	//put back protection
	VirtualProtectEx(h,(LPVOID)addr,len,prev,0);
	
	CloseHandle(h);
	return true;
}

bool read_memory(int pid, SIZE_T addr, void* bytes, int len){

	//zero buffer in case of failure
	memset(bytes,0,len);
	
	//open process for reading
	HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pid);
	if(!h){
		printf("can't open!\n");
		return false;
	}
	
	//unprotect the memory
	DWORD prev;
	VirtualProtectEx(h,(LPVOID)addr,len,PAGE_EXECUTE_READ,&prev);
	//^don't check for failure, because even on failure we can still read
	
	//read it
	SIZE_T count;
	if(!ReadProcessMemory(h,(LPVOID)addr,bytes,len,&count)){
		printf("can't read! %d\n",GetLastError());
	}
	
	//put back protection
	VirtualProtectEx(h,(LPVOID)addr,len,prev,0);
	
	CloseHandle(h);
	return true;
}








