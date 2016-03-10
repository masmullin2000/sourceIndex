#include <iostream>
#include <functional>

#define FILE_DATABASE "files.db"
#define IDENT_DATABASE "idents.db"
#define LOCS_DATABASE "locs.db"

#include "FileProcessor.h"
#include "FileList.h"

using namespace std;

int main( int argc, char** argv )
{
  int threadAmt = 256;
  if( argc < 2 ) {
    cout
      << "usage: "
      << endl
      << "  "
      << argv[0]
      << " <file>"
      << endl;

    return 1;
  } else if( argc == 3 ) {
    threadAmt = atoi(argv[2]);
  }

  FileProcessor fp;
  string fList = argv[1];
  fp.run( fList, threadAmt );

  return 0;
}