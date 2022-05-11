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
#include <cstdio>

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

auto prefixList = std::list<std::string>({
    string("|&"),
    string("|"),
    string(">>"),
    string(">")
});

int _countRedirections(const char * cmd_line) {
    const char *cur = cmd_line;
    int count = 0;
    while (*cur != '\0') {
        bool found = false;
        for(auto prefix : prefixList){
            if(isPrefixOf(prefix.c_str(), cur)){
                count++;
                cur += prefix.length();
                found = true;
                break;
            }
        }
        if (!found){
            cur++;
        }
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
   }else if(firstWord.compare("quit") == 0){
       return new QuitCommand(cmd_line,&(SmallShell::getInstance().jobsList));
   }else{
       return new ExternalCommand(cmd_line);
   }
   //return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    std::string prompt;
    Command *cmd = new CommandsPack(cmd_line);
    if (cmd != nullptr) {
        jobsList.update();
        if(!cmd->doesNeedFork){
            cmd->execute();
            delete cmd;
        }else if (cmd->doesRunInBackground) {
            jobsList.addJob(dynamic_cast<CommandsPack*>(cmd));
            cmd->execute();
        }else{
            CommandsPack* curCmd = dynamic_cast<CommandsPack*>(cmd);
            cur = curCmd;
            cmd->execute();
            if (curCmd->doesNeedFork) {
                wait();
            }
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
        cur = nullptr;
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

std::ostream& operator<<(std::ostream& os, const Command& cmd){
    os << cmd.cmd_line;
}

void GetCurrDirCommand::execute() {
    (*outputStream) << string(getCurDir()) << "\n";
}

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line)
    :BuiltInCommand(cmd_line)
{}

std::string GetCurrDirCommand::getCurDir() {
    int size = pathconf(".", _PC_PATH_MAX);
    char curDir[size];
    getcwd(curDir,sizeof(char)*size);
    return std::string(curDir);
}

BuiltInCommand::BuiltInCommand(const char* cmd_line)
    :Command(cmd_line), outputStream(new userostream(cout))
{

}

void BuiltInCommand::setRedirection(const std::string& file, RedirectionType type) {
    fstream* outStream = new fstream();
    if (type == R_NORMAL){
        outStream->open(file.c_str(), ios_base::out);
    }else{
        outStream->open(file.c_str(), ios_base::out | std::ios_base::app);
    }
    if ((outStream->rdstate() & std::ifstream::failbit) != 0){
        throw FileError();
    }else{
        delete outputStream;
        outputStream = new userostream(*outStream);
    }
}



BuiltInCommand::~BuiltInCommand() {
    delete outputStream;
}

void BuiltInCommand::setOutPipe(userostream& os, PipeType type) {

    if (type == P_NORMAL) {
        delete outputStream;
        outputStream = &os;
    }else{
        delete errorStream;
        errorStream = &os;
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
    *outputStream << "smash pid is " << to_string(getpid()) << "\n";
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
        *outputStream << "smash error: cd: too many arguments" << "\n";
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
/*
JobsList::JobEntry &JobsList::operator[](int x)  {
    std::list<JobEntry>::iterator it = jobsList.begin();
  // Advance the iterator by x positions,
     std::advance(it, x);
}
*/

JobsCommand::JobsCommand(const char *cmdLine) : BuiltInCommand(cmdLine) {

}

void JobsCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    smash.jobsList.update();
    *outputStream << smash.jobsList.toString();
}

JobsList::JobEntry::JobEntry(CommandsPack &command, bool isStopped, int jobId, time_t time_insert)
                            :command(command),isStopped(isStopped),jobId(jobId),time_insert(time_insert) {

}

bool JobsList::JobEntry::operator<(JobsList::JobEntry &job) const {
    return this->jobId < job.jobId;
}

std::ostream &operator<<(ostream &os, const JobsList::JobEntry &entry) {
    time_t time_print;
    time(&time_print);
    os << "[" << entry.jobId <<"] "<< entry.command<< " " <<difftime( time_print,entry.time_insert)<<" secs";
    return os;
}

std::string JobsList::JobEntry::toString() const {
    stringstream ss;
    ss << *this;
    return ss.str();
}

std::ostream &operator<<(ostream &os, const CommandsPack &cmd) {
    os << *(cmd.programs.front().command) << " : " << cmd.getPid() ;
    return os;
}

std::ostream &operator<<(ostream &os, const JobsList& l) {
    for (const JobsList::JobEntry& job : l.jobsList){
        if (job.isStopped){
            os << job <<" stopped"<< std::endl;
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
        std::cout << job.command.toString2() <<std::endl;
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

void JobsList::removeJobByIdToRunInForeground(int jobId) {
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

std::string JobsList::toString() const {
    stringstream ss;
    ss << *this;
    return ss.str();
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
    doesNeedFork = true;
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
    doesNeedFork = false;
    for(auto& program : programs){
        if(program.command->doesNeedFork){
            doesNeedFork = true;
            break;
        }
    }
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

std::string Command::toString2() const {
    return cmd_line;
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
    Command* command = SmallShell::getInstance().CreateCommand(cmd.c_str());
    Program program(*command);
    programs.push_back(program);
    program.dontDestroyCommand();
    //TODO
    return cur++;
}


void setFileAsOutput(string path, bool isAppended);


void CommandsPack::execute() {
    auto i = programs.begin();
    RedirectionControl control;
    while(i != programs.end()) {
        control.prepareFor(*i);
        //sleep(2);
        if (i->command->doesNeedFork){
            ExternalCommand& cmd = *dynamic_cast<ExternalCommand*>(i->command);
            int forkResult = fork();
            if (forkResult == 0) {
                control.inSon();
                if (i->isLast()){
                    control.setRedirectionIfNeeded(outFile, outFileType);
                }
                cmd.execute();
            }else{
                cmd.pid = forkResult;
                control.inFather();
            }
        }else{
            control.builtIn();
            BuiltInCommand& cmd = *dynamic_cast<BuiltInCommand*>(i->command);
            if (i->isLast()){
                control.setRedirectionIfNeeded(outFile, outFileType);
            }
            cmd.execute();
        }

        ++i;
    }

}

bool CommandsPack::isSingleProgram() const {
    return processesNum == 1;
}

pid_t CommandsPack::getPid() const {
    assert(this->isSingleProgram());
    ExternalCommand& cmd = *dynamic_cast<ExternalCommand*>(programs.front().command);
    return cmd.pid;
}

int CommandsPack::wait() {
    //it is assumed that the status of the processes is the same
    int status;
    for(auto i = programs.begin(); i != programs.end(); ++i){
        if (i->command->doesNeedFork) {
            ExternalCommand& cmd = *dynamic_cast<ExternalCommand*>(i->command);
            waitpid(cmd.pid, &status, WUNTRACED);
        }
    }
    return status;



}

void CommandsPack::sendSig(int signum){
    for(Program& program :programs) {
        if (doesNeedFork) {
            ExternalCommand& cmd = *dynamic_cast<ExternalCommand*>(programs.front().command);
            kill(cmd.pid, signum);
        }
    }
}

std::string CommandsPack::toString() const {
    stringstream ss;
    ss << *this;
    return ss.str();
}

CommandsPack::~CommandsPack() {

}

std::string CommandsPack::toString2() const {
    return ((to_string(this->getPid())+": ") + (*(this->programs.front().command)).toString2());
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
        *outputStream << "smash error: kill: job-id " << to_string(jobId) << " does not exist" << "\n";
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
    *outputStream << "signal number " << to_string(signal) << " was sent to pid " << to_string(pid) << "\n";
}

CommandsPack::Program::Program(Command& command)
    :command(&command), pipeType(P_NONE), destroyCommand(true)
{}

bool CommandsPack::Program::isLast() const {
    return pipeType == P_NONE;
}

CommandsPack::Program::~Program() {
    if (destroyCommand) {
        delete command;
    }
}

void CommandsPack::Program::dontDestroyCommand() {
    destroyCommand = false;
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
    jobsList.removeJobByIdToRunInForeground(jobId);
    assert(smash.cur == nullptr);
    smash.cur = job;
    smash.wait();
}

CommandsPack::Pipe::Pipe()
    :closeInput(false),
     closeOutput(false)
{
    int pipeArr[2];
    pipe(pipeArr);
    inPipe = pipeArr[0];
    outPipe = pipeArr[1];
    //cout << "pipe created " << inPipe << "," << outPipe << "\n";
}

void CommandsPack::Pipe::dupInputTo(int fd) {
    //cout << "pipe input connected fd:" << fd << " inpipe " << inPipe << " pid:" << getpid() << "\n";
    dup2(inPipe, fd);
}

void CommandsPack::Pipe::dupOutputTo(int fd) {
    //cout << "pipe output connected fd:" << fd << " outpipe " << outPipe << " pid:" << getpid() << "\n";
    dup2(outPipe, fd);
}

userostream *CommandsPack::Pipe::GetOutputStream() {
    //cout << "pipe output connected pipe:" << outPipe << endl;
    return new userostream(outPipe);

}

void CommandsPack::Pipe::SetToCloseInput() {
    closeInput = true;
}

void CommandsPack::Pipe::SetToCloseOutput() {
    closeOutput = true;
}

void CommandsPack::Pipe::SetNotToCloseInput() {
    closeInput = false;
}

void CommandsPack::Pipe::SetNotToCloseOutput() {
    closeOutput = false;
}

CommandsPack::Pipe::~Pipe() {
    if (closeInput){
        close(inPipe);

    }
    if (closeOutput){
        close(outPipe);
    }
}


userostream &operator<<(userostream &os, const string &str) {
    if (os.cStyleFile){
        write(os.fd, str.c_str(), str.length());
        return os;
    }else{
        *os.stream << str;
        return os;
    }

}

userostream::userostream(int fd){
    //file = fdopen(fd, "a");
    cStyleFile = true;
    this->fd = fd;
}

userostream::userostream(ostream &stream) {
    this->stream = &stream;
    cStyleFile = false;
}

userostream::~userostream() {
    if (cStyleFile){
        close(fd);
    }else{
        if (stream != &std::cout && stream != &std::cerr){
            dynamic_cast<fstream*>(stream)->close();
            delete stream;
        }
    }
}


CommandsPack::RedirectionControl::RedirectionControl()
    :prevPipe(nullptr), curPipe(nullptr)
{}

void CommandsPack::RedirectionControl::createPipeIfNeeded() {
    delete prevPipe;
    prevPipe = curPipe;
    if (curProgram->isLast()){
        curPipe = nullptr;
    }else {
        curPipe = new Pipe();
    }
}

void CommandsPack::RedirectionControl::setCurProgramInPipeIfNeeded() {
    if (!isSettingInPipeNeeded()){
        return;
    }
    if (curProgram->command->doesNeedFork){
        prevPipe->dupInputTo(0);
    }else{
        //do nothing
    }
}

void CommandsPack::RedirectionControl::setCurProgramOutPipeIfNeeded() {
    if (!isSettingOutPipeNeeded()){
        return;
    }
    if (curProgram->command->doesNeedFork){
        if (curProgram->pipeType == P_NORMAL) {
            curPipe->dupOutputTo(1);
        }else if (curProgram->pipeType == P_ERR){
            curPipe->dupOutputTo(1);
        }
    }else{
        BuiltInCommand& cmd = *dynamic_cast<BuiltInCommand*>(curProgram->command);
        cmd.setOutPipe(*curPipe->GetOutputStream(), curProgram->pipeType);
    }
}

CommandsPack::RedirectionControl::~RedirectionControl() {
    delete curPipe;
    delete prevPipe;
}

void CommandsPack::RedirectionControl::prepareFor(CommandsPack::Program &program) {
    curProgram = &program;
    createPipeIfNeeded();
}

void CommandsPack::RedirectionControl::setPipingIfNeeded() {
    if (prevPipe != nullptr){
        setCurProgramInPipeIfNeeded();
    }
    if(curProgram->pipeType != P_NONE){
        setCurProgramOutPipeIfNeeded();
    }
}

void CommandsPack::RedirectionControl::setRedirectionIfNeeded(const string &outFile,
                                                              RedirectionType type) {
    assert(curProgram->isLast());
    if (type == R_NONE){
        return;
    }
    if (curProgram->command->doesNeedFork){
        if (type == R_NORMAL){
            int outFD = open(outFile.c_str(), O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR); //check last
            dup2(outFD, 1);
            close(outFD);
        }else if (type == R_APPEND){
            int outFD = open(outFile.c_str(), O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR); //check last
            dup2(outFD, 1);
            close(outFD);
        }
    }else{
        BuiltInCommand& cmd = *dynamic_cast<BuiltInCommand*>(curProgram->command);
        cmd.setRedirection(outFile, type);
    }
}

void CommandsPack::RedirectionControl::inFather() {
    cleanupCurProgramPipe();
}

void CommandsPack::RedirectionControl::inSon() {
    setPipingIfNeeded();
    cleanupCurProgramPipe();
}

void CommandsPack::RedirectionControl::cleanupCurProgramPipe() {
    if (isSettingInPipeNeeded()) {
        prevPipe->SetToCloseInput();
    }
    if (isSettingOutPipeNeeded()) {
        curPipe->SetToCloseOutput();
    }
}

void CommandsPack::RedirectionControl::dontCleanupCurProgramPipe() {
    if (isSettingInPipeNeeded()) {
        //prevPipe->SetToCloseInput();
        prevPipe->SetToCloseInput();
    }
    if (isSettingOutPipeNeeded()) {
        curPipe->SetNotToCloseOutput();
    }
}

void CommandsPack::RedirectionControl::builtIn() {
    setPipingIfNeeded();
    dontCleanupCurProgramPipe();
}

bool CommandsPack::RedirectionControl::isSettingInPipeNeeded() {
    return prevPipe != nullptr;
}

bool CommandsPack::RedirectionControl::isSettingOutPipeNeeded() {
    return curPipe != nullptr;
}


QuitCommand::QuitCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line),jobs(jobs) {

}

void QuitCommand::execute() {
    if(args[1] != nullptr)
    {
     if(areEqual(args[1],"kill")){
         jobs->killAllJobs();
     }else{
         throw system_error();
     }}else{
            *outputStream << "am here 2";
            throw system_error();
     }
}
