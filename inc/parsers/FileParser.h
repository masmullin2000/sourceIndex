#pragma once

#include <memory>
#include <forward_list>

#include "FileProcessor.h" // for Token
#define MAX_WORD_SZ 128


using namespace std;

enum class FILE_TYPES {
  PLAINTEXT    = 0,
  C,
  CPP,
  JAVA,
  XML
};

class FileParser
{
public:
  FileParser( const string& fileName ) : _fName(fileName) {}
  virtual     ~FileParser(){};
  virtual      shared_ptr<forward_list<Token>> parse();
protected:
  bool         getFileContents(char** fileContents, uint64_t* sz);
  const string&   _fName;
};

shared_ptr<FileParser>
Make_Parser
(
  string &fileName
);