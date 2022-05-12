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

enum RedirectionType{R_NONE, R_NORMAL, R_APPEND};
enum PipeType{P_NONE, P_NORMAL, P_ERR};

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
    bool redirectionDetected(int curArg) const;
public:
    bool doesRunInBackground;
    bool doesNeedFork; //default value is false
  explicit Command(const char* cmd_line, bool areArgsReady = false , Args readyArgs =Args(EMPTY_ARGS));
  virtual ~Command(){};
  virtual void execute() = 0;
  std::string toString2() const;
  friend std::ostream& operator<<(std::ostream& os, const Command& cmd);
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};
std::ostream& operator<<(std::ostream& os, const Command& cmd);

class userostream{
private:
    int fd;
    std::ostream* stream;
    bool cStyleFile;
public:
    explicit userostream(int fd);
    explicit userostream(std::ostream& stream);
    ~userostream();
    friend userostream& operator<<(userostream& os, const std::string& string);
};
userostream& operator<<(userostream& os, const std::string& str);


class BuiltInCommand : public Command {

protected:
    userostream * outputStream;
    userostream * errorStream;
public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand();
  void setRedirection(const std::string& file, RedirectionType type);
  void setOutPipe(userostream& os, PipeType type);
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
    class Program{
    public:
        Command* command;
        PipeType pipeType;
        bool destroyCommand;
        bool terminated;
        Program(Command& commmand);
        void dontDestroyCommand();
        ~Program();
        bool isLast() const;
    };
    class Pipe{
    private:
        int inPipe;
        int outPipe;
        bool closeInput;
        bool closeOutput;
    public:
        Pipe();
        ~Pipe();
        void dupInputTo(int fd);
        void dupOutputTo(int fd);
        userostream* GetOutputStream();
        void SetToCloseInput();
        void SetToCloseOutput();
        void SetNotToCloseInput();
        void SetNotToCloseOutput();
    };

    class RedirectionControl{
        Program* curProgram;
        Pipe* curPipe;
        Pipe* prevPipe;
        void createPipeIfNeeded();
        void setCurProgramInPipeIfNeeded();
        void setCurProgramOutPipeIfNeeded();
        void setPipingIfNeeded();
        void cleanupCurProgramPipe();
        void dontCleanupCurProgramPipe();
        bool isSettingInPipeNeeded();
        bool isSettingOutPipeNeeded();
    public:
        RedirectionControl();
        ~RedirectionControl();
        void prepareFor(Program& program);
        void setRedirectionIfNeeded(const std::string& outFile, RedirectionType type);
        void inFather();
        void inSon();
        void builtIn();
    };
    std::list<Program> programs;
    std::string* inFile;
    std::string outFile;
    RedirectionType outFileType;
    int processesNum;
    bool quitExists;
    Command * CreateCommand(const char* cmd_line);
    bool isCmdLegal() const;
    bool endOfTextDetected(int curArg) const;
    int setRedirection(int curArg);
    int addProgram(int curArg);
public:

    CommandsPack(const char* cmd_line);
    ~CommandsPack() override;
  void execute() override;

    bool isSingleProgram() const;
    pid_t getPid() const;

    int wait();
    void sendSig(int signum);
    bool quitDone() const;
    std::string toString() const;
    std::string toString2() const;

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

// TODO: Add your data
    JobsList* jobs;
    public:
    QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};




class JobsList {
 public:
 // TODO: Add your data members
private:
    class JobEntry {
    public:
        CommandsPack& command;
        bool isStopped;
        int jobId;
        time_t time_insert;

        JobEntry(CommandsPack& command,bool isStopped,int jobId,time_t time_insert);
        bool operator<(JobEntry& job) const;
        friend std::ostream& operator<<(std::ostream& os, const JobEntry& job);
        std::string toString() const;

    };
    std::list<JobEntry> jobsList;
    JobEntry * getJobById(int jobId);
    const JobEntry * getJobById(int jobId) const;

public:
   JobsList();
  ~JobsList(){};
  void addJob(CommandsPack* cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void update();
  CommandsPack& getJobCommandById(int jobId);
  void removeJobByIdToRunInForeground(int jobId);
  CommandsPack& getLastJob(int* jobId);
  CommandsPack& getLastStoppedJob(int* jobId);
  void StopJob(int jobId);
  void RunJob(int jobId);
  bool doesExist(int jobId) const;
  bool isStopped(int jobId) const;
  std::string toString2() const;
  // TODO: Add extra methods or modify exisitng ones as needed
  //JobEntry& operator[](int x) ;
    std::string toString() const;
  friend std::ostream& operator<<(std::ostream& os, const JobsList& list);
  friend std::ostream& operator<<(std::ostream& os, const JobEntry& job);
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
  ForegroundCommand(const char* cmd_line);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line);
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
    void wait();
};


class SyntaxError : public std::exception{};
class SyscallFailure : public std::exception{};
class JobDoesntExist : public std::exception{};
class FileError : public std::exception{};
class NoJobs : public std::exception{};
class NoStoppedJobs : public std::exception{};
class ProgramEnded : public std::exception{};
class NoForkNeededInPack : public std::exception{};
#endif //SMASH_COMMAND_H_
