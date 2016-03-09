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

  //cout << lhs.str() << " vs " << rhs.str() << " is " << rc << endl;
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
#if 1
  stringstream sqls;
  sqls << "INSERT INTO Files (name) VALUES('" << _fileName << "');";

  /*if( SQLITE_OK != sqlite3_exec(db,sqls.str().c_str(),0,0,0) ) {
    cerr << "could not insert file [" <<sqls.str().c_str() << "]" << endl;
    return FileProcessorErrors::SQL_INSERT;
  }*/
  
  unique_lock<mutex> lk(primaryKeyMutex);
  sqlite3_stmt* iStmt;
  if( SQLITE_OK != sqlite3_prepare_v2(db,sqls.str().c_str(),-1,&iStmt,0) ) {
    cerr << "could not insert file [" <<sqls.str().c_str() << "]" << endl;
    return FileProcessorErrors::SQL_INSERT;
  }
  
  sqlite3_step(iStmt);
  //cout << x;
  sqlite3_finalize(iStmt);
  
  uint64_t pk = sqlite3_last_insert_rowid(db);
  //cout << pk << endl;
  lk.unlock();
#else
uint64_t pk = 0;
#endif
  
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
        //sqlite3_bind_int(idStmt,2,pk);
#if 0
        sqlite3_bind_text(idStmt,2,word,-1,SQLITE_STATIC);
        
        int rc = sqlite3_step(idStmt);
        
        //cout << pk << ":" << word << endl;
        
        //cout << sqlite3_errmsg(db) << endl;
        sqlite3_clear_bindings(idStmt);
        sqlite3_reset(idStmt);
#endif
        Identifier i(string(word),pk,line);
        lst.push_front(i);

        //cout << words.size() << endl;
        //words.insert(string(word));
        //stringstream inSql;
        
        //inSql << "INSERT INTO Identifiers (fk_files,name) VALUES ("
        //     << pk << ",'" << word << "');";
        //if( SQLITE_OK != sqlite3_exec(db,inSql.str().c_str(),0,0,0) ) {
          //cerr << "identifiers no insert::" << inSql.str().c_str() << endl;
          //return FileProcessorErrors::SQL_INSERT;
        //}
      }
    }
  }
#if 1
  unique_lock<mutex> sim(setInsertMutex);
  ids.push_front(std::move(lst));
  /*for( Identifier i: lst ) {
    ids.insert(i);
  }*/
  sim.unlock();
#endif
  
  //sqlite3_exec(db,"END TRANSACTION",0,0,0);

#undef c
  munmap(file,sb.st_size);

  return FileProcessorErrors::SUCCESS;
}
