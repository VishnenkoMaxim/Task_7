#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <cstring>
#include <errno.h>
#include <chrono>
#include <ctime>

using namespace std;

#define OPEN_DYNAMIC "{"
#define CLOSE_DYNAMIC "}"

class Console {
private:
    int count;
public:
    Console() : count(0){}

    void PrintCommand(const string &_str){
        if (count == 0) PrintHead();
        count++;
        cout << _str;
    }

    void PrintHead(){cout << "bulk: ";}
    void PrintComma(){cout << ",";}

    void PrintEnd(){
        count = 0;
        cout << endl;
    }
};

class Log {
private:
    int fd;
    time_t _time;
public:
    Log() : fd(-1), _time() {}

    void Write(string &_str){
        if (fd == -1){
            char buf[64];
            sprintf(buf,"%ld",_time);
            string file_name = "bulk";
            file_name.append(buf);
            file_name.append(".log");
            fd = open(file_name.c_str(), O_WRONLY | O_CREAT);
        }
        if (fd > 0){
            _str += "\n";
            write(fd, _str.c_str(), _str.size());
        }
    }

    void Close(){
        close(fd);
        fd = -1;
    }

    void SetCurrentBlockTime(){
        _time = time(nullptr);
    }
};

class ICommand{
public:
    virtual void Execute() = 0;
    virtual ~ICommand() = default;
};

class ConsoleCommand: public ICommand{
protected:
    shared_ptr<Console> console;
    explicit ConsoleCommand(shared_ptr<Console> _console): console(std::move(_console)) {}
public:
    ~ConsoleCommand() override = default;
};

class LogCommand: public ICommand{
protected:
    shared_ptr<Log> log;
    explicit LogCommand(shared_ptr<Log> _log): log(std::move(_log)) {}
public:
    ~LogCommand() override = default;
};

class PrintCommand: public ConsoleCommand {
    string cmd;
public:
    PrintCommand(shared_ptr<Console> _console, string _cmd) : ConsoleCommand(std::move(_console)), cmd(std::move(_cmd)){}

    void Execute() override {
        console->PrintCommand(cmd);
    }
};

class LogWriteCommand : public LogCommand{
private:
    string cmd;
public:
    LogWriteCommand(shared_ptr<Log> _lg, string _cmd) : LogCommand(std::move(_lg)), cmd(std::move(_cmd)) {}

    void Execute() override {
        log->Write(cmd);
    }
};

class Commands{
private:
    vector<shared_ptr<ConsoleCommand>> commands;
    vector<shared_ptr<LogCommand>> log_commands;
    unsigned short N;
    int state;

    shared_ptr<Console> console;
    shared_ptr<Log> lg;
public:
    explicit Commands(const unsigned short n) : N(n), state(0) {
        console = std::make_shared<Console>();
        lg = std::make_shared<Log>();
    }

    void Add(string &cmd){
        if (cmd == OPEN_DYNAMIC){
            if (++state <= 1){
                Execute();
                return;
            }
        } else if (cmd == CLOSE_DYNAMIC){
            if (--state <= 0){
                Execute();
                state = 0;
                return;
            }
        }
        if (cmd != OPEN_DYNAMIC && cmd != CLOSE_DYNAMIC) {
            commands.emplace_back(new PrintCommand(console, cmd));
            log_commands.emplace_back(new LogWriteCommand(lg, cmd));
            if (log_commands.size() == 1) lg->SetCurrentBlockTime();
        }
        if (commands.size() >= N) Execute();
    }

    void Execute(){
        if (commands.empty()) return;

        unsigned int i = 0;
        for (auto &it : commands){
            it->Execute();
            if (i < commands.size()-1) console->PrintComma();
            ++i;
        }
        console->PrintEnd();

        for (auto &it : log_commands){
            it->Execute();
        }
        lg->Close();

        PostActions();
    }

    void PostActions(){
        commands.clear();
        log_commands.clear();
    }

    void Exit(){
        if (state == 0) Execute();
        console.reset();
    }
};

int main(int argc, char  **argv) {
    unsigned short N;

    if (argc < 2){
        N = 3;
    } else N = atoi(argv[1]);

    Commands commands(N);

    while(true){
        string cmd;
        cin >> cmd;
        if (cin.eof()){
            commands.Exit();
            return 0;
        }
        commands.Add(cmd);
    }

    return 0;
}
