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
    int _count;
public:
    Console() : _count(0){}

    void PrintCommand(const string &_str){
        if (_count == 0) PrintHead();
        _count++;
        cout << _str;
    }

    void PrintHead(){cout << "bulk: ";}
    void PrintComma(){cout << ",";}

    void PrintEnd(){
        _count = 0;
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
    static int count;
public:
    PrintCommand(shared_ptr<Console> _console, string _cmd) : ConsoleCommand(std::move(_console)), cmd(std::move(_cmd)){
        count++;
    }

    void Execute() override {
        console->PrintCommand(cmd);
        count--;
        if (count != 0) console->PrintComma();
        else console->PrintEnd();
    }
};

class LogWriteCommand : public LogCommand{
private:
    string cmd;
    static int count;
public:
    LogWriteCommand(shared_ptr<Log> _lg, string _cmd) : LogCommand(std::move(_lg)), cmd(std::move(_cmd)) {
        count++;
        if (count == 1) log->SetCurrentBlockTime();
    }

    void Execute() override {
        log->Write(cmd);
        count--;
        if (count == 0) log->Close();
    }
};

int LogWriteCommand::count = 0;
int PrintCommand::count = 0;

class Commands{
private:
    vector<shared_ptr<ICommand>> commands;

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
        if (cmd != OPEN_DYNAMIC && cmd != CLOSE_DYNAMIC){
            commands.emplace_back(new PrintCommand(console, cmd));
            commands.emplace_back(new LogWriteCommand(lg, cmd));
        }
        if (commands.size() >= 2*N) Execute();
    }

    void Execute(){
        if (commands.empty()) return;

        for (auto &it : commands){
            it->Execute();
        }

        PostActions();
    }

    void PostActions(){
        commands.clear();
    }

    void Exit(){
        if (state == 0) Execute();
        console.reset();
        lg.reset();
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
