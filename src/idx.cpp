#include <iostream>
#include <functional>
#include <thread>

#include <tclap/CmdLine.h>

#include "FileProcessor.h"
#include "FileList.h"

using namespace std;
using namespace TCLAP;

int main( int argc, char** argv )
{
  try {
    int threadAmt = thread::hardware_concurrency();
    string fList;

    CmdLine cmd("", ' ', "0.1");

    ValueArg<string> file_list_param("l","fileList","A list of files to be processed",true,"f.lst","string");
    ValueArg<int> threadAmt_list_param("j","threads","How many threads to use (default is #of cpu threads)",false,-1,"integer");
    SwitchArg massive_memory_param("m","massive_memory","set to use massive amounts of memory to speed up database creation",false);
    SwitchArg index_param("i","no_index_database","Speed database creation, slows searching",false);

    cmd.add(file_list_param);
    cmd.add(threadAmt_list_param);
    cmd.add(massive_memory_param);
    cmd.add(index_param);

    cmd.parse(argc,argv);

    fList = file_list_param.getValue();
    if( threadAmt_list_param.getValue() > 0 ) {
      threadAmt = threadAmt_list_param.getValue();
    }
    bool massive_memory = massive_memory_param.getValue();
    bool idx = index_param.getValue();

    FileProcessor fp;
    fp.run( fList, threadAmt, massive_memory, !idx );
  } catch( ArgException &e ) {
    cerr << "error " << e.error() << endl;
  }

  return 0;
}