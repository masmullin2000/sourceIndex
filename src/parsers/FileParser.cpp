#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "utils.h"


#include "boost/filesystem.hpp"

#include "FileParser.h"

#define MAX_WORD_SZ 128

using namespace std;
namespace fs = boost::filesystem;

shared_ptr<FileParser>
Make_Parser
(
  string &fileName
)
{
  shared_ptr<FileParser>  rc;
  fs::path file(fileName);

  if( file.extension().compare(".h") == 0 ) {
    rc = make_shared<FileParser>(fileName);
  } else {
    rc = make_shared<FileParser>(fileName);
  }
  
  return rc;
}
  
FileParser::FileParser
(
  const string& fileName
) : _fName(fileName)
{
  ;
}

shared_ptr<forward_list<Token>>
FileParser::parse()
{
  shared_ptr<forward_list<Token>> toks = nullptr;
  
  if( _fName.length() < 1 ) {
    return toks;
  }

  int fd = open(_fName.c_str(), O_RDONLY);
  if( fd == -1 ) {
    close(fd);
    return toks;
  }

  struct stat sb;
  if( fstat(fd,&sb) == -1 ) {
    close(fd);
    return toks;
  }

  if( !S_ISREG(sb.st_mode) ) {
    close(fd);
    return toks;
  }

  char* file = (char*)mmap(0,sb.st_size,PROT_READ,MAP_PRIVATE,fd,0);
  if( file == MAP_FAILED ) {
    close(fd);
    return toks;
  }

  close(fd);
  toks = make_shared<forward_list<Token>>();

  char word[MAX_WORD_SZ];
  word[0] = '\0';
  int j = 0;
  int line = 1;
#define c file[i]
  for( int i = 0; i < sb.st_size; i++ ) {
    if( (IsIdentifierNonDigit(c) || IsDigit(c)) && j < MAX_WORD_SZ-1 ) {
      word[j++] = c;
    } else {
      word[j] = '\0';
      j = 0;
      if( word[0] != '\0' && !IsDigit(word[0]) ) {
        Token t;
        t.word = string(word);
        t.line = line;
        toks->emplace_front(t);
      }
      if( c == '\n' ) line++;
    }
  }
#undef c
  munmap(file,sb.st_size);

  return toks;
}