#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "utils.h"

#include "boost/filesystem.hpp"

#include "FileParser.h"
#include "CParser.h"

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

  if( (file.extension().compare(".c") == 0)     ||
      (file.extension().compare(".cc") == 0)     ||
      (file.extension().compare(".h") == 0)     ||
      (file.extension().compare(".cpp") == 0)   ||
      (file.extension().compare(".cxx") == 0)   ||
      (file.extension().compare(".hpp") == 0)) {
    rc = make_shared<CParser>(fileName);
  } else if( (file.extension().compare(".java") == 0) ) {
    rc = make_shared<JavaParser>(fileName);
  } else {
    rc = make_shared<FileParser>(fileName);
  }
  
  return rc;
}

bool
FileParser::getFileContents
(
  char**      fileContents,
  uint64_t*   sz
)
{
  *fileContents = nullptr;
  *sz = 0;

  int fd = open(_fName.c_str(), O_RDONLY);
  if( fd == -1 ) {
    return false;
  }

  struct stat sb;
  if( fstat(fd,&sb) == -1 ) {
    close(fd);
    return false;
  }
  *sz = sb.st_size;

  if( !S_ISREG(sb.st_mode) ) {
    close(fd);
    return false;
  }

  *fileContents = (char*)mmap(0,*sz,PROT_READ,MAP_PRIVATE,fd,0);
  if( fileContents == MAP_FAILED ) {
    close(fd);
    return false;
  }

  close(fd);
  return true;
}

shared_ptr<forward_list<Token>>
FileParser::parse()
{
  shared_ptr<forward_list<Token>> toks = nullptr;
  
  if( _fName.length() < 1 | _fName == "." ) {
    return toks;
  }
  char* file = nullptr;
  uint64_t sz = 0;
  getFileContents( &file, &sz );

  toks = make_shared<forward_list<Token>>();

  char word[MAX_WORD_SZ];
  word[0] = '\0';
  int j = 0;
  int line = 1;
#define c file[i]
  for( uint64_t i = 0; i < sz; i++ ) {
    if( file != nullptr ) {
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
    }}
  }
#undef c
  munmap(file,sz);

  return toks;
}