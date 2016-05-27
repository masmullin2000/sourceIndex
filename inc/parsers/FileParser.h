#pragma once

#include <memory>
#include <forward_list>

#include "FileProcessor.h" // for Token

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
  FileParser( const string& fileName );
  virtual     ~FileParser(){};
  virtual      shared_ptr<forward_list<Token>> parse();
private:
  const string&   _fName;
};

shared_ptr<FileParser>
Make_Parser
(
  string &fileName
);