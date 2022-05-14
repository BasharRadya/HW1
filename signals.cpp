#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    SmallShell& smash = SmallShell::getInstance();
    std::cout << "smash: got ctrl-Z\n";
    if (smash.cur == nullptr){
        return;
    }

    std::cout << "smash: process " << smash.cur->getPid() << " was stopped\n";
    smash.cur->sendSig(SIGSTOP);


}

void ctrlCHandler(int sig_num) {
    SmallShell& smash = SmallShell::getInstance();
    std::cout << "smash: got ctrl-C\n";
    if (smash.cur == nullptr){
        return;
    }
    std::cout << "smash: process " << smash.cur->getPid() << " was killed\n";
    smash.cur->sendSig(SIGKILL);

}

void alarmHandler(int sig_num) {
    SmallShell& smash = SmallShell::getInstance();
    smash.alarmControl.alarmArrived();
}

