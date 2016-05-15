
#include <stdlib.h>
#include <stdio.h>

#include <mach/mach.h>

#include <string>
#include <vector>
using namespace std;

int find_process(const string& part){
	string cmd = "pgrep -i "+part;
	FILE* p = popen(cmd.c_str(),"r");
	char buf[64] = {0};
	fread(buf,1,64,p);
	pclose(p);
	return atoi(buf);
}

bool scan_memory(int pid, void* bytes, int len, vector<size_t>& results){
	
	//get task from pid
	mach_port_t task;
	kern_return_t kr = task_for_pid(mach_task_self(),pid,&task);
	if(kr!=KERN_SUCCESS){
		printf("task_for_pid() error! %d: %s\n",kr,mach_error_string(kr));
		return false;
	}
	
	//for each memory region
	vm_address_t start = VM_MIN_ADDRESS;
	while(start<VM_MAX_ADDRESS){
		vm_size_t size;
		vm_region_flavor_t flavor = VM_REGION_BASIC_INFO;
		vm_region_basic_info_64 info;
		mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT_64;
		memory_object_name_t object_name;
		kr = vm_region_64(task,&start,&size,flavor,(vm_region_info_t)&info,&info_count,&object_name);
		if(kr!=KERN_SUCCESS) break;
		
		//for each chunk in region
		vm_address_t end = start+size;
		vm_address_t addr;
		for(addr=start;addr<end;addr+=4096){
			
			//zero our buffer so we know when we fail
			char buf[4096];
			memset(buf,0,4096);
			
			//read it
			vm_size_t count = 0;
			vm_read_overwrite(task,addr,4096,(vm_address_t)buf,&count);
			
			//scan for match
			size_t j;
			for(j=len;j<count;j++){
				if(memcmp(buf+j-len,bytes,len)==0){
					results.push_back(addr+j-len);
				}
			}
			
			//just in case
			if(count<4096) break;
		}
		start = end;
	}
	
	return true;
}

bool write_memory(int pid, size_t addr, void* bytes, int len){
	
	mach_port_t task;
	kern_return_t kr = task_for_pid(mach_task_self(),pid,&task);
	if(kr!=KERN_SUCCESS){
		printf("task_for_pid() error! %d: %s\n",kr,mach_error_string(kr));
		return false;
	}
	
	vm_write(task,addr,(vm_address_t)bytes,len);
	
	return true;
}

bool read_memory(int pid, size_t addr, void* bytes, int len){
	
	//zero buffer in case of failure
	memset(bytes,0,len);
	
	//get task from pid
	mach_port_t task;
	kern_return_t kr = task_for_pid(mach_task_self(),pid,&task);
	if(kr!=KERN_SUCCESS){
		printf("task_for_pid() error! %d: %s\n",kr,mach_error_string(kr));
		return false;
	}
	
	//read data
	vm_size_t count = 0;
	vm_read_overwrite(task,addr,len,(vm_address_t)bytes,&count);
	
	return true;
}
