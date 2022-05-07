#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <time.h>
#include <utime.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <unistd.h>
#include <cassert>
#include <fcntl.h>

using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

const std::string WHITESPACE = " \n\r\t\f\v";

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

bool isPrefixOf(const char* prefix, const char* str){
    while(*prefix != '\0'){
        if (*str != *prefix){
            return false;
        }
        str++;
        prefix++;
    }
    return true;
}

int _countRedirections(const char * cmd_line) {
    const char *cur = cmd_line;
    int count = 0;
    while (*cur != '\0') {
        if (isPrefixOf("|&", cur)) {
            count++;
            cur += 2;
            continue;
        }
        if (isPrefixOf("|", cur)) {
            count++;
            cur++;
            continue;
        }
        if (isPrefixOf(">>", cur)) {
            count++;
            cur += 2;
            continue;
        }
        if (isPrefixOf(">", cur)) {
            count++;
            cur++;
            continue;
        }
        cur++;
    }
    return  count;
}
void writeStrWithoutTermination(const char* copiedFrom, char* copiedTo){
    while(*copiedFrom != '\0'){
        *copiedTo = *copiedFrom;
        copiedTo++;
        copiedFrom++;
    }
}
char* _addSpacesBeforeRedirections(const char * cmd_line){
    int redirectionTimes = _countRedirections(cmd_line);
    int len = std::string(cmd_line).length();
    char* temp = (char*) malloc(sizeof(char) * (len + redirectionTimes + 1));
    const char * copiedFrom = cmd_line;
    char * copiedTo = temp;
    while(*copiedFrom != '\0'){
        if (isPrefixOf("|&", copiedFrom)){
            writeStrWithoutTermination(" |& ", copiedTo);
            copiedFrom += 2;
            copiedTo += 4;
            continue;
        }
        if (isPrefixOf("|", copiedFrom)){
            writeStrWithoutTermination(" | ", copiedTo);
            copiedFrom += 1;
            copiedTo += 3;
            continue;
        }
        if (isPrefixOf(">>", copiedFrom)){
            writeStrWithoutTermination(" >> ", copiedTo);
            copiedFrom += 2;
            copiedTo += 4;
            continue;
        }
        if (isPrefixOf(">", copiedFrom)){
            writeStrWithoutTermination(" > ", copiedTo);
            copiedFrom += 1;
            copiedTo += 3;
            continue;
        }
        *copiedTo = *copiedFrom;
        copiedFrom++;
        copiedTo++;
    }
    *copiedTo = '\0';
    return temp;
}


int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()

  cmd_line = _addSpacesBeforeRedirections(cmd_line);

  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
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

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell()
    :prompt("smash> "),
     jobsList(JobsList()),
     cur(nullptr),
     prevDir(nullptr)

{

// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  char **args = (char**)malloc(sizeof(char*)*22); //check if malloc succesded

  int x = _parseCommandLine(cmd_line,args);
  if(nullptr==args[0])
      return nullptr;
  char* arg1=args[0];

   if (firstWord.compare("chprompt") == 0) {

       getInstance().jobsList.addJob(new ExternalCommand(cmd_line), false);
       return new ChangePromptCommand(cmd_line);
     }else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
   }else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line);
   }else if (firstWord.compare("jobs") == 0) {
        return new JobsCommand(cmd_line);
   }else{
       return new CommandsPack(cmd_line);
   }

    // For example:
/*
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    std::string prompt;
    int test;
    Command *cmd = CreateCommand(cmd_line);
    if (cmd != nullptr){
    cmd->execute();}

  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

bool SmallShell::prevDirExists() {
    return (prevDir != nullptr);
}

std::string SmallShell::getPrevDir() {
    assert(prevDir != nullptr);
    return *prevDir;
}

void SmallShell::changePrevDir(std::string prev) {
    delete prevDir;
    prevDir = new std::string(prev);
}

Command::Command(const char * cmd_line)
    :cmd_line(cmd_line),
     args((char**)malloc(sizeof(char*) * (MAX_ARGS_NUM + 1)))

{
    if (_isBackgroundComamnd(cmd_line)){
        int len = strlen(cmd_line);
        char* temp = (char*) malloc(sizeof(char) * (len + 1));
        strcpy(temp, cmd_line);
        _removeBackgroundSign(temp);
        cmd_line = temp;
        doesRunInBackground = true;
    }else{
        doesRunInBackground = false;
    }
    _parseCommandLine(cmd_line, args);
}

void GetCurrDirCommand::execute() {
    *outputStream << getCurDir() << std::endl;
}

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line)
    : BuiltInCommand(cmd_line)
{}

std::string GetCurrDirCommand::getCurDir() {
    int size = pathconf(".", _PC_PATH_MAX);
    char curDir[size];
    getcwd(curDir,sizeof(curDir));
    std::string temp(curDir);
    return std::string(curDir);
}

BuiltInCommand::BuiltInCommand(const char* cmd_line)
    :Command(cmd_line), inputStream(&cin), outputStream(&cout)
{}

void ChangePromptCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    if(nullptr == args[1]){
        smash.prompt = "smash> ";
    }
    const char* arg2 = args[1];
    smash.prompt = arg2;
    smash.prompt.append(std::string("> "));
}

ChangePromptCommand::ChangePromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line)

{}

void ShowPidCommand::execute() {
    *outputStream << "smash pid is " << getpid() << std::endl;
}

void ChangeDirCommand::execute() {
    if (args[1] == nullptr){
        //TODO
    }else if (args[2] == nullptr){
        SmallShell& smash = SmallShell::getInstance();
        std::string prevDir = GetCurrDirCommand::getCurDir();
        int changeResult;
        if (std::string(args[1]) == std::string("-")){
            changeResult = chdir(smash.getPrevDir().c_str());
        }else{
            changeResult = chdir(args[1]);
        }
        if (changeResult == CHANGE_FAILURE){
            //TODO
        }else{
            smash.changePrevDir(prevDir);
        }
    }else{
        *outputStream << "smash error: cd: too many arguments" << std::endl;
    }

}

ChangeDirCommand::ChangeDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line)
{}

JobsList::JobsList()
    :jobsList(std::list<JobEntry>())
{

}

void JobsList::addJob(ExternalCommand *cmd, bool isStopped) {
    jobsList.sort();
    int newjob_id;
    int size=jobsList.size();
    if (jobsList.empty()== true){
        newjob_id=1;
    }else{
        newjob_id=jobsList.back().jobId+1;
    }
    time_t newjob_time;
    time(&newjob_time);

    JobEntry newjob(*cmd, isStopped, newjob_id, newjob_time);
    jobsList.push_back(newjob);
}

void JobsList::printJobsList() {
    int size=jobsList.size();

}

JobsList::JobEntry &JobsList::operator[](int x)  {
    std::list<JobEntry>::iterator it = jobsList.begin();
  // Advance the iterator by x positions,
     std::advance(it, x);
}


JobsCommand::JobsCommand(const char *cmdLine) : BuiltInCommand(cmdLine) {

}

void JobsCommand::execute() {

}

JobsList::JobEntry::JobEntry(ExternalCommand &command, bool isStopped, int jobId, time_t time_insert)
                            :command(command),isStopped(isStopped),jobId(jobId),time_insert(time_insert) {

}

bool JobsList::JobEntry::operator<(JobsList::JobEntry &job) const {
    return this->jobId < job.jobId;
}

ExternalCommand::ExternalCommand(const char *cmd_line) : Command(cmd_line) ,pid(0){

}

void ExternalCommand::execute() {
    int result = setpgrp();
    if(result == FAILURE){
        //TODO
    }
    execv(args[0],args);
}

CommandsPack::CommandsPack(const char *cmd_line) : Command(cmd_line) {
    if (!isCmdLegal()){
        //TODO
        return;
    }
    const static int MAX_PROCESSES_NUM = 100;
    int curArg = 0;
    int curProgramIndex = 0;
    while(true){
        if (endOfTextDetected(curArg)){
            break;
        }
        if (redirectionDetected(curArg)){
            curArg = setRedirection(curArg);
            continue;
        }
        curArg = addProgram(curArg);
        curProgramIndex++;
    }
    this->processesNum = curProgramIndex;
}

bool CommandsPack::isCmdLegal() const {
    return true;
}

bool CommandsPack::endOfTextDetected(int curArg) const {
    return (args[curArg] == nullptr);
}

bool areEqual(std::string const & string1, std::string const & string2){
    return string1 == string2;
}

bool CommandsPack::redirectionDetected(int curArg) const {
    return (areEqual(args[curArg], ">") ||
            areEqual(args[curArg], ">>") ||
            areEqual(args[curArg], "|") ||
            areEqual(args[curArg], "&|"));
}

int CommandsPack::setRedirection(int curArg) {
    if (areEqual(args[curArg], "|")){
        programs.back().pipeType = P_NORMAL;
        return curArg + 1;
    }else if (areEqual(args[curArg], "&|")){
        programs.back().pipeType = P_ERR;
        return curArg + 1;
    }else if (areEqual(args[curArg], ">")){
        outFileType = R_NORMAL;
        *outFile = std::string(args[curArg + 1]);
        return curArg + 2;
    }else if (areEqual(args[curArg], ">>")){
        outFileType = R_APPEND;
        *outFile = std::string(args[curArg + 1]);
        return curArg + 2;
    }
}

int CommandsPack::addProgram(int curArg) {
    (char**)malloc(sizeof(char*) * (MAX_ARGS_NUM + 1));
    int cur = curArg;
    std::string cmd("");
    while(!endOfTextDetected(cur) && !redirectionDetected(cur)){
        cmd += std::string(args[cur]) + std::string(" ");
        cur++;
    }
    programs.emplace_back(ExternalCommand(cmd.c_str()));
    //TODO
}

void CommandsPack::execute() {
    auto i = programs.begin();
    int* prevFd = nullptr;
    int* curFd = nullptr;
    while(i != programs.end()) {
        prevFd = curFd;
        if (!(i->isLast())){
            curFd = new int[2];
            pipe(curFd);
        }
        int forkResult = fork();
        if (forkResult == 0) {
            if (!(i->isLast())) {
                dup2(curFd[1], 1);
                close(curFd[0]);
                close(curFd[1]);
            }else if (outFileType != R_NORMAL){
                int outFD = open(inFile->c_str(), O_WRONLY|O_CREAT|O_TRUNC); //check last
                dup2(outFD, 1);
                close(outFD);
            }else if (outFileType == R_APPEND){
                int outFD = open(inFile->c_str(), O_WRONLY|O_CREAT|O_APPEND); //check last
                dup2(outFD, 1);
                close(outFD);
            }
            if (prevFd != nullptr){
                dup2(prevFd[0],0);
                close(prevFd[0]);
                close(prevFd[1]);
            }
            i->command.execute();
            delete prevFd;
            delete curFd;
            break;
        }
        //save pid in command in father process
        i->command.pid = forkResult;
        delete prevFd;
        ++i;
    }
    delete curFd;

}
/*
KillCommand::KillCommand(const char *cmd_line)
    : BuiltInCommand(cmd_line)
{}

int str2int(std::string s){
    int x;
    std::stringstream ss;
    ss << s;
    ss >> x;
    return x;
}

void KillCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    JobsList::JobEntry* job;
    int jobId = str2int(args[1]);
    try{
        job = smash.jobsList.getJobById(jobId);
    }catch(JobDoesntExist&){
        *outputStream << "smash error: kill: job-id " << jobId << " does not exist" << std:endl;
        return;
    }
    ExternalCommand& cmd = job->command;
    pid_t pid = cmd.pid;
    int signal = str2int(args[2]);
    int result = kill(pid, signal);
    if (result == SYSCALL_FAILURE){
        //TODO
    }
    if (signal == SIGSTOP){
        job->isStopped = true;
    }
    if (signal == SIGCONT){
        job->isStopped = false;
    }
    *outputStream << "signal number " << signal << " was sent to pid " << pid;
}
*/
CommandsPack::Program::Program(ExternalCommand commmand)
    :command(command), pipeType(P_NONE)
{}

bool CommandsPack::Program::isLast() const {
    return pipeType == P_NONE;
}

