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
#include <fstream>

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

void copyArgs(const char* const* src, char** dest){
    while(*src != nullptr){
        int len = strlen(*src);
        *dest = (char*) malloc(sizeof(char) * (len + 1));
        strcpy(*dest, *src);
        dest++;
        src++;
    }
    *dest = nullptr;
}


Args::Args(char const* const*  args){
    obj =  (char**)malloc(sizeof(char*) * (MAX_ARGS_NUM + 1));
    copyArgs(args, obj);
}
Args::~Args(){
    char** cur = obj;
    while(*cur != nullptr){
        free(*cur);
        cur++;
    }
    free(*cur);
    free(obj);
}
 char const * const* Args::getObj() const{
    return obj;
}

Args::Args(const Args &args) {
    obj =  (char**)malloc(sizeof(char*) * (MAX_ARGS_NUM + 1));
    copyArgs(args.obj, obj);
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
    auto prefixList = std::list<std::string>({
        string("|&"),
        string("|"),
        string(">>"),
        string(">")

    });
    while (*cur != '\0') {
        bool found = false;
        for(auto prefix : prefixList){
            if(isPrefixOf(prefix.c_str(), cur)){
                count++;
                cur += prefix.length() - 1;
                found = true;
                break;
            }
        }
        if (!found){
            cur++;
        }
        /*if (isPrefixOf("|&", cur)) {
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
        cur++;*/
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
    auto prefixList = std::list<std::string>({
        string("|&"),
        string("|"),
        string(">>"),
        string(">")
    });
    while(*copiedFrom != '\0'){
        bool found = false;
        for(const auto& prefix : prefixList){
            if (isPrefixOf(prefix.c_str(), copiedFrom)) {
                string replacement = " " + prefix + " ";
                writeStrWithoutTermination(replacement.c_str(), copiedTo);
                copiedFrom += prefix.length();
                copiedTo += replacement.length();
                found = true;
                break;
            }
        }
        if (!found){
            *copiedTo = *copiedFrom;
            copiedFrom++;
            copiedTo++;
        }
        /*if (isPrefixOf("|&", copiedFrom)){
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
        copiedTo++;*/
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
       return new ChangePromptCommand(cmd_line);
     }else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
   }else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line);
   }else if (firstWord.compare("jobs") == 0) {
       return new JobsCommand(cmd_line);
   }else if (firstWord.compare("showpid") == 0) {
       return new ShowPidCommand(cmd_line);
   }else if(firstWord.compare("kill") == 0) {
       return new KillCommand(cmd_line);
   }else if(firstWord.compare("bg") == 0){
       return new BackgroundCommand(cmd_line);
   }else if(firstWord.compare("fg") == 0){
       return new ForegroundCommand(cmd_line);
   }else{
       return new CommandsPack(cmd_line);
   }
   //return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    std::string prompt;
    Command *cmd = CreateCommand(cmd_line);
    if (cmd != nullptr) {
        jobsList.update();
        if(!cmd->doesNeedFork){
            cmd->execute();
        }else if (cmd->doesRunInBackground) {
            jobsList.addJob(dynamic_cast<CommandsPack*>(cmd));
            cmd->execute();
        }else{
            CommandsPack* curCmd = dynamic_cast<CommandsPack*>(cmd);
            cur = curCmd;
            cmd->execute();
            wait();
            cur = nullptr;
        }
    }

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

void SmallShell::wait() {
    int status = cur->wait();
    if (WIFSTOPPED(status)){
        jobsList.addJob(cur, true);
    }else{
        delete cur;
    }
}


Command::Command(const char* cmd_line, bool areArgsReady, Args readyArgs )
    :cmd_line(cmd_line),
     args((char**)malloc(sizeof(char*) * (MAX_ARGS_NUM + 1))),
     doesNeedFork(false)

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
    if (!areArgsReady) {
        _parseCommandLine(cmd_line, args);
    }else{
        copyArgs(readyArgs.getObj(), args);
    }
}

void GetCurrDirCommand::execute() {
    *outputStream << getCurDir() << std::endl;
}

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line)
    :BuiltInCommand(cmd_line)
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
{
    for(int i = 0; args[i] != nullptr; i++){
        if (redirectionDetected(i)){
            setRedirection(i);
        }
    }
}

void BuiltInCommand::setRedirection(int curArg) {
    fstream* outStream = new fstream();
    const char* path = args[curArg + 1];
    if (string(args[curArg]) == string(">")){
        outStream->open(path, ios_base::out);
    }else if(string(args[curArg]) == string(">>")){
        outStream->open(path, ios_base::out | std::ios_base::app);
    }else{
        throw SyntaxError();
    }
    if ((outStream->rdstate() & std::ifstream::failbit) != 0){
        throw FileError();
    }else{
        outputStream = outStream;
    }

}

BuiltInCommand::~BuiltInCommand() {
    if (outputStream != &std::cout){
        static_cast<fstream*>(outputStream)->close();
        delete outputStream;
    }
}

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

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

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

void JobsList::addJob(CommandsPack *cmd, bool isStopped) {
    update();
    jobsList.sort();
    int newjob_id;
    int size=jobsList.size();
    if (jobsList.empty()){
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

}

JobsList::JobEntry &JobsList::operator[](int x)  {
    std::list<JobEntry>::iterator it = jobsList.begin();
  // Advance the iterator by x positions,
     std::advance(it, x);
}


JobsCommand::JobsCommand(const char *cmdLine) : BuiltInCommand(cmdLine) {

}

void JobsCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    smash.jobsList.update();
    *outputStream << smash.jobsList;
}

JobsList::JobEntry::JobEntry(CommandsPack &command, bool isStopped, int jobId, time_t time_insert)
                            :command(command),isStopped(isStopped),jobId(jobId),time_insert(time_insert) {

}

bool JobsList::JobEntry::operator<(JobsList::JobEntry &job) const {
    return this->jobId < job.jobId;
}

std::ostream &operator<<(ostream &os, const JobsList::JobEntry &entry) {
    os << "jobid: " << entry.jobId << " stopped:" << entry.isStopped
            << " " << entry.command;
    return os;
}

std::ostream &operator<<(ostream &os, const CommandsPack &cmd) {
    os << "pid: " << cmd.getPid() << "cmdLine: " << cmd.cmd_line ;
    return os;
}

std::ostream &operator<<(ostream &os, const JobsList& l) {
    for (const JobsList::JobEntry& job : l.jobsList){
        if (job.isStopped){
            os << job << std::endl;
        }
    }
    for (const JobsList::JobEntry& job : l.jobsList){
        if (!job.isStopped){
            os << job << std::endl;
        }
    }

    return os;
}

void JobsList::killAllJobs() {
    for(JobsList::JobEntry& job : jobsList){
        job.command.sendSig(SIGKILL);
    }
    update();
}

void JobsList::update() {
    auto iList = std::list<std::list<JobEntry>::iterator>();
    for(auto i = jobsList.begin(); i != jobsList.end(); ++i){
        int status;
        JobEntry& job = *i;
        int waitResult = waitpid(job.command.getPid(), &status, WNOHANG); //TODO check if return value is needed
        //if process changed state
        if (waitResult == job.command.getPid() && (WIFEXITED(status) || WIFSIGNALED(status))){
            iList.push_back(i);
        }
    }
    for(auto i : iList){
        jobsList.erase(i);
    }
}

void JobsList::RunJob(int jobId) {
    JobEntry* job = getJobById(jobId);
    assert(job != nullptr);
    if (!job->isStopped){
        return;
    }else {
        job->isStopped = false;
        job->command.sendSig(SIGCONT);
    }
}

void JobsList::StopJob(int jobId) {
    JobEntry* job = getJobById(jobId);
    assert(job != nullptr);
    job->isStopped = true;
    job->command.sendSig(SIGSTOP);
}

const JobsList::JobEntry *JobsList::getJobById(int jobId) const{
    for(const JobEntry& job : jobsList) {
        if (job.jobId == jobId){
            return &job;
        }
    }
    return nullptr;
}

JobsList::JobEntry *JobsList::getJobById(int jobId){
    return const_cast<JobEntry*>
            (const_cast<const JobsList*>(this)->getJobById(jobId));
}

void JobsList::removeJobById(int jobId) {
    for(auto i = jobsList.begin(); i != jobsList.end(); ++i){
        if (i->jobId == jobId){
            jobsList.erase(i);
            return;
        }
    }
}

CommandsPack&  JobsList::getLastJob(int* jobId){
    if(jobsList.empty()){
        throw NoJobs();
    }else{
        *jobId = jobsList.back().jobId;
        return jobsList.back().command;
    }
}

CommandsPack&  JobsList::getLastStoppedJob(int* jobId){
    for(auto i = jobsList.rbegin(); i != jobsList.rend(); ++i){
        if (i->isStopped){
            *jobId = i->jobId;
            return i->command;
        }
    }
    throw NoStoppedJobs();
}

CommandsPack &JobsList::getJobCommandById(int jobId) {
    return getJobById(jobId)->command;
}

bool JobsList::doesExist(int jobId) const {
    return getJobById(jobId) != nullptr;
}

bool JobsList::isStopped(int jobId) const {
    assert(doesExist(jobId));
    return getJobById(jobId)->isStopped;
}

Args ExternalCommand::getModifiedLine(const char * cmd_line) const{
    std::string commandArg =  std::string(cmd_line);
    const char * args[4];
    args[0] = "/bin/bash";
    args[1] = "-c";
    args[2] = cmd_line;
    args[3] = nullptr;
    return Args(args);

}

ExternalCommand::ExternalCommand(const char *cmd_line)
    :Command(cmd_line, true, getModifiedLine(cmd_line)) ,
     pid(0)
{
}

void ExternalCommand::execute() {
    int result = setpgrp();
    if(result == FAILURE){
        //TODO
    }
    execv(args[0],args);

}

CommandsPack::CommandsPack(const char *cmd_line) : Command(cmd_line), outFile() {
    if (!isCmdLegal()){
        //TODO
        return;
    }
    doesNeedFork = true;
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

bool Command::redirectionDetected(int curArg) const {

    bool result = (areEqual(args[curArg], ">") ||
            areEqual(args[curArg], ">>") ||
            areEqual(args[curArg], "|") ||
            areEqual(args[curArg], "&|"));
    if (!result){
        return false;
    }
    if (args[curArg + 1] == nullptr){
        throw SyntaxError();
    }else{
        return true;
    }
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
        outFile = std::string(args[curArg + 1]);
        return curArg + 2;
    }else if (areEqual(args[curArg], ">>")){
        outFileType = R_APPEND;
        outFile = std::string(args[curArg + 1]);
        return curArg + 2;
    }
}

int CommandsPack::addProgram(int curArg) {
    (char**)malloc(sizeof(char*) * (MAX_ARGS_NUM + 1));
    int cur = curArg;
    std::string cmd("");
    while(!endOfTextDetected(cur) && !redirectionDetected(cur)){
        cmd += std::string(args[cur]);
        if (!endOfTextDetected(cur + 1) && !redirectionDetected(cur + 1)){
            cmd += std::string(" ");
        }
        cur++;
    }
    ExternalCommand command(cmd.c_str());
    Program program(command);
    programs.push_back(program);
    //TODO
    return cur++;
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
        }else{
            curFd = nullptr;
        }
        int forkResult = fork();
        if (forkResult == 0) {
            if (!(i->isLast())) {
                int output;
                if (i->pipeType == P_NORMAL){
                    output = 1;
                }else{
                    output = 2;
                }
                dup2(curFd[1], output);
                close(curFd[0]);
                close(curFd[1]);
            }else if (outFileType == R_NORMAL){
                int outFD = open(outFile.c_str(), O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR); //check last
                dup2(outFD, 1);
                close(outFD);
            }else if (outFileType == R_APPEND){
                int outFD = open(outFile.c_str(), O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR); //check last
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
            return;
        }
        if (prevFd != nullptr){
            close(prevFd[0]);
        }
        if (curFd != nullptr){
            close(curFd[1]);
        }
        //save pid in command in father process
        i->command.pid = forkResult;
        delete prevFd;
        ++i;
    }
    delete curFd;
}

bool CommandsPack::isSingleProgram() const {
    return processesNum == 1;
}

pid_t CommandsPack::getPid() const {
    //assert(this->isSingleProgram());
    return programs.front().command.pid;
}

int CommandsPack::wait() {
    //it is assumed that the status of the processes is the same
    int status;
    for(auto i = programs.begin(); i != programs.end(); ++i){
        waitpid(i->command.pid ,&status, WUNTRACED);
    }
    return status;



}

void CommandsPack::sendSig(int signum){
    for(Program& program :programs){
        kill(program.command.pid, signum);
    }
}




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
    CommandsPack* job;
    int jobId = str2int(args[1]);
    try{
        job = &smash.jobsList.getJobCommandById(jobId);
    }catch(JobDoesntExist&){
        *outputStream << "smash error: kill: job-id " << jobId << " does not exist" << std::endl;
        return;
    }
    pid_t pid = job->getPid();
    int signal = - str2int(args[2]);
    if (signal == SIGSTOP){
        smash.jobsList.StopJob(jobId);
    }else if (signal == SIGCONT){
        smash.jobsList.RunJob(jobId);
    }else{
        int result = kill(pid, signal);
        if (result == FAILURE){
            //TODO
            throw SyscallFailure();
        }
    }
    *outputStream << "signal number " << signal << " was sent to pid " << pid << std::endl;
}

CommandsPack::Program::Program(ExternalCommand command)
    :command(command), pipeType(P_NONE)
{}

bool CommandsPack::Program::isLast() const {
    return pipeType == P_NONE;
}

BackgroundCommand::BackgroundCommand(const char *cmd_line)
    :BuiltInCommand(cmd_line)
{}

void BackgroundCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    JobsList& jobsList = smash.jobsList;
    int jobId;
    CommandsPack* job;
    if (args[1] == nullptr){
        jobsList.getLastStoppedJob(&jobId);
    }else{
        jobId = str2int(args[1]);
        assert(jobsList.doesExist(jobId));
        assert(jobsList.isStopped(jobId));
    }
    jobsList.RunJob(jobId);
}


ForegroundCommand::ForegroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

void ForegroundCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    JobsList& jobsList = smash.jobsList;
    int jobId;
    CommandsPack* job;
    if (args[1] == nullptr){
        job = &jobsList.getLastJob(&jobId);
    }else{
        jobId = str2int(args[1]);
        assert(jobsList.doesExist(jobId));
        job = &jobsList.getJobCommandById(jobId);
    }
    jobsList.RunJob(jobId);
    jobsList.removeJobById(jobId);
    smash.cur = job;
    smash.wait();
}
