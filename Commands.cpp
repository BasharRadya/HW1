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
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>


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
        if (*dest == nullptr){
            throw std::bad_alloc();
        }
        strcpy(*dest, *src);
        dest++;
        src++;
    }
    *dest = nullptr;
}


void freeArgs(char** args){
    char** cur = args;
    while(*cur != nullptr){
        free(*cur);
        cur++;
    }
    free(*cur);
    free(args);
}

Args::Args(char const* const*  args){
    obj =  (char**)malloc(sizeof(char*) * (MAX_ARGS_NUM + 1));
    if (obj == nullptr){
        throw std::bad_alloc();
    }
    try {
        copyArgs(args, obj);
    }catch(std::bad_alloc&){
        if(obj != nullptr){
            freeArgs(obj);
        }
    }
}
Args::~Args(){
    freeArgs(obj);
}
 char const * const* Args::getObj() const{
    return obj;
}

Args::Args(const Args &args) {
    obj =  (char**)malloc(sizeof(char*) * (MAX_ARGS_NUM + 1));
    if (obj == nullptr){
        throw std::bad_alloc();
    }
    copyArgs(args.obj, obj);
}


bool syscallFailed(int returnValue){
    return returnValue < 0;
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

auto redirectionSigns = std::list<std::string>({
    string("|&"),
    string("|"),
    string(">>"),
    string(">")
});

auto timeSeparators = std::list<std::string>({
    string(":")
});


int _countSigns(const char * cmd_line, std::list<string>& signs = redirectionSigns) {
    const char *cur = cmd_line;
    int count = 0;
    while (*cur != '\0') {
        bool found = false;
        for(auto prefix : signs){
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
char* _addSpacesBeforeAndAfterSigns(const char * cmd_line, std::list<string>& signs = redirectionSigns){
    int redirectionTimes = _countSigns(cmd_line, signs);
    int len = std::string(cmd_line).length();
    char* temp = (char*) malloc(sizeof(char) * (len + redirectionTimes * 2 + 1));
    if (temp == nullptr){
        throw std::bad_alloc();
    }
    const char * copiedFrom = cmd_line;
    char * copiedTo = temp;
    while(*copiedFrom != '\0'){
        bool found = false;
        for(const auto& prefix : signs){
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


int _parseCommandLine(const char* cmd_line, char** args, std::list<string>& separators = redirectionSigns ) {
  FUNC_ENTRY()
  cmd_line = _addSpacesBeforeAndAfterSigns(cmd_line, separators);
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    if(args[i] == nullptr){
        throw std::bad_alloc();
    }
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  args[i] = nullptr;
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

int str2int(std::string s){
    int x;
    std::stringstream ss;
    ss << s;
    ss >> x;
    return x;
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
Command * CommandsPack::CreateCommand(const char* cmd_line){
  string cmd_s = _trim(string(cmd_line));
  //string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  char **tempArgs = (char**)malloc(sizeof(char*)*22); //check if malloc succesded
  if (tempArgs == nullptr){
      throw std::bad_alloc();
  }
  int x = _parseCommandLine(cmd_s.c_str(),tempArgs);
  std::string arg1;
  if (*tempArgs == nullptr){
      arg1 = "";
  }else{
      arg1 = tempArgs[0];
  }
  freeArgs(tempArgs);
   if (arg1 == "chprompt") {
       return new ChangePromptCommand(cmd_line);
     }else if (arg1 == "pwd") {
        return new GetCurrDirCommand(cmd_line);
   }else if (arg1 == "cd") {
        return new ChangeDirCommand(cmd_line);
   }else if (arg1 == "jobs") {
       return new JobsCommand(cmd_line);
   }else if (arg1 == "showpid") {
       return new ShowPidCommand(cmd_line);
   }else if(arg1 == "kill") {
       return new KillCommand(cmd_line);
   }else if(arg1 == "bg"){
       return new BackgroundCommand(cmd_line);
   }else if(arg1 == "fg"){
       return new ForegroundCommand(cmd_line);
   }else if(arg1 == "quit"){
       quitExists = true;
       return new QuitCommand(cmd_line,&(SmallShell::getInstance().jobsList));
   }else if(arg1 == "touch") {
       return new TouchCommand(cmd_line);
   }else if(arg1 == "tail"){
       return new TailCommand(cmd_line);
   }else{
       return new ExternalCommand(cmd_line);
   }
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    std::string prompt;
    CommandsPack *cmd = new CommandsPack(cmd_line);
    int alarmDuration = cmd->getAlarmDuration();
    if (alarmDuration > 0){
        SmallShell::getInstance().alarmControl.add(alarmDuration, *cmd);
    }

    bool quitDone;
    if (cmd != nullptr) {
        jobsList.update();
        if(!cmd->doesNeedFork){
            cmd->execute();
            quitDone = cmd->quitDone();
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
        if (quitDone){
            throw ProgramEnded();
        }

    }
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

bool SmallShell::prevDirExists() {
    return (prevDir != nullptr);
}

std::string SmallShell::getPrevDir() {
    assert(prevDirExists());
    return *prevDir;
}

void SmallShell::changePrevDir(std::string prev) {
    delete prevDir;
    prevDir = new std::string(prev);
}

void SmallShell::wait() {
    int status;
    int waitResult = cur->wait(&status);
    if (waitResult == cur->getPid() && WIFSTOPPED(status)){
        jobsList.addJob(cur, true);
    }else{
        bool quitDone = cur->quitDone();
        SmallShell::getInstance().alarmControl.removeIfContained(*cur);
        delete cur;
        cur = nullptr;
        if (quitDone){
            throw ProgramEnded();
        }
    }
}


Command::Command(const char* cmd_line, bool areArgsReady, Args readyArgs )
    :cmd_line(cmd_line),
     doesNeedFork(false)

{
    args = nullptr;
    char* temp = nullptr;
    try {
        args = (char **) malloc(sizeof(char *) * (MAX_ARGS_NUM + 1));
        if (args == nullptr) {
            throw std::bad_alloc();
        }
        if (_isBackgroundComamnd(cmd_line)) {
            int len = strlen(cmd_line);
            temp = (char *) malloc(sizeof(char) * (len + 1));
            if (temp == nullptr) {
                throw std::bad_alloc();
            }
            strcpy(temp, cmd_line);
            _removeBackgroundSign(temp);
            cmd_line = temp;
            doesRunInBackground = true;
        } else {
            doesRunInBackground = false;
        }

        if (!areArgsReady) {
            _parseCommandLine(cmd_line, args);
        } else {
            copyArgs(readyArgs.getObj(), args);
        }
    }catch(std::bad_alloc){
        if (args != nullptr){
            freeArgs(args);
        }
        free(temp);
        throw std::bad_alloc();
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
    char * result = getcwd(curDir,sizeof(char)*size);
    if (result == nullptr){

        throw SyscallFailure("getcwd");
    }
    return std::string(curDir);
}

BuiltInCommand::BuiltInCommand(const char* cmd_line)
    :Command(cmd_line), outputStream(new userostream(cout)), errorStream(new userostream(cerr))
{

}

void BuiltInCommand::setRedirection(const std::string& file, RedirectionType type) {
    userostream* temp = new userostream(file, type);
    delete outputStream;
    outputStream = temp;
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

void BuiltInCommand::cleanpipe() {
    delete outputStream;
    outputStream= nullptr;
    delete errorStream;
    errorStream= nullptr;


}

void ChangePromptCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    if(nullptr == args[1]){
        smash.prompt = "smash> ";
    }else {
        const char *arg2 = args[1];
        smash.prompt = std::string(arg2) + std::string("> ");
    }
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
        *errorStream << "smash error: cd: too few arguments" << "\n";
        return;
    }else if (args[2] == nullptr){
        std::string newDir = args[1];

        SmallShell& smash = SmallShell::getInstance();
        std::string prevDir = GetCurrDirCommand::getCurDir();
        int changeResult;
        if (newDir == std::string("-")){
            if (smash.prevDirExists()) {
                changeResult = chdir(smash.getPrevDir().c_str());
            }else{
                *errorStream << "smash error: cd: OLDPWD not set\n";
                return;
            }
        }else{
            changeResult = chdir(newDir.c_str());
        }
        if (syscallFailed(changeResult)){
            throw SyscallFailure("chdir");
        }else{
            smash.changePrevDir(prevDir);
        }
    }else{
        *errorStream << "smash error: cd: too many arguments" << "\n";
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
    jobsList.sort();
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

JobsList::JobEntry::JobEntry(CommandsPack &command, bool isStopped, int jobId, time_t insertionTime)
                            : command(command), isStopped(isStopped), jobId(jobId), insertionTime(insertionTime) {

}

bool JobsList::JobEntry::operator<(JobsList::JobEntry &job) const {
    return this->jobId < job.jobId;
}

std::ostream &operator<<(ostream &os, const JobsList::JobEntry &entry) {
    time_t time_print;
    time(&time_print);
    os << "[" << entry.jobId << "] " << entry.command << " " << difftime( time_print,entry.insertionTime) << " secs";
    return os;
}

std::string JobsList::JobEntry::toString() const {
    stringstream ss;
    ss << *this;
    return ss.str();
}

bool JobsList::JobEntry::operator<(const JobsList::JobEntry &job) const {
    return this->jobId < job.jobId;
}

std::string JobsList::toString2() const {
    string str;
    for(const JobsList::JobEntry& job : jobsList){
        str += job.command.toString2() + "\n";
    }
    return str;
}

std::ostream &operator<<(ostream &os, const CommandsPack &cmd) {
    os << dynamic_cast<const Command&>(cmd) << " : " << cmd.getPid() ;
    return os;
}

std::ostream &operator<<(ostream &os, const JobsList& l) {

    for (const JobsList::JobEntry& job : l.jobsList){
        if (job.isStopped){
            os << job <<" (stopped)"<< std::endl;
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
    jobsList.sort();
    auto iList = std::list<std::list<JobEntry>::iterator>();
    for(auto i = jobsList.begin(); i != jobsList.end(); ++i){
        JobEntry& job = *i;
        int status;
        int waitResult = job.command.wait(&status, true); //TODO check if return value is needed
        //if process changed state

        if (waitResult == job.command.getPid() && (WIFEXITED(status) || WIFSIGNALED(status))){
            SmallShell::getInstance().alarmControl.removeIfContained(i->command);
            iList.push_back(i);
        }
    }
    for(auto i : iList){
        jobsList.erase(i);
    }
    jobsList.sort();
}

void JobsList::runJob(int jobId) {
    JobEntry* job = getJobById(jobId);
    assert(job != nullptr);
    if (!job->isStopped){
        return;
    }else {
        job->isStopped = false;
        job->command.sendSig(SIGCONT);
    }
}

void JobsList::stopJob(int jobId) {
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
    if( doesExist(jobId)){
        return getJobById(jobId)->command;}
    else{
        throw JobDoesntExist();
    }
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

int JobsList::getSize() const {
    return jobsList.size();
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

ExternalCommand::ExternalCommand(const char* cmd_line)
    :Command(cmd_line, true, getModifiedLine(cmd_line)) ,
     pid(0)
{
    doesNeedFork = true;
}

void ExternalCommand::execute() {
    int result = setpgrp();
    execv(args[0],args);
    throw SyscallFailure("execv");
}



CommandsPack::CommandsPack(const char *cmd_line)
: Command(cmd_line), outFile(), quitExists(false), outFileType(R_NONE)
{
    if (!isCmdLegal()){
        //TODO
        return;
    }
    int curArg;
    const static int MAX_PROCESSES_NUM = 100;
    if (args[0] != nullptr && args[0] == std::string("timeout")) {
        if (args[1] == nullptr || str2int(args[1]) <= 0) {
            std::cerr << "timeout : syntax error\n";
            return;
        }
        curArg = 2;
        //SmallShell::getInstance().alarmControl.add(str2int(args[1]), *this);
    }else {
        curArg = 0;
    }
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
            areEqual(args[curArg], "|&"));
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

Command::~Command() {
    freeArgs(args);
}


int CommandsPack::setRedirection(int curArg) {
    if (areEqual(args[curArg], "|")){
        programs.back().pipeType = P_NORMAL;
        return curArg + 1;
    }else if (areEqual(args[curArg], "|&")){
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
    int cur = curArg;
    std::string cmd("");
    while(!endOfTextDetected(cur) && !redirectionDetected(cur)){
        cmd += std::string(args[cur]);
        if (!endOfTextDetected(cur + 1) && !redirectionDetected(cur + 1)){
            cmd += std::string(" ");
        }
        cur++;
    }
    Command* command = CreateCommand(cmd.c_str());
    Program program(*command);
    programs.push_back(program);
    program.dontDestroyCommand();
    //TODO
    return cur++;
}


void setFileAsOutput(string path, bool isAppended);


void CommandsPack::execute() {
    auto i = programs.begin();
    RedirectionControl& control=*(new RedirectionControl());
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
                delete &control;
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
            cmd.cleanpipe();
        }

        ++i;
    }
    delete &control;
}

bool CommandsPack::isSingleProgram() const {
    return processesNum == 1;
}

pid_t CommandsPack::getPid() const {
    //assert(this->isSingleProgram());
    for(auto& program : programs){
        if (program.command->doesNeedFork){
            ExternalCommand& cmd = *dynamic_cast<ExternalCommand*>(program.command);
            return cmd.pid;
        }
    }
    throw NoForkNeededInPack();
}

int CommandsPack::wait(int* status, bool noHang) {
    //it is assumed that the status of the processes is the same
    int curStatus;
    bool oneProgramHalted = false;
    bool noProgramResponded = true;
    bool noHangEnabled;
    if (noHang){
        noHangEnabled = 1;
    }else{
        noHangEnabled = 0;
    }
    for(auto i = programs.begin(); i != programs.end(); ++i){
        if (i->command->doesNeedFork && (!i->terminated)) {
            ExternalCommand& cmd = *dynamic_cast<ExternalCommand*>(i->command);
            int waitResult = waitpid(cmd.pid, &curStatus, WUNTRACED | (noHangEnabled * WNOHANG));
            if (waitResult == cmd.pid){
                noProgramResponded = false;
            }
            if (waitResult == cmd.pid && (WIFEXITED(curStatus) || WIFSIGNALED(curStatus))){
                i->terminated = true;
                //cout << "program terminated\n";
                if (!oneProgramHalted){
                    *status = curStatus;

                }
            }else if (waitResult == cmd.pid && WIFSTOPPED(curStatus)){ //program halted
                //cout << "program halted\n";
                *status = curStatus;
                oneProgramHalted = true;
            }

        }
    }


    if (noProgramResponded){
        return 0;
    } else{
        return this->getPid();
    }



}

void CommandsPack::sendSig(int signum){
    for(Program& program :programs) {
        if (program.command->doesNeedFork && !program.terminated) {
            //cout << "sent sig:" << signum << "\n";
            ExternalCommand& cmd = *dynamic_cast<ExternalCommand*>(program.command);
            int result = kill(cmd.pid, signum);
            if (syscallFailed(result)){
                throw SyscallFailure("kill");
            }
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
    return ((to_string(this->getPid())+": ") + static_cast<const Command*>(this)->toString2());
}

bool CommandsPack::quitDone() const{
    for(const auto& program: programs){
        if (program.command->doesNeedFork) {
            assert(program.terminated);
        }
    }
    return quitExists;
}

int CommandsPack::getAlarmDuration() const {
    if(args[0] != nullptr && args[1] != nullptr){
        if (args[0] == string("timeout")){
            return str2int(args[1]);
        }
    }
    return 0;
}


KillCommand::KillCommand(const char *cmd_line)
    : BuiltInCommand(cmd_line)
{}



void KillCommand::execute() {
    if (args[1] == nullptr || args[2] == nullptr || args[3] != nullptr){
        *errorStream << "smash error: kill: invalid arguments\n";
        return;
    }
    SmallShell& smash = SmallShell::getInstance();
    CommandsPack* job;
    int jobId = str2int(args[2]);
    try{
        job = &smash.jobsList.getJobCommandById(jobId);
    }catch(JobDoesntExist&){
        *errorStream << "smash error: kill: job-id " << to_string(jobId) << " does not exist" << "\n";
        return;
    }
    pid_t pid = job->getPid();
    int signal = - str2int(args[1]);
    if (signal <= 0){
        *errorStream << "smash error: kill: invalid arguments\n";
        return;
    }
    if (signal == SIGSTOP){
        smash.jobsList.stopJob(jobId);
    }else if (signal == SIGCONT){
        smash.jobsList.runJob(jobId);
    }else{
        job->sendSig(signal);
    }
    *outputStream << "signal number " << to_string(signal) << " was sent to pid " << to_string(pid) << "\n";
}

CommandsPack::Program::Program(Command& command)
    :command(&command), pipeType(P_NONE), destroyCommand(true), terminated(false)
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
    if (args[1] != nullptr && args[2] != nullptr){
        *errorStream << "smash error: bg: invalid arguments\n";
        return;
    }
    SmallShell& smash = SmallShell::getInstance();
    JobsList& jobsList = smash.jobsList;
    int jobId;
    CommandsPack* job;
    if (args[1] == nullptr){
        try {
            job = &jobsList.getLastStoppedJob(&jobId);
        }catch(NoStoppedJobs&){
            *errorStream << "smash error: bg: there is no stopped jobs to resume\n";
            return;
        }
    }else{
        jobId = str2int(args[1]);
        if(!jobsList.doesExist(jobId)){
            *errorStream << "smash error: bg: job-id " << to_string(jobId) << " does not exist\n";
            return;
        }
        job = &jobsList.getJobCommandById(jobId);
        if(jobsList.isStopped(jobId)){
            *errorStream << "smash error: bg: job-id " << to_string(jobId) << " is already running in the background\n";
            return;
        }
    }
    *outputStream << job->toString() << "\n";
    jobsList.runJob(jobId);
}


ForegroundCommand::ForegroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

void ForegroundCommand::execute() {
    if (args[1] != nullptr && args[2] != nullptr){
        *errorStream << "smash error: fg: invalid arguments\n";
        return;
    }
    SmallShell& smash = SmallShell::getInstance();
    JobsList& jobsList = smash.jobsList;
    int jobId;
    CommandsPack* job;
    if (args[1] == nullptr){
        try{
            job = &jobsList.getLastJob(&jobId);
        }catch(NoJobs&){
            *errorStream << "smash error: fg: jobs list is empty\n";
            return;
        }
    }else{
        jobId = str2int(args[1]);
        if (!jobsList.doesExist(jobId)){
            *errorStream << "smash error: fg: job-id " << to_string(jobId) << " does not exist\n";
            return;
        }
        job = &jobsList.getJobCommandById(jobId);
    }
    *outputStream << job->toString() << "\n";
    jobsList.runJob(jobId);
    jobsList.removeJobByIdToRunInForeground(jobId);
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
   // cout << "pipe output connected fd:" << fd << " outpipe " << outPipe << " pid:" << getpid() << "\n";
    dup2(outPipe, fd);
}

userostream *CommandsPack::Pipe::GetOutputStream() {
   // cout << "pipe output connected pipe:" << outPipe << endl;
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
       // std::cout << "in pipe "<<inPipe << " " << closeInput<< std::endl;

    }
    if (closeOutput){
        close(outPipe);
        //std::cout << "out pipe "<<outPipe << " " << closeOutput<< std::endl;
    }
}


userostream &operator<<(userostream &os, const string &str) {
    int writeResult = write(os.fd, str.c_str(), str.length());
    if (syscallFailed(writeResult)){
        throw SyscallFailure("write");
    }
    return os;

}



userostream::userostream(std::ostream& stream) {
    if (&stream == &std::cout){
        fd = 1;
    }else if (&stream == &std::cerr){
        fd = 2;
    }else{
        throw Unsupported();
    }
}

userostream::~userostream() {
    if (fd > 2){
        close(fd);
    }
}

userostream::userostream(std::string path, RedirectionType type) {
    int flags;
    if (type == R_NORMAL){
        flags = O_WRONLY | O_TRUNC | O_CREAT;
    }else{
        flags = O_WRONLY | O_CREAT | O_APPEND;
    }
    fd = open(path.c_str(), flags,S_IRUSR|S_IWUSR);
    if (syscallFailed(fd)){
        throw SyscallFailure("open");
    }
}

userostream::userostream(int fd) {
    this->fd = fd;
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
            curPipe->dupOutputTo(2);
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
    if(args[1] != nullptr && areEqual(args[1],"kill")){
        JobsList& jobsList = SmallShell::getInstance().jobsList;
        jobsList.update();
        *outputStream << "smash: sending SIGKILL signal to " << to_string(jobsList.getSize()) << " jobs:\n";
        *outputStream << jobsList.toString2();
        jobs->killAllJobs();
    }
}

TouchCommand::TouchCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

void TouchCommand::execute() {
    struct stat info;
        stat(args[1],&info);
    if( !S_ISREG(info.st_mode) ) // S_ISDIR() doesn't exist on my windows
    {
        throw SyscallFailure("utime");
        return;}
    struct tm *d1 = nullptr;
    char **argstouch = nullptr;
    try {
        if (args[1] == nullptr || args[2] == nullptr || args[3] != nullptr) {
            *errorStream << "smash error: touch: invalid arguments\n";
            return ;
        }
        d1 = (tm *) malloc(sizeof(tm));
        if (d1 == nullptr) {
            throw std::bad_alloc();
        }
        struct utimbuf newtime{};
        argstouch = (char **) malloc(sizeof(char *) * 22); //check if malloc succesded
        if (argstouch == nullptr) {
            throw std::bad_alloc();
        }
        int x = _parseCommandLine(args[2], argstouch, timeSeparators);

        d1->tm_sec = str2int(argstouch[0]);
        d1->tm_min = str2int(argstouch[2]);
        d1->tm_hour = str2int(argstouch[4]);
        d1->tm_mday = str2int(argstouch[6]);
        d1->tm_mon = str2int(argstouch[8]) - 1;
        d1->tm_year = str2int(argstouch[10]) - 1900;
        d1->tm_isdst = -1;
        newtime.actime = mktime(d1);
        newtime.modtime = mktime(d1);
        int fd=utime(args[1], &newtime);

        if (syscallFailed(fd)){
            throw SyscallFailure("utime");
        }
        free(d1);
        freeArgs(argstouch);
    }catch(std::bad_alloc& e){
        free(d1);
        if (argstouch != nullptr) {
            freeArgs(argstouch);
        }
        throw e;
    }
}

class fileReader{
private:
    enum BufferExtractionType{B_NL_REACHED, B_BUFFER_ENDED, B_EOF_REACHED};
    int fd;
    bool eofReachedInBuffer;
    bool noMoreLines;
    const static int BUFFER_SIZE = 10;
    char buffer[BUFFER_SIZE];
    int bufferCurIndex;
    int bufferSizeAfterEof;
    int getBufferEnd() const{
        if (eofReachedInBuffer){
            return bufferSizeAfterEof - 1;
        }else{
            return BUFFER_SIZE - 1;
        }
    }
    bool isInBufferBoundaries(int i) const{
        return (i < BUFFER_SIZE && !eofReachedInBuffer) ||
               (i < bufferSizeAfterEof && eofReachedInBuffer);
    }
    const static int NL_NOT_FOUND_IN_BUFFER = -1;
    int getNextNewLineIndexInBuffer() const{
        int i = bufferCurIndex;
        while(isInBufferBoundaries(i)){
            if (buffer[i] =='\n'){
                return i;
            }
            ++i;
        }
        return NL_NOT_FOUND_IN_BUFFER;
    }
    std::string getSubStringFromBuffer(int start, int end){
        return std::string(buffer + start, end - start + 1);
    }
    BufferExtractionType getNextSegmentFromBuffer(std::string& str){
        if (bufferCurIndex >= BUFFER_SIZE) {
            assert(!eofReachedInBuffer);
            str = "";
            return B_BUFFER_ENDED;
        }
        int start = bufferCurIndex;
        int end;
        BufferExtractionType returnVal;
        int nextLineIndex = getNextNewLineIndexInBuffer();
        if (nextLineIndex == NL_NOT_FOUND_IN_BUFFER) {
            end = getBufferEnd();
            if(eofReachedInBuffer){
                returnVal = B_EOF_REACHED;
            }else{
                returnVal = B_BUFFER_ENDED;
            }
        }else{
            end = nextLineIndex - 1;
            bufferCurIndex = nextLineIndex + 1;
            returnVal = B_NL_REACHED;
        }
        int size = end - start + 1;
        str = std::string(buffer + start, size);
        return returnVal;
    }
    void readToBuffer(){
        bufferCurIndex = 0;
        int numOfCharsRead = read(fd, buffer, BUFFER_SIZE);
        if (syscallFailed(numOfCharsRead)){
            throw SyscallFailure("read");
        }
        if (numOfCharsRead < BUFFER_SIZE) {
            eofReachedInBuffer = true;
            bufferSizeAfterEof = numOfCharsRead;
        }
    }
public:
    class EOFReached : public std::exception{};
    explicit fileReader(const string& path){
        fd = open(path.c_str(), O_RDONLY);
        if (fd < 0){
            throw SyscallFailure("open");
        }
        eofReachedInBuffer = false;
        noMoreLines = false;
        try{
            readToBuffer();
        }catch(SyscallFailure& e){
            close(fd);
            throw e;
        }
    }
    ~fileReader(){
        close(fd);
    }
    std::string getNextLine() {
        if (noMoreLines) {
            throw EOFReached();
        }
        string curLine;
        while(true) {
            std::string temp;
            BufferExtractionType type = getNextSegmentFromBuffer(temp);
            curLine += temp;
            if (type == B_NL_REACHED) {
                return curLine;
            } else if (type == B_BUFFER_ENDED){
                readToBuffer();
            } else if (type == B_EOF_REACHED){
                noMoreLines = true;
                return curLine;
            }
        }
        return curLine;
    }
};

template<class T>
class CircularBuffer{
private:

    T** array;
    int size;
    bool isFull;
    int nextAddingPosition;
    int getNextIndex(int i){
        return (i + 1) % size;
    }
    int getPrevIndex(int i){
        return (i - 1 + size) % size;
    }

public:
    CircularBuffer(int n)
    :isFull(false), nextAddingPosition(0), size(n)
    {
        array = new T*[n];
    }
    ~CircularBuffer(){
        int end;
        if (!isFull){
            end = nextAddingPosition - 1;
        }else{
            end = size - 1;
        }
        for(int i = 0; i <= end; i++){
            delete array[i];
        }
        delete array;
    }
    void add(const T& toAdd){
        T* temp = new T(toAdd);
        if (isFull){
            delete array[nextAddingPosition];

        }
        array[nextAddingPosition] = temp;
        nextAddingPosition = getNextIndex(nextAddingPosition);
        if (nextAddingPosition == 0){
            isFull = true;
        }
    }
    std::list<T> getList(){
        std::list<T> list;
        if (!isFull && nextAddingPosition == 0){
            return list;
        }
        int start;
        int end = getPrevIndex(nextAddingPosition);
        if (isFull){
            start = nextAddingPosition;

        }else{
            start = 0;
        }
        int i = start;
        while(true){
            list.push_back(*array[i]);
            if (i == end){
                break;
            }
            i = getNextIndex(i);
        }
        return list;
    }
};
void TailCommand::execute() {
    char * fileName;
    int linesNum;
    if (args[1] == nullptr){
        *errorStream << "smash error: tail: invalid arguments\n";
        return;
    }
    if (args[2] == nullptr){
        linesNum = 10;
        fileName = args[1];
    }else if (args[3] == nullptr && - str2int(args[1]) >= 0){
        linesNum = - str2int(args[1]);
        fileName = args[2];
    }else{
        *errorStream << "smash error: tail: invalid arguments\n";
        return;
    }
    fileReader reader(fileName);
    if (linesNum == 0){
        return;
    }
    CircularBuffer<string> lines(linesNum);
    try{
        while(true){
            lines.add(reader.getNextLine());
        }
    }catch(fileReader::EOFReached&){
        //do nothing
    }
    auto list = lines.getList();
    bool isFirst = true;
    for(const string& line : list){
        if(isFirst){
            isFirst = false;
        }else{
            *outputStream << "\n";
        }
        *outputStream << line;

    }
}

TailCommand::TailCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}


AlarmControl::Entry::Entry(time_t alarmDuration, CommandsPack& cmd)
        :cmd(cmd), terminated(false)
        {
            time_t curTime;
            time(&curTime);
            alarmTime = alarmDuration + curTime;
        }
        bool AlarmControl::Entry::operator<(const Entry& compTo) const{
            return this->alarmTime < compTo.alarmTime;
        }


    void AlarmControl::add(time_t alarmDuration, CommandsPack& cmd){
        Entry entry(alarmDuration, cmd);
        if (entries.empty()){
            alarm(alarmDuration);
        }else if (entries.front().alarmTime > entry.alarmTime){
            alarm(alarmDuration);
        }
        entries.push_back(entry);
        entries.sort();
    }
    void AlarmControl::removeIfContained(CommandsPack& toDelete){
        for(auto i = entries.begin(); i != entries.end(); ++i){
            if (&(i->cmd) == &toDelete){
                i->terminated = true;
                return;
            }
        }
    }
    void AlarmControl::alarmArrived(){
        cout << "smash: got an alarm\n";
        time_t curTime;
        time(&curTime);
        SmallShell::getInstance().jobsList.update();
        auto iList = std::list<std::list<Entry>::iterator>();
        for(auto i = entries.begin(); i != entries.end(); ++i){
            if (i->alarmTime <= curTime){
                iList.push_back(i);
            }else{
                break;
            }
        }
        for(auto i : iList){
            if (!(i->terminated)){
                std::cout << "smash: " << dynamic_cast<Command&>(i->cmd) <<" timed out!\n";
                i->cmd.sendSig(SIGKILL);
            }
            entries.erase(i);
        }
        if (!entries.empty()){
            alarm(entries.front().alarmTime - curTime);
        }
    }

