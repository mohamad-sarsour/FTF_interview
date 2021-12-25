#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_
#include <sys/types.h>
#include <vector>
#include <time.h>
#include <csetjmp>
#include <fcntl.h>
#include <iostream>
#include <sys/wait.h>
#include <signal.h>
#include <memory>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define JOBS_MAX_AMOUNT (150)



using namespace std;
class JobsList;
class SmallShell;
typedef enum { finished = 0, stopped = 1, background = 2, empty = 3 }job_status;

class Command {
	// TODO: Add your data members
protected:
	// string entered_cmd;

public:
	string entered_cmd;
	bool stam;
	bool pipe_redir;
	bool optional_arg;
	bool built_in_cmd;
	bool background_cmd;
	string newlen;
	vector<string> args;
	JobsList* list;
	char* lastD;
	bool INVALID_ARG;
	bool Job_id_in;
	bool too_many_args;
	bool OLDPWD_null;
	bool listempty;

	Command(const char* cmd_line) :entered_cmd(cmd_line), stam(false), optional_arg(false), built_in_cmd(false), background_cmd(false) {};
	virtual ~Command() = default;
	virtual void execute() = 0;
	//virtual void prepare();
	//virtual void cleanup();
	// TODO: Add your extra methods if needed
	string& getNameCmd() {
		return entered_cmd;
	}
	void optArgExsite() {
		optional_arg = true;
	}
	void addArg(const string& s) {
		args.push_back(s);
	}
	int ArgsAmount() {
		return args.size();
	}
	bool is_built_in_cmd() {
		return built_in_cmd;
	}
};

class BuiltInCommand : public Command {
public:
	explicit BuiltInCommand(const char* cmd_line) :Command(cmd_line) {
		newlen = "smash";
		pipe_redir = false;
		lastD = nullptr;
		INVALID_ARG = false;
		Job_id_in = true;
		too_many_args = false;
		OLDPWD_null = false;
		background_cmd = false;
		listempty = false;
		optional_arg = false;
		built_in_cmd = true;
	}
	virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
public:
	ExternalCommand(const char* cmd_line);
	virtual ~ExternalCommand() = default;
	virtual void execute();


};

class PipeCommand : public Command {
	// TODO: Add your data members
public:
	PipeCommand(const char* cmd_line);
	virtual ~PipeCommand() = default;
	void execute() override;

};

class RedirectionCommand : public Command {
	// TODO: Add your data members
public:
	explicit RedirectionCommand(const char* cmd_line);
	virtual ~RedirectionCommand() {}
	void execute() override;
	//void prepare() override;
	//void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {


public:
	// TODO: Add your data members public:
	ChangeDirCommand(const char* cmd_line, char** plastPwd);
	~ChangeDirCommand() override {
		if(lastD){
		free(lastD);}
	
	}
	void execute() override;

protected:
	char** plastPwd;

};

class GetCurrDirCommand : public BuiltInCommand {
public:
	GetCurrDirCommand(const char* cmd_line);
	//virtual ~GetCurrDirCommand() {}
	void execute() override;


};

class ShowPidCommand : public BuiltInCommand {
public:
	ShowPidCommand(const char* cmd_line);
	virtual ~ShowPidCommand() {}
	void execute() override;
};


class QuitCommand : public BuiltInCommand {
	// TODO: Add your data members
	void execute() override;
	//just let it public man
public:
	virtual ~QuitCommand() {}
	QuitCommand(const char* cmd_line, JobsList* jobs);

};


class JobsList {

public:
	class JobEntry {
		// TODO: Add your data members
	public:
		job_status j_status;
		int j_id;
		pid_t p_id;
		shared_ptr<Command> cmd;
		time_t time;

		JobEntry() :j_status(empty), j_id(0), p_id(0), time(0) {};

	};
	vector<JobEntry> arr;
	vector<JobEntry> non_built_in_cmd;
	vector<JobEntry> stoppedd;
	// TODO: Add your data members
public:
	//vector<JobEntry> stoppedd;
	JobsList() :arr(JOBS_MAX_AMOUNT), non_built_in_cmd(JOBS_MAX_AMOUNT), stoppedd(JOBS_MAX_AMOUNT) {
		// non_built_in_cmd.pop_back();
	}
	~JobsList() = default;
	void addJob(shared_ptr<Command> cmd, bool isStopped = false) {
		//  removeFinishedJobs();
		vector<JobEntry>& list = cmd->is_built_in_cmd() ? arr : non_built_in_cmd;
		job_status stat = isStopped ? stopped : background;
		JobEntry& tmp = get_empty_place_in_list(list);
		tmp.j_status = stat;
		//   tmp.cmd=new Command(cmd->entered_cmd.c_str());
		//  tmp.cmd->entered_cmd = cmd->getNameCmd();
		tmp.cmd = cmd;
		tmp.time = time(nullptr);
		tmp.j_id = get_max_job_id() + 1;
		if (stat == stopped) {
			JobsList::JobEntry new_j = JobEntry();
			new_j.j_id = tmp.j_id;
			new_j.p_id = tmp.p_id;
			new_j.cmd = tmp.cmd;
			new_j.time = tmp.time;
			new_j.j_status = stopped;
			stoppedd.push_back(new_j);
		}
	}
	void set_job_pid(int job_id, pid_t pid, string en_cmd)
	{
		JobEntry* job = getJobById(job_id);
		job->p_id = pid;
		job->cmd->entered_cmd = en_cmd;

		JobEntry* job1 = getJobById_SS(job_id);
		job1->p_id = pid;
		job1->cmd->entered_cmd = en_cmd;


	}

	bool is_job_id_in(int job_id) {
		for (JobEntry& job : non_built_in_cmd) {
			if (job.j_id == job_id) {
				return true;

			}
		}
		return false;
	}

	bool is_job_pid_in(int job_pid) {
		for (JobEntry& job : non_built_in_cmd) {
			if (job.p_id == job_pid) {
				return true;

			}
		}
		return false;
	}

	bool is_list_empty() {
		int i = 0;
		for (JobEntry& job : non_built_in_cmd) {
			if (job.j_status != finished && job.j_status != empty) {
				i++;
			}

		}

		if (i == 0) {
			return true;
		}
		return false;
	}
	void send_SIG_TO_JOB(int job_id, int SIGNUM) {
		JobsList::JobEntry* job = getJobById(job_id);
		if (!job) {
			return;
		}
		if (SIGNUM == 19 || SIGNUM == 20) {
			job->j_status = stopped;
			JobsList::JobEntry new_j = JobEntry();
			new_j.j_id = job->j_id;
			new_j.p_id = job->p_id;
			new_j.cmd = job->cmd;
			new_j.time = job->time;
			new_j.j_status = stopped;
			stoppedd.push_back(new_j);
		}
		if (SIGNUM == SIGCONT) {
			for (JobEntry& jobb : stoppedd) {
				if (jobb.j_status != empty && job->j_id == jobb.j_id) {
					jobb.j_status = empty;
					jobb.j_id = 0;
					// delete jobb.cmd;
					 //jobb.cmd = nullptr;
				}
			}
			job->j_status = background;
		}

		if (SIGNUM == SIGKILL) {
			for (JobEntry& jobb : stoppedd) {
				if (jobb.j_status != empty && job->j_id == jobb.j_id) {
					jobb.j_status = empty;
					jobb.j_id = 0;
					// delete jobb.cmd;
					 // jobb.cmd = nullptr;
				}
			}
			job->j_status = finished;

		}


		if (kill(job->p_id, SIGNUM) == -1) {

			perror("smash error: kill failed");

		}


	}
	void printJobsList() {
		removeFinishedJobs();
		for (int i = 1; i < JOBS_MAX_AMOUNT * 2; i++) {
			JobEntry* job = getJobById(i);
			if (job) {
				if (job->j_status != finished && job->j_status != empty) {
					std::cout << "[" << job->j_id << "] " << job->cmd->getNameCmd() << " : " << job->p_id << " " << (difftime(time(nullptr), job->time)) << " secs";
					if (job->j_status == stopped) {
						std::cout << " (stopped)";
					}
					std::cout << '\n';

				}
			}
		}

	}
	

	void killAllJobs(bool kill_with_print) {
		if (kill_with_print) {
			int counter = 0;

			for (JobEntry& job : non_built_in_cmd) {
				if (job.j_status != finished && job.j_status != empty) {
					counter++;
				}
			}
			cout << "smash: sending SIGKILL signal to " << counter << " jobs:" << endl;

			for (JobEntry& job : non_built_in_cmd) {
				if (job.j_status != finished && job.j_status != empty) {
					cout << job.p_id << ": " << job.cmd->getNameCmd() << endl;
				}
			}
		}

		for (JobEntry& job : non_built_in_cmd) {
			if (job.j_status != finished && job.j_status != empty) {
				int ii = kill(job.p_id, SIGKILL);

				if (ii == -1) {
					perror("smash error: kill failed");
				}

				job.j_status = empty;
				job.j_id = 0;
				job.p_id = 0;
				if (job.cmd) {
					// delete job.cmd;
				}
				// job.cmd = nullptr;
			}
		}
	}


void printstoppedlist(){
	for (JobEntry& job : stoppedd) {
		
		if(job.j_status!=empty){
							std::cout << "[" << job.j_id << "]" << job.cmd->getNameCmd() << " : " << job.p_id;
	std::cout << '\n';
	}
	
		}
		
	
	}
	void removeFinishedJobs() {

		for (JobEntry& job : non_built_in_cmd) {
			if (job.j_status != empty && job.j_status != stopped) {
				int stat;
				if (waitpid(job.p_id, &stat, WNOHANG) != 0) {


					job.j_status = finished;
				}

			}
		}

		for (JobEntry& job : non_built_in_cmd) {
			if (job.j_status == finished) {
				removeJobById(job.j_id);
			}
		}
	}
     void SetNewId( int jobId) {
		 JobEntry* job=getJobById(jobId);
		// if(job->j_id < get_max_job_id()){
		// job->j_id = get_max_job_id() + 1;}
		 job->time = time(nullptr);
		 for(JobEntry& jobb : stoppedd){
			 if(jobb.j_id==jobId){
				// jobb.j_id=job->j_id;
				 jobb.time=job->time;
				 }
			 }
		 }

	JobEntry* getJobById(int jobId) {
		for (JobEntry& job : non_built_in_cmd) {
			if (job.j_id == jobId) {
				return &job;
			}
		}
		return nullptr;
	}
	JobEntry* getJobById_SS(int jobId) {
		for (JobEntry& job : stoppedd) {
			if (job.j_id == jobId) {
				return &job;
			}
		}
		return nullptr;
	}

	void removeJobById(int jobId) {

		for (JobEntry& job : non_built_in_cmd) {
			if (job.j_id == jobId) {
				job.j_status = empty;
				job.j_id = 0;
				//  delete job.cmd;
				   //job.cmd = nullptr;
			}
		}
			for (JobEntry& job : stoppedd) {
			if (job.j_id == jobId) {
				job.j_status = empty;
				job.j_id = 0;
				job.p_id=0;
				//  delete job.cmd;
				   //job.cmd = nullptr;
			}
		}
	}

	JobEntry* getLastJob(int* lastJobId);

	JobEntry* getLastStoppedJob(int* jobId) {
		int max = 0;
		for (JobEntry& jobb : stoppedd) {

			if (jobb.j_id > max&& jobb.j_status != empty) {
				max = jobb.j_id;
			}

		}

		if (max == 0) {
			*jobId = -1;
			return nullptr;
		}
		for (JobEntry& jobb : stoppedd) {

			if (jobb.j_id == max) {
				*jobId = max;
				return &jobb;
			}

		}
		return nullptr;
	}
	// TODO: Add extra methods or modify exisitng ones as needed

	JobEntry& get_empty_place_in_list(vector<JobEntry>& list) {
		for (JobEntry& i : list) {
			if (i.j_status == empty) {
				return i;
			}
		}
		//throw std::exception("no enough space in JobList !");
		return list[0];
	}

	int get_max_job_id() {
		int result = 0;
		for (JobEntry& i : non_built_in_cmd) {
			if (i.j_status != empty) {
				result = result > i.j_id ? result : i.j_id;
			}
		}
		return result;
	}


	void print_job(int job_id) {
		JobsList::JobEntry* job = getJobById(job_id);
		if (job) {
			cout << job->cmd->getNameCmd() << " : " << job->p_id << "\n";
		}
	}


	void    print_non_built_in_cmds()
	{
		for (JobEntry& job : non_built_in_cmd) {
			if (job.j_status == finished || job.j_status == empty) {
				continue;
			}
			std::cout << "[" << job.j_id << "]" << job.cmd->getNameCmd() << " : " << (difftime(time(nullptr), job.time)) << " secs";
			if (job.j_status == stopped) {
				std::cout << " (stopped)";
			}
			std::cout << '\n';
		}
	}
};

class JobsCommand : public BuiltInCommand {
	// TODO: Add your data members
public:
	JobsCommand(const char* cmd_line, JobsList* jobs);
	virtual ~JobsCommand() {}
	void execute() override;
};

class KillCommand : public BuiltInCommand {
	// TODO: Add your data members
public:
	KillCommand(const char* cmd_line, JobsList* jobs);
	virtual ~KillCommand() {}
	void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
	// TODO: Add your data members
public:
	ForegroundCommand(const char* cmd_line, JobsList* jobs);
	virtual ~ForegroundCommand() {}
	void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
	// TODO: Add your data members
public:
	BackgroundCommand(const char* cmd_line, JobsList* jobs);
	virtual ~BackgroundCommand() {}
	void execute() override;
};

class CatCommand : public BuiltInCommand {
public:
	CatCommand(const char* cmd_line);
	virtual ~CatCommand() {}
	void execute() override;
};
class ChpromptCommand : public BuiltInCommand {
public:
	ChpromptCommand(const char* cmd_line, string newlen);
	virtual ~ChpromptCommand() {}
	void execute() override;
};

class SmallShell {
private:
	// TODO: Add your data members


	SmallShell();
public:
	JobsList* jobs;
	string newlen;
	pid_t for_pid;
	shared_ptr<Command> curr_cmd;
	shared_ptr<Command> for_cmd;
	/* void MakeLastJobDONE(){
		 JobsList::JobEntry* job=  jobs->getJobById(jobs->get_max_job_id());
		 job->j_status=job_status::finished;
		 cout<<"smash: process "<<job->p_id<<" was killed\n";
	 kill(job->p_id,SIGKILL);
	 }
		void MakeLastJobStop(){
		 JobsList::JobEntry* job=  jobs->getJobById(jobs->get_max_job_id());
		 job->j_status=job_status::stopped;
		 cout<<"smash: process "<<job->p_id<<" was stopped\n";
		 jobs->send_SIG_TO_JOB(job->j_id,SIGTSTP);

	 }*/
	string LASTD;

	bool  is_built_job_in_shell(int job_id) {
		return jobs->is_job_id_in(job_id);
	}
	bool is_Jobs_empty() {
		return jobs->is_list_empty();

	}
	JobsList* retListP() {
		return jobs;
	}

	Command* CreateCommand(const char* cmd_line);
	SmallShell(SmallShell const&) = delete; // disable copy ctor
	void operator=(SmallShell const&) = delete; // disable = operator
	static SmallShell& getInstance() // make SmallShell singleton
	{
		static SmallShell instance;

		// Guaranteed to be destroyed.
	// Instantiated on first use.
		return instance;
	}
	~SmallShell();
	int executeCommand(const char* cmd_line);
	// TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
