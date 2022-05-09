#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    cout << "ctrlz\n";
    SmallShell& smash = SmallShell::getInstance();
    if (smash.cur == nullptr){
        return;
    }
    smash.cur->sendSig(SIGSTOP);
    smash.jobsList.addJob(smash.cur, true);

}

void ctrlCHandler(int sig_num) {
    cout << "ctrlc\n";
    SmallShell& smash = SmallShell::getInstance();
    if (smash.cur == nullptr){
        return;
    }
    smash.cur->sendSig(SIGKILL);
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

