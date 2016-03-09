#include "FileProcessor.h"
#include "utils.h"

#include <iostream>
#include <sstream>
#include <list>


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>
#include <sqlite3.h>

using namespace std;

extern sqlite3 *db;
extern sqlite3_stmt *idStmt;

bool operator<( const Identifier& lhs, const Identifier& rhs ) {
  bool rc = true;
  if( lhs.word.compare(rhs.word) > 0 ) rc = false;

  return rc;
}

list<list<Identifier>> ids;

#define MAX_WORD_SZ 128

FileProcessor::FileProcessor()
{
  ;//_fileName = nullptr;
}

FileProcessor::FileProcessor
(
  string fileName
)
{
  setFile(fileName);
}

void
FileProcessor::setFile
(
  string fileName
)
{
  _fileName = fileName;
}

FileProcessorErrors
FileProcessor::run()
{
  if( _fileName.length() < 1 )
    return FileProcessorErrors::FILE_NOT_FOUND;

  int fd = open(_fileName.c_str(), O_RDONLY);
  if( fd == -1 )
    return FileProcessorErrors::FILE_NOT_FOUND;

  struct stat sb;
  if( fstat(fd,&sb) == -1 )
    return FileProcessorErrors::FILE_NO_STATUS;

  if( !S_ISREG(sb.st_mode) )
    return FileProcessorErrors::FILE_NOT_FOUND;

  char* file = (char*)mmap(0,sb.st_size,PROT_READ,MAP_PRIVATE,fd,0);
  if( file == MAP_FAILED )
    return FileProcessorErrors::FILE_MMAP_ERROR;

  close(fd);
  stringstream sqls;
  sqls << "INSERT INTO Files (name) VALUES('" << _fileName << "');";

  unique_lock<mutex> lk(primaryKeyMutex);
  sqlite3_stmt* iStmt;
  if( SQLITE_OK != sqlite3_prepare_v2(db,sqls.str().c_str(),-1,&iStmt,0) ) {
    cerr << "could not insert file [" <<sqls.str().c_str() << "]" << endl;
    return FileProcessorErrors::SQL_INSERT;
  }

  sqlite3_step(iStmt);
  sqlite3_finalize(iStmt);

  uint64_t pk = sqlite3_last_insert_rowid(db);
  lk.unlock();

  char word[MAX_WORD_SZ];
  word[0] = '\0';
  int j = 0;
  int line = 1;
  list<Identifier> lst;

#define c file[i]
  for( int i = 0; i < sb.st_size; i++ ) {
    //char c = file[i];
    if( (IsIdentifierNonDigit(c) || IsDigit(c)) && j < MAX_WORD_SZ-1 ) {
      word[j++] = c;
    } else {
      if( c == '\n' ) line++;
      word[j] = '\0';
      j = 0;
      if( word[0] != '\0' && !IsDigit(word[0]) ) {

        Identifier i(string(word),pk,line);
        lst.push_front(i);
      }
    }
  }
  unique_lock<mutex> sim(setInsertMutex);
  ids.push_front(std::move(lst));

  sim.unlock();
#undef c
  munmap(file,sb.st_size);

  return FileProcessorErrors::SUCCESS;
}
