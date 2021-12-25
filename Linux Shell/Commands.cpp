#include <unistd.h>
#include <utility>
#include <sstream>
#include <iomanip>
#include "Commands.h"
#include <vector>
#include <string.h>
#include <algorithm>

#include <errno.h>
#include <cerrno>
#include <clocale>
#include <cstring>

using namespace std;
#define WHITESPACE " "

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else

#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

bool isNumber(const string& str) {
	for (char const& c : str) {
		if (std::isdigit(c) == 0)return false;

	}
	return true;

}
string _ltrim(const std::string& s)
{
	size_t start = s.find_first_not_of(WHITESPACE);
	return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
	size_t end = s.find_last_not_of(WHITESPACE);
	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
	return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
	FUNC_ENTRY()
		int i = 0;
	std::istringstream iss(_trim(string(cmd_line)).c_str());
	for (std::string s; iss >> s; ) {
		args[i] = (char*)malloc(s.length() + 1);
		memset(args[i], 0, s.length() + 1);
		strcpy(args[i], s.c_str());
		args[++i] = nullptr;
	}
	return i;

	FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
	const string str(cmd_line);
	return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
	const string str(cmd_line);
	// find last character other than spaces
	unsigned int idx = str.find_last_not_of(WHITESPACE);
	// if all characters are spaces then return
	if (idx == string::npos) {
		return;
	}
	// if the command line does not end with & then return
	if (cmd_line[idx] != '&') {
		return;
	}
	// replace the & (background sign) with space and then remove all tailing spaces.
	cmd_line[idx] = ' ';
	// truncate the command line string up to the last non-space character
	cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

vector<string> _parseCat(const char* cmd_line) {
	vector<string> args;
	std::string s = cmd_line;
	std::string delimiter = " ";

	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		token.erase(remove(token.begin(), token.end(), ' '), token.end());
		if (!token.empty()) {
			args.push_back(token);
		}

		s.erase(0, pos + delimiter.length());
	}
	args.push_back(s);

	for (string& str : args) {
		str.erase(remove(str.begin(), str.end(), ' '), str.end());
	}

	return args;
}

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell() {
	this->jobs = new JobsList();
	this->newlen = "smash";
	this->LASTD = "";
	for_pid = -1;
	for_cmd = nullptr;
}

SmallShell::~SmallShell() {

	delete jobs;
	//delete for_cmd;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command* SmallShell::CreateCommand(const char* cmd_line) {
	// For example:
	char** args = new char* [COMMAND_MAX_ARGS]; // you should suppose this MACRO
	for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
		args[i] = nullptr;
	}
	_parseCommandLine(cmd_line, args);
	string firstWord = args[0];
	//Command * ccmd;
	for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
		if (args[i]) {
			if (strcmp(args[i], "|") == 0 || strcmp(args[i], "|&") == 0) {
				//_removeBackgroundSign(cmd_line);
				Command* ccmd = new PipeCommand(cmd_line);
				ccmd->pipe_redir = true;
				for (int j = 0; j < COMMAND_MAX_ARGS; j++) {
					if (args[j]) {
						ccmd->optional_arg = true;
						ccmd->addArg(args[j]);
					}
				}
				for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
					if (args[i]) {
						free(args[i]);
					}
				}
				delete[] args;
				return ccmd;
			}
		}
	}


	string cmd_lin = cmd_line;
	bool with_append = cmd_lin.find(">>") != string::npos ? true : false;
	bool with_append1 = cmd_lin.find(">") != string::npos ? true : false;
	if (with_append || with_append1) {
		Command* cmd = new RedirectionCommand(cmd_line);
		cmd->pipe_redir = true;
		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				free(args[i]);
			}
		}
		delete[] args;
		return cmd;
	}

	bool with_pipe = cmd_lin.find("|") != string::npos ? true : false;
	bool with_pipeAmper = cmd_lin.find("|&") != string::npos ? true : false;
	if (with_pipe || with_pipeAmper) {
		//_removeBackgroundSign(cmd_line);
		Command* cmd = new PipeCommand(cmd_line);
		cmd->pipe_redir = true;
		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				free(args[i]);
			}
		}
		delete[] args;
		return cmd;
	}

	firstWord.erase(remove(firstWord.begin(), firstWord.end(), '&'), firstWord.end());

	if (firstWord == "pwd") {
		Command* cmd = new GetCurrDirCommand(cmd_line);
		//here optional_arg is true because we are not asked to handle it in errors
		cmd->addArg(args[0]);
		cmd->built_in_cmd = true;
		cmd->optional_arg = false;

		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				free(args[i]);
			}
		}
		delete[] args;
		return cmd;

	}
	else if (firstWord == "cd") {
                  SmallShell& smash=getInstance();
		string SS = smash.LASTD;
		char* cstr = (char*)malloc(smash.LASTD.length() + 1);
		//shared_ptr<char> cstr(new char[LASTD.length() + 1]);
		
		char* ll[1] = { 0 };
		if (!SS.empty()) {
			strcpy(cstr, smash.LASTD.c_str());
			string dd = cstr;
			ll[0] = cstr;
		}
if(SS.empty()){
		
		*ll = nullptr; 
	
	}
		Command* cmd = new ChangeDirCommand(cmd_line, ll);

		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				if (i > 0) {
					cmd->optional_arg = true;
				}
				if (i > 1) {
					cmd->too_many_args = true;
					break;
				}
				cmd->addArg(args[i]);
			}
		}
		cmd->built_in_cmd = true;
		if (args[1]) {
			if (smash.LASTD.empty() && strcmp(args[1], "-") == 0) {
				cmd->OLDPWD_null = true;
				//cout<<"dd\n";
				for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
					if (args[i]) {
						free(args[i]);
					}
				}
				delete[] args;
				free(cstr);
				return cmd;
			}
		}

		char* last = get_current_dir_name();
		if (args[1]) {
			if (!smash.LASTD.empty() && strcmp(args[1], "-") == 0) {
				smash.LASTD = last;
				string c = ll[0];
				for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
					if (args[i]) {
						free(args[i]);
					}
				}
				delete[] args;
				free(cstr);
				free(last);
				return cmd;
			}

			if (strcmp(args[1], "-") != 0) {
				if (chdir(args[1]) == 0) {
					chdir(last);
					smash.LASTD = last;
				}
			}
		}
        
		free(last);

		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				free(args[i]);
			}
		}
		delete[] args;
		free(cstr);
		return cmd;
	}
	else if (firstWord == "chprompt") {
		if (args[1]) {
			string newlen1 = args[1];
			newlen1.erase(remove(newlen1.begin(), newlen1.end(), '&'), newlen1.end());
			Command* cmd = new ChpromptCommand(cmd_line, newlen1);
			cmd->built_in_cmd = true;
			newlen = newlen1;
			cmd->addArg(args[0]);
			for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
				if (args[i]) {
					free(args[i]);
				}
			}
			delete[] args;
			return cmd;
		}
		newlen = "smash";
		Command* cmd = new ChpromptCommand(cmd_line, "smash");
		cmd->built_in_cmd = true;
		cmd->addArg(args[0]);
		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				free(args[i]);
			}
		}
		delete[] args;
		return cmd;

	}
	else if (firstWord == "showpid") {
		//Command * cmd;

		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				if (strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) {
					Command* cmd = new RedirectionCommand(cmd_line);
					cmd->pipe_redir = true;
					for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
						if (args[i]) {
							if (i > 0) {
								cmd->optional_arg = true;
							}

							cmd->addArg(args[i]);
						}
					}
					for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
						if (args[i]) {
							free(args[i]);
						}
					}
					delete[] args;
					return cmd;
				}


				if (strcmp(args[i], "|") == 0 || strcmp(args[i], "|&") == 0) {
					//_removeBackgroundSign(cmd_line);
					Command* cmd = new PipeCommand(cmd_line);
					for (int j = 0; j < COMMAND_MAX_ARGS; j++) {
						if (args[j]) {
							if (j > 0) {
								cmd->optional_arg = true;
							}

							cmd->addArg(args[j]);
						}
					}

					cmd->addArg(args[0]);
					for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
						if (args[i]) {
							free(args[i]);
						}
					}
					delete[] args;
					return cmd;
				}
			}

		}
		Command* cmd = new ShowPidCommand(cmd_line);
		//here optional_arg is true because we are not asked to handle it in errors
		cmd->built_in_cmd = true;
		cmd->optional_arg = false;
		cmd->addArg(args[0]);
		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				free(args[i]);
			}
		}
		delete[] args;
		return cmd;

	}
	else if (firstWord == "jobs") {
		//Command * cmd;

		Command* cmd = new JobsCommand(cmd_line, jobs);
		//here optional_arg is true because we are not asked to handle it in errors
		cmd->built_in_cmd = true;
		cmd->optional_arg = false;
		cmd->addArg(args[0]);
		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				free(args[i]);
			}
		}
		delete[] args;
		return cmd;
	}
	else if (firstWord == "kill") {
		Command* cmd = new KillCommand(cmd_line, jobs);
		cmd->built_in_cmd = true;
		cmd->addArg(args[0]);
		if (args[1]) {
			if (args[1][0] != '-') {
				cmd->INVALID_ARG = true;
				for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
					if (args[i]) {
						free(args[i]);
					}
				}
				delete[] args;
				return cmd;
			}
		}
		if (!args[1] || !args[2]) {
			cmd->INVALID_ARG = true;
			for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
				if (args[i]) {
					free(args[i]);
				}
			}
			delete[] args;
			return cmd;

		}
		string str = args[2];
		str.erase(remove(str.begin(), str.end(), '&'), str.end());
		if (!isNumber(str)) {
			cmd->INVALID_ARG = true;
			for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
				if (args[i]) {
					free(args[i]);
				}
			}
			delete[] args;
			return cmd;
		}
		string token = args[1];
		token.erase(remove(token.begin(), token.end(), '-'), token.end());
		if (!isNumber(token)) {
			cmd->INVALID_ARG = true;
			for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
				if (args[i]) {
					free(args[i]);
				}
			}
			delete[] args;
			return cmd;
		}

		if (!is_built_job_in_shell(strtol(args[2], nullptr, 10)))
		{
			cmd->Job_id_in = false;
		}
		for (int i = 1; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				if (i > 0) {
					cmd->optional_arg = true;
				}
				if (i > 2&&strcmp(args[i],"&")!=0) {
					cmd->INVALID_ARG = true;
				}
				cmd->addArg(args[i]);
			}
		}
		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				free(args[i]);
			}
		}
		delete[] args;
		return cmd;
	}
	else if (firstWord == "fg") {
		Command* cmd = new  ForegroundCommand(cmd_line, jobs);
		cmd->built_in_cmd = true;
		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				if (i > 0) {
					cmd->optional_arg = true;
				}

				if (i >= 2) {
					cmd->INVALID_ARG = true;
					for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
						if (args[i]) {
							free(args[i]);
						}
					}
					delete[] args;
					return cmd;
					break;
				}
				cmd->addArg(args[i]);
			}
		}
		if (args[1]) {
			string str = args[1];
			str.erase(remove(str.begin(), str.end(), '&'), str.end());

			if (!isNumber(str)) {
				cmd->INVALID_ARG = true;
				for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
					if (args[i]) {
						free(args[i]);
					}
				}
				delete[] args;
				return cmd;
			}
			if (!is_built_job_in_shell(strtol(str.c_str(), nullptr, 10)))
			{
				cmd->Job_id_in = false;
			}
		}
		if (!cmd->optional_arg && is_Jobs_empty()) {
			cmd->listempty = true;
		}
		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				free(args[i]);
			}
		}
		delete[] args;
		return cmd;

	}
	else if (firstWord == "bg") {
		Command* cmd = new BackgroundCommand(cmd_line, jobs);
		cmd->built_in_cmd = true;
		cmd->optional_arg = false;
		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				if (i > 0) {
					cmd->optional_arg = true;
				}

				if (i >= 2) {
					cmd->INVALID_ARG = true;
					for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
						if (args[i]) {
							free(args[i]);
						}
					}
					delete[] args;
					return cmd;
					break;
				}
				cmd->addArg(args[i]);
			}
		}

		if (args[1]) {

			string str = args[1];
			str.erase(remove(str.begin(), str.end(), '&'), str.end());
			if(str.empty()){
				cmd->optional_arg=false;
				for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
					if (args[i]) {
						free(args[i]);
					}
				}
					delete[] args;
				return cmd;
				}
			if (!isNumber(str)) {
				cmd->INVALID_ARG = true;
				for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
					if (args[i]) {
						free(args[i]);
					}
				}
				delete[] args;
				return cmd;
			}

			if (!is_built_job_in_shell(strtol(str.c_str(), nullptr, 10)))
			{
				cmd->Job_id_in = false;
			}
		}
		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				free(args[i]);
			}
		}
		delete[] args;
		return cmd;

	}

	else if (firstWord == "quit") {
		Command* cmd = new QuitCommand(cmd_line, jobs);
		cmd->built_in_cmd = true;
		cmd->addArg(args[0]);
		if (args[1]) {
			string str = args[1];
			str.erase(remove(str.begin(), str.end(), '&'), str.end());

			if (str == "kill") {
				cmd->addArg(args[1]);
				cmd->optional_arg = true;
				for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
					if (args[i]) {
						free(args[i]);
					}
				}
				delete[] args;
				return cmd;
			}
		}


		cmd->optional_arg = false;
		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				free(args[i]);
			}
		}
		delete[] args;
		return cmd;

	}

	else if (firstWord == "cat") {
		bool is_there_args = false;
		Command* cmd = new CatCommand(cmd_line);
		cmd->args = _parseCat(cmd_line);
		
		for (unsigned int i = 0; i < cmd->args.size(); i++) {
			
			if (args[i]) {
	
				if(i>0){
				is_there_args = true;
			}
			}

		}

		if (is_there_args) {
			cmd->optional_arg = true;
		}
		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				free(args[i]);
			}
		}
		delete[] args;
		return cmd;

	} 

	else {
		Command* cmd = new ExternalCommand(cmd_line);
		cmd->built_in_cmd = false;
		cmd->pipe_redir = false;
		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				cmd->addArg(args[i]);
				if (i > 0) {
					cmd->optional_arg = true;
				}
			}

		}
		if (_isBackgroundComamnd(cmd_line)) {

			cmd->background_cmd = true;
		}
		for (int i = 0; i < COMMAND_MAX_ARGS; i++) {
			if (args[i]) {
				free(args[i]);
			}
		}
		delete[] args;
		return cmd;
	}

}
 

int SmallShell::executeCommand(const char* cmd_line) {
	// TODO: Add your implementation here

	//shared_ptr<Command> cmd(CreateCommand(cmd_line));
	curr_cmd.reset(CreateCommand(cmd_line));
	//cout << "right after CreatCommand reference count = " << curr_cmd.use_count() << endl;
	if (!curr_cmd->is_built_in_cmd() && curr_cmd->background_cmd) {
		jobs->addJob(curr_cmd);
	}
	// jobs.print_non_built_in_cmds();
	curr_cmd->list = retListP();
	//cout << "before cmd->exe reference count = " << curr_cmd.use_count() << endl;
	curr_cmd->execute();
	//cout << "after cmd->exe reference count = " << curr_cmd.use_count() << endl;
	for_cmd = nullptr;
	//cout << "after for_cmd=nullpter reference count = " << curr_cmd.use_count() << endl;
	for_pid = -1;

	if (curr_cmd->built_in_cmd) {
		string firstWord = curr_cmd->args[0];
		firstWord.erase(remove(firstWord.begin(), firstWord.end(), '&'), firstWord.end());

		if (firstWord == "quit") { 
			return 0;
		}
	}
	jobs->removeFinishedJobs();
	return 1;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

void ChangeDirCommand::execute() {
	if (!optional_arg) {
		perror("smash error: chdir failed");
		return;
	}
	if (!too_many_args) {
		char* last = get_current_dir_name();

		if (args[1] != "-") {
			string str = args[1];
			str.erase(remove(str.begin(), str.end(), '&'), str.end());
			if (chdir(str.c_str()) == 0) {
				free(last);

				return;
				//	LASTD=last;
			}
			else {
				perror("smash error: chdir failed");
			}
			free(last);
			return;
		}
		else {

			if (OLDPWD_null) {
				std::cerr << "smash error: cd: OLDPWD not set\n";
				free(last);
				return;
			}
			//	string currDIR=lastD;
			//	cout<<currDIR<<"\n";
			if(!lastD){
					perror("smash error: chdir failed");
				free(last);
			return;
				}
			chdir(lastD);

			//LASTD=last;
			free(last);
			return;
		}


	}
	if (OLDPWD_null && args[1] == "-" && !too_many_args) {
		std::cerr << "smash error: cd: OLDPWD not set\n";
		return;
	}



	if (too_many_args) {
		std::cerr << "smash error: cd: too many arguments\n";
	}


}



void GetCurrDirCommand::execute() {
	char* currDIR = get_current_dir_name();
	string string1 = string(currDIR);
	cout << string1 << "\n";

	free(currDIR);

}


void JobsCommand::execute() {
	list->printJobsList();
}

void KillCommand::execute() {

	if (INVALID_ARG) {
		std::cerr << "smash error: kill: invalid arguments\n";
		return;
	}
	string str = args[2];
	str.erase(remove(str.begin(), str.end(), '&'), str.end());

	int job_id = std::stoi(args[2], nullptr, 10);
	if (!this->Job_id_in) {
		std::cerr << "smash error: kill: job-id " << job_id << " does not exist\n";
		return;
	}
	string SIG = args[1];
	string token = SIG;
	token.erase(remove(token.begin(), token.end(), '-'), token.end());

	int SIGNUM = std::stoi(token, nullptr, 10);

	JobsList::JobEntry* job = list->getJobById(job_id);
	cout << "signal number " << SIGNUM << " was sent to pid " << job->p_id << "\n";
	list->send_SIG_TO_JOB(job_id, SIGNUM);

}

void QuitCommand::execute() {
	if (optional_arg) {
		string str = args[1];
		str.erase(remove(str.begin(), str.end(), '&'), str.end());
		if (str == "kill") {
			list->killAllJobs(true);

		}
	}

	//list->killAllJobs(false);

	//exit(1);
}


void ForegroundCommand::execute() {
	int job_id;
	if (this->listempty && !this->optional_arg && !this->INVALID_ARG) {
		std::cerr << "smash error: fg: jobs list is empty\n";
		return;
	}  
	if (!optional_arg && !this->INVALID_ARG) {
		int wstatus;
		job_id = list->get_max_job_id();
		pid_t pid = list->getJobById(job_id)->p_id;
		list->print_job(job_id);
		list->send_SIG_TO_JOB(job_id, SIGCONT);
		SmallShell& smash = SmallShell::getInstance();
		smash.for_pid = pid;
		
		smash.for_cmd.reset(new ExternalCommand(this->entered_cmd.c_str()));
	
		waitpid(pid, &wstatus, WUNTRACED);
		if(list->getJobById(job_id)){
	if (list->getJobById(job_id)->j_status != stopped) {
			list->removeJobById(job_id);
		}
if(list->getJobById(job_id)){
		if (list->getJobById(job_id)->j_status == stopped) {
		list->SetNewId(job_id);
	}}}
		//delete smash.for_cmd;
		//smash.for_cmd=nullptr;
		return;
	}



	if (this->INVALID_ARG) {
		std::cerr << "smash error: fg: invalid arguments\n";
		return;

	}
	string str = args[1];
	str.erase(remove(str.begin(), str.end(), '&'), str.end());
	job_id = std::stoi(str, nullptr, 10);
	if (!this->Job_id_in) {
		std::cerr << "smash error: fg: job-id " << job_id << " does not exist \n";
		return;
	}

	int wstatus;
	pid_t pid = list->getJobById(job_id)->p_id;
	list->print_job(job_id);
	list->send_SIG_TO_JOB(job_id,SIGCONT);
	SmallShell& smash = SmallShell::getInstance();
	smash.for_pid = pid;
	//delete smash.for_cmd;
	smash.for_cmd.reset(new ExternalCommand(this->entered_cmd.c_str()));
	//list->removeJobById(job_id);
	waitpid(pid, &wstatus, WUNTRACED);
	if(list->getJobById(job_id)){
	if (list->getJobById(job_id)->j_status != stopped) {
		
		list->removeJobById(job_id);
	}
	if(list->getJobById(job_id)){
	if (list->getJobById(job_id)->j_status == stopped) {
		list->SetNewId(job_id);
	}}}
	//delete smash.for_cmd;
	//smash.for_cmd=nullptr;

}


void BackgroundCommand::execute() {
	int job_id;
	if (!optional_arg && !this->INVALID_ARG) {

		JobsList::JobEntry* job = list->getLastStoppedJob(&job_id);
		if (!job) {
			std::cerr << "smash error: bg: there is no stopped jobs to resume\n";
			return;
		}

		list->print_job(job_id);
	
		list->send_SIG_TO_JOB(job_id, SIGCONT);
		JobsList::JobEntry* job1 = list->getJobById(job_id);
		job_status j = job_status::background;
		job1->j_status = j;
		return;
	}


	if (this->INVALID_ARG) {
		std::cerr << "smash error: bg: invalid arguments\n";
		return;
	}
	string str = args[1];
	str.erase(remove(str.begin(), str.end(), '&'), str.end());
	job_id = std::stoi(str, nullptr, 10);
	// JobsList::JobEntry * job1=list->getJobById(job_id);
	if (!this->Job_id_in) {
		std::cerr << "smash error: bg: job-id " << job_id << " does not exist\n";
		return;
	}

	JobsList::JobEntry* job = list->getJobById(job_id);
	if (!job) {
		return;
	}
	if (job->j_status == background) {
		std::cerr << "smash error: bg: job-id " << job_id << " is already running in the background\n";
		return;
	}

	list->send_SIG_TO_JOB(job_id, SIGCONT);
	list->print_job(job_id);
	job_status j = job_status::background;
	job->j_status = j;



}


void ShowPidCommand::execute() {
	pid_t pid = getpid();
	cout << "smash pid is " << pid << endl;


}
void CatCommand::execute() {
	if (!optional_arg) {
		std::cerr << "smash error: cat: not enough arguments\n";
	}

	for (unsigned int i = 1; i < args.size(); i++) {
		//cout<<args[i]<<endl;
		int fd = open(args[i].c_str(), O_RDONLY);
		if (fd == -1) {
		perror("smash error: open failed");
		
		return;
	}
	
		char c;
		int ret_val=read(fd, &c, 1);
		while (ret_val) {
			write(1, &c, 1);
			ret_val=read(fd, &c, 1);
		
		}
		
	}


}

void ChpromptCommand::execute() {

}
////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////constructors/////////////////////////////////////////////////////

JobsCommand::JobsCommand(const char* cmd_line, JobsList* jobs) :BuiltInCommand(cmd_line)
{
	list = jobs;

}
KillCommand::KillCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line) {

	list = jobs;

}
QuitCommand::QuitCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line) {

	list = jobs;

}


ForegroundCommand::ForegroundCommand(const char* cmd_line, JobsList* jobs) :BuiltInCommand(cmd_line) {


	list = jobs;

}
ChangeDirCommand::ChangeDirCommand(const char* cmd_line, char** plastPwd) :BuiltInCommand(cmd_line) {
	if(*plastPwd){
		lastD=(char*)malloc(strlen(*plastPwd) + 1);
strcpy(lastD,*plastPwd);}
	//lastD = *plastPwd;
	else{
		lastD=nullptr;
		}

}
BackgroundCommand::BackgroundCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line) {

	list = jobs;
}
GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}

ShowPidCommand::ShowPidCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}



CatCommand::CatCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}
ChpromptCommand::ChpromptCommand(const char* cmd_line, string newlen) : BuiltInCommand(cmd_line) {}

void ExternalCommand::execute() {
	//SmallShell& smash = SmallShell::getInstance();
	char* bash = const_cast<char*>("bash");
	char* flag = const_cast<char*>("-c");
	string token = entered_cmd;
	token.erase(remove(token.begin(), token.end(), '&'), token.end());
	char* argv[4] = { bash,flag,const_cast<char*>(token.c_str()),nullptr };

	pid_t pid = fork();
	setpgrp();


	if (pid == -1) {
		perror("smash error: fork failed");

		return;
	}
	else if (pid == 0) { // then we in child process

		if (execvp("/bin/bash", argv) < 0) {
			perror("smash error: execvp failed");
			this->stam = true;
		}

		exit(0);
	}
	else {// we in the parent process
		if (!background_cmd) {
			SmallShell& smash = SmallShell::getInstance();
			smash.for_pid = pid;

			smash.for_cmd = smash.curr_cmd;
			//smash.for_cmd.reset(this);
			//smash.for_cmd = shared_ptr<Command>(this);
			waitpid(pid, nullptr, WUNTRACED);// waiting for child to terminate
			smash.for_pid = -1;
			return;
		}

		list->getJobById(list->get_max_job_id())->p_id = pid;
		return;
	}
	// list->getJobById(list->get_max_job_id())->p_id = pid;
}

void RedirectionCommand::execute() {
	bool with_append = entered_cmd.find(">>") != string::npos ? true : false;
	string barrier = with_append ? ">>" : ">";
	string cmd = entered_cmd.substr(0, entered_cmd.find(barrier));
	auto index = !with_append ? entered_cmd.find(barrier) + 1 : entered_cmd.find(barrier) + 2;
	string outFile = entered_cmd.substr(index, entered_cmd.size());

	cmd = _ltrim(_rtrim(cmd));
	outFile = _ltrim(_rtrim(outFile));
	if (outFile.find("&") != string::npos) {
		outFile = outFile.erase(outFile.find_last_of("&"));
	}
	outFile = _ltrim(_rtrim(outFile));

	int fd = dup(1);
	close(1); // Release fd stdout
	int afd = 0;
	if (with_append) {
		afd = open(outFile.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
	}
	else {
		afd = open(outFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
	}
	if (afd == -1) {
		perror("smash error: open failed");
	}
	//cout<<"helloo\n";
	SmallShell& tmp = SmallShell::getInstance();
	shared_ptr<Command> comand(tmp.CreateCommand(cmd.c_str()));
	comand->execute();
	//   this->
//cout<<"helloo\n";
	close(1);
	dup(fd);
	close(fd);
}


PipeCommand::PipeCommand(const char* cmd_line) :Command(cmd_line) {}
RedirectionCommand::RedirectionCommand(const char* cmd_line) : Command(cmd_line) {}
ExternalCommand::ExternalCommand(const char* cmd_line) : Command(cmd_line) {}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void  PipeCommand::execute() {

	pid_t pip_pid = fork();

	if (pip_pid == 0) {
		setpgrp();
		int pipefd[2];
		pipe(pipefd);
		pid_t pid = fork();

		if (pid == 0) {// first command
			setpgrp();

			bool with_amperc = entered_cmd.find("|&") != string::npos ? true : false;
			string barrier = with_amperc ? "|&" : "|";
			string left_cmd = entered_cmd.substr(0, entered_cmd.find(barrier));
			auto index = !with_amperc ? entered_cmd.find(barrier) + 1 : entered_cmd.find(barrier) + 2;
			string right_cmd = entered_cmd.substr(index,entered_cmd.size());

			left_cmd = _ltrim(_rtrim(left_cmd));
			right_cmd = _ltrim(_rtrim(right_cmd));
			
			if (right_cmd.find("&") != string::npos) {
				right_cmd=right_cmd.erase(right_cmd.find_last_of("&"));
				right_cmd = _ltrim(_rtrim(right_cmd));
			}

			SmallShell& tmp = SmallShell::getInstance();
			shared_ptr<Command> cmd_left(tmp.CreateCommand(left_cmd.c_str()));

			if (!with_amperc) {
				dup2(pipefd[1], 1);
			}
			else {
				dup2(pipefd[1], 2);
			}
			close(pipefd[1]);
			close(pipefd[0]);
			cmd_left->execute();
			exit(0);

		}
		else {//second command
			dup2(pipefd[0], 0);
			close(pipefd[0]);
			close(pipefd[1]);

			bool with_amperc = entered_cmd.find("|&") != string::npos ? true : false;
			string barrier = with_amperc ? "|&" : "|";
			string left_cmd = entered_cmd.substr(0, entered_cmd.find(barrier));
			auto index = !with_amperc ? entered_cmd.find(barrier) + 1 : entered_cmd.find(barrier) + 2;
			string right_cmd = entered_cmd.substr(index,entered_cmd.size());

			left_cmd = _ltrim(_rtrim(left_cmd));
			right_cmd = _ltrim(_rtrim(right_cmd));
			
			if (right_cmd.find("&") != string::npos) {
				right_cmd=right_cmd.erase(right_cmd.find_last_of("&"));
				right_cmd = _ltrim(_rtrim(right_cmd));
			}

			SmallShell& tmp = SmallShell::getInstance();
			shared_ptr<Command> cmd_right(tmp.CreateCommand(right_cmd.c_str()));

			cmd_right->execute();
			if (waitpid(pid, nullptr, WUNTRACED) == -1) {
				perror("waitpid failed");
				return;
			}
			exit(0);
		}
		exit(0);

	}
	else {// smash procces
		setpgrp();
		if (waitpid(pip_pid, nullptr, WUNTRACED) == -1) {
			perror("waitpid failed");
			return;
		}
	}



}


