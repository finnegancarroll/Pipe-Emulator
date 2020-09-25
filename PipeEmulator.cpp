//Finnegan Carroll

#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

using namespace std;

struct cmd{
  string command;
  vector<string> args;
};

//Gets one line of input from the user. Returns string.
string getInput();
//Takes in a string, and parses into tokens. Returns vector of strings.
vector<string> parseTokens(string input);
//Takes vector of strings and sorts them into commands. Returns a vector of commands.
vector<cmd> getCommands(vector<string> list);
//Deletes extra whitespace and quotes off a string.
string stripQuotes(string s);

const string CMDLINESTART = "prompt$";
const char SPACE = ' ';
const char PIPE = '|';
const char Q1 = '\'';
const char Q2 = '\"';
const int STDIN = 0;//stdin file descriptor
const int STDOUT = 1;//stdout file descriptor
const int MAXARGS = 50;//Max number of arguments per command
const int MAXBYTES = 1000;//Max number of bytes read from command output

int main(){
  //These two lines get user input and sort it into a vector of command structs
  vector<string> stringCmds = parseTokens(getInput());
  vector<cmd> cmdList = getCommands(stringCmds);
  
  int pid = 0;//Set to zero so first fork happens
  int pipeRW[2];
  int cmdId = -1;//So the original parent can identify itself.
  
  for(unsigned int i = 0; i < cmdList.size(); i++){
    if(pid == 0){//Parent Process on 1st run, child on all other
      pipe(pipeRW);
      pid = fork();
      
      if(pid == 0){//The child process will return 0
        dup2(pipeRW[1], STDOUT);//Make child's stdout point to the same location as
                                //pipeRW[1], or the pipes write.
        close(pipeRW[1]);       //Close child's file descriptor connected to pipe write.  
        close(pipeRW[0]);       //Close child's file descriptor connected to pipe read.
        cmdId = i;              //Saves the command id to be executed by this child.
      }else if(pid > 0){//The parent will return an actual process id.
        dup2(pipeRW[0], STDIN); //Make parent's stdin point to the same location as
                                //pipeRW[0], or the pipes read.
        close(pipeRW[1]);       //Close parent's file descriptor connected to pipe write.  
        close(pipeRW[0]);       //Close parent's file descriptor connected to pipe read.   
      }
    }
  } 
  
  int* status = nullptr;
  while(wait(status) != -1){}//Wait on child processes
  
  if(cmdId != -1){//Children code
    char* param[MAXARGS];
    cmd cmdStruct = cmdList[((cmdList.size() - 1) - cmdId)];
    //This line gets the command struct for the given child.
    //The youngest child runs the furthest left command.  
    param[0] = (char*)(cmdStruct.command).c_str();
    for(unsigned int i = 0; i < cmdStruct.args.size(); i++){
      param[i + 1] = (char*)(cmdStruct.args[i]).c_str();
    }
    param[cmdStruct.args.size() + 1] = NULL;//Last arg must be null
    
    execvp(param[0], param);//Replace code with command param[0] with arguments param
    
  }else{//Parent code 
    char output[MAXBYTES];
    int bytesRead = read(STDIN, output, MAXBYTES);//Read returns how many bytes were read
    for(int i = 0; i < bytesRead; i++){
      cout << output[i];
    }
    cout << endl;
  }
}

//Short function to grab raw user input
string getInput(){
  string inputLine;
  cout << CMDLINESTART;
  getline(cin, inputLine);
  return inputLine;
}

//Takes in user input and returns list of token strings
vector<string> parseTokens(string input){
  vector<string> tokenList;
  string::iterator tstart = input.end();
  
  for(string::iterator it = input.begin(); it < input.end(); it++){
    if(tstart == input.end()){//Not searching
      if(*it == PIPE){//Space then pipe case
        tokenList.push_back(string(it, it + 1));
      }else if(*it != SPACE){//Start searching if anything other than space found
        tstart = it;
        if(it == input.end() - 1){
          tokenList.push_back(string(it, it + 1));
        }
      }
    }else if(*tstart == Q1 || *tstart == Q2){//Searching for quotes/double quotes
      if(*it == *tstart){
        tokenList.push_back(string(tstart, it + 1));
        tstart = input.end();
      }
    }else if(*it == SPACE || it == input.end() - 1 || *it == PIPE){//Searching for pipe/space/end of string
      if(*it == PIPE){
        tokenList.push_back(string(tstart, it));
        tokenList.push_back("|");
        tstart = input.end();
      }else{//A space and end of string both act as boundaries for the token
        tokenList.push_back(string(tstart, it + 1));
        tstart = input.end();
      }
    }
  }   
  return tokenList;
}

//Sorts a list of tokens into a list of command structs
vector<cmd> getCommands(vector<string> list){  
  vector<cmd> cmdList;
  int argNum = -1;
  int cmdNum = 0;
  cmd in;
  for(unsigned int i = 0; i  < list.size(); i++){
    if(list[i] == "|"){//Bars mean a new command
      cmdNum++;
      argNum = -1;
    }else if(argNum == -1){//The first string is the command
      in.command = stripQuotes(list[i]); 
      cmdList.push_back(in);   
      argNum++;
    }else if(argNum > -1){//Following strings are arguments
      cmdList[cmdNum].args.push_back(stripQuotes(list[i])); 
      argNum++;
    }
  }
  return cmdList;
}

//Strips extra white space and quotes from strings
string stripQuotes(string s){
  if(*(s.begin()) == *(s.end() - 1)){
    if(*(s.begin()) == Q1 || *(s.begin()) == Q2){
      s.erase(s.begin());
      s.erase(s.end() - 1);
    }
  }
  for(string::iterator it = s.begin(); it < s.end(); it++){
    if(isspace(*it)){
      s.erase(it);
    }
  }
  return s;
}