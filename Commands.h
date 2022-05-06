    #ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <ctime>
#include <list>
#include <stack>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
// TODO: Add your data members

  const std::string cmd_line;
protected:
    static const int MAX_ARGS_NUM = 21;
    char** args;
public:
  Command(const char* cmd_line);
  virtual ~Command(){};
  virtual void execute() = 0;
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
 public:
    int pid;
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
};

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
      ExternalCommand& command;
      bool isStopped;
      int jobId;
      time_t time_insert;
   JobEntry(ExternalCommand& command,bool isStopped,int jobId,time_t time_insert);
   bool operator<(JobEntry& job) const;
  };
 // TODO: Add your data members
private:
    std::list<JobEntry> jobsList;
public:
   JobsList();
  ~JobsList(){};
  void addJob(ExternalCommand* cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

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
    ForegroundCommand* cur;
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
