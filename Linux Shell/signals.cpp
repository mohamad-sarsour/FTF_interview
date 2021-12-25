#include <iostream>
#include <signal.h>
#include <csetjmp>

#include "signals.h"
#include "Commands.h"

extern jmp_buf env;
using namespace std;
extern pid_t smashpid;
void ctrlZHandler(int sig_num) {
cout<< "smash: got ctrl-Z\n";
  SmallShell& smash = SmallShell::getInstance();
     if(smash.for_pid==-1){
		 return;}
		 
 
  
     int ii= kill(smash.for_pid,SIGSTOP);
     if(ii==-1){
		 perror("smash error: kill failed");
		 }
		 if(!smash.jobs->is_job_pid_in(smash.for_pid)){
		 smash.jobs->addJob(smash.for_cmd,true);
		
		 //cout<<smash.jobs->get_max_job_id()<<endl;
 smash.jobs->set_job_pid( smash.jobs->get_max_job_id() ,smash.for_pid,smash.for_cmd->getNameCmd());
 }
 else{
	  for (JobsList::JobEntry& job : smash.jobs->non_built_in_cmd) {
            if(job.p_id==smash.for_pid){
                
                job.j_status=stopped;
                
                JobsList::JobEntry new_j= JobsList::JobEntry();
new_j.j_id=job.j_id;
 new_j.p_id=job.p_id;
  new_j.cmd=job.cmd;
      new_j.time=job.time; 
 new_j.j_status=stopped;
       smash.jobs->stoppedd.push_back(new_j);
                
                
                break;

            }
	 }
	 }
        cout<<"smash: process "<<smash.for_pid<<" was stopped\n";
      
  smash.for_pid=-1;

}

void ctrlCHandler(int sig_num) {
cout<< "smash: got ctrl-C\n";
   SmallShell& smash = SmallShell::getInstance();
     if(smash.for_pid==-1){
		 return;}
  //int job_id=smash.jobs->get_max_job_id()+1;
    // JobsList::JobEntry* job=  smash.jobs->getJobById(smash.jobs->get_max_job_id());
        //job->j_status=job_status::finished;
     int ii= kill(smash.for_pid,SIGKILL);
     if(ii==-1){
		 perror("smash error: kill failed");
		 }
      for(JobsList::JobEntry& job : smash.jobs->non_built_in_cmd){
		   if(job.p_id==smash.for_pid){
                
                job.j_status=finished;
		  
	  }
		  
		  }
        cout<<"smash: process "<<smash.for_pid<<" was killed\n";
   
smash.for_pid=-1;
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

