#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    SmallShell& smash = SmallShell::getInstance();
    if (smash.cur == nullptr){
        return;
    }
    smash.cur->sendSig(SIGSTOP);

}

void ctrlCHandler(int sig_num) {
    SmallShell& smash = SmallShell::getInstance();
    if (smash.cur == nullptr){
        return;
    }
    smash.cur->sendSig(SIGKILL);
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

