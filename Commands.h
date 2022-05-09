    #ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <ctime>
#include <list>
#include <stack>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
const static int FAILURE = -1;
static const int MAX_ARGS_NUM = 21;


class Args{
    char** obj;
public:
    Args(char const* const*  args);
    Args(const Args& args);
    ~Args();
    char const* const* getObj() const;
};

const static char* EMPTY_ARGS[1] = {nullptr};

class CommandsPack;

class Command {
// TODO: Add your data members
    const std::string cmd_line;
protected:
    static const int MAX_ARGS_NUM = 21;
    char** args;
public:
    bool doesRunInBackground;
    bool doesNeedFork; //default value is false
  explicit Command(const char* cmd_line, bool areArgsReady = false , Args readyArgs =Args(EMPTY_ARGS));
  virtual ~Command(){};

  virtual void execute() = 0;
  friend std::ostream &operator<<(std::ostream &os, const CommandsPack &cmd);
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
protected:
    std::istream * inputStream;
    std::ostream * outputStream;
public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
private:
    Args getModifiedLine(const char * cmd_line) const;
public:
        pid_t pid;
        ExternalCommand(const char* cmd_line);
        virtual ~ExternalCommand() {}
        void execute() override;
    };

class CommandsPack : public Command {
private:
    enum RedirectionType{R_NONE, R_NORMAL, R_APPEND};
    enum PipeType{P_NONE, P_NORMAL, P_ERR};
    class Program{
    public:
        ExternalCommand command;
        PipeType pipeType;
        Program(ExternalCommand commmand);
        bool isLast() const;
    };
    std::list<Program> programs;
    std::string* inFile;

    std::string outFile;
    RedirectionType outFileType;
    int processesNum;
    bool isCmdLegal() const;
    bool endOfTextDetected(int curArg) const;
    bool redirectionDetected(int curArg) const;
    int setRedirection(int curArg);
    int addProgram(int curArg);
public:

    CommandsPack(const char* cmd_line);
    ~CommandsPack() override = default;
  void execute() override;

    bool isSingleProgram() const;
    pid_t getPid() const;

    int wait();
    void sendSig(int signum);
    friend std::ostream &operator<<(std::ostream &os, const CommandsPack &cmd);
};

    std::ostream &operator<<(std::ostream &os, const CommandsPack &cmd);
class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
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

    class ChangePromptCommand : public BuiltInCommand {
// TODO: Add your data members
      public:
        ChangePromptCommand(const char* cmd_line);
        virtual ~ChangePromptCommand() {}
        void execute() override;
    };

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
private:
    const static int CHANGE_FAILURE = -1;
public:
  ChangeDirCommand(const char* cmd_line);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
  static std::string getCurDir();
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class BackgroundCommand;

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};




class JobsList {
 public:
  class JobEntry {
  public:
      CommandsPack& command;
      bool isStopped;
      int jobId;
      time_t time_insert;

   JobEntry(CommandsPack& command,bool isStopped,int jobId,time_t time_insert);
   bool operator<(JobEntry& job) const;
   friend std::ostream& operator<<(std::ostream& os, const JobEntry& job);
  };
 // TODO: Add your data members
private:
    std::list<JobEntry> jobsList;
public:
   JobsList();
  ~JobsList(){};
  void addJob(CommandsPack* cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  void updateJobStatusAsStopped(int jobId);
  void updateJobStatusAsRunning(int jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
  JobEntry& operator[](int x) ;
  friend std::ostream& operator<<(std::ostream& os, const JobsList& list);

};


std::ostream& operator<<(std::ostream& os, const JobsList::JobEntry& job);
std::ostream& operator<<(std::ostream& os, const JobsList& list);
class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char *cmd_Line);
  virtual ~JobsCommand() {}
  void execute() override;


};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line);
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

class TailCommand : public BuiltInCommand {
 public:
  TailCommand(const char* cmd_line);
  virtual ~TailCommand() {}
  void execute() override;
};

class TouchCommand : public BuiltInCommand {
 public:
    int test1;
  TouchCommand(const char* cmd_line);
  virtual ~TouchCommand() {}
  void execute() override;
};


class SmallShell {
 private:
  // TODO: Add your data members
  std::string* prevDir;
  SmallShell();
 public:
    std::string prompt;
    JobsList jobsList;
    CommandsPack* cur;
    bool prevDirExists();
    std::string getPrevDir();
    void changePrevDir(std::string prev);
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
