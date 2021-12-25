#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <csetjmp>
#include "Commands.h"
#include "signals.h"

pid_t smashpid;
jmp_buf env;

int main(int argc, char* argv[]) {
    //TODO: setup sig alarm handler
   if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    SmallShell& smash = SmallShell::getInstance();
pid_t pid=getpid();
smashpid=pid;
    

   
    while(true) {
       
   
        std::cout <<smash.newlen<<"> ";
        std::string cmd_line;
    std::getline(std::cin, cmd_line);
if(cmd_line.empty()){
continue;
}
    
       if(! smash.executeCommand(cmd_line.c_str())) {return 1;}
        
    }
    
    return 0;
}
