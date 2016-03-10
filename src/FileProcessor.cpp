#include "FileProcessor.h"
#include "utils.h"

#include <iostream>
#include <sstream>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sqlite3.h>

using namespace std;

#define MAX_WORD_SZ 128

FileProcessorErrors
FileProcessor::run
(
  sqlite3                          *db,
  string                            fName,
  unordered_map<string,uint32_t>   &ids,
  forward_list<Location>           &locs,
  uint32_t                         &id_key
)
{
  if( fName.length() < 1 )
    return FileProcessorErrors::FILE_NOT_FOUND;

  int fd = open(fName.c_str(), O_RDONLY);
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
  sqls << "INSERT INTO Files (name) VALUES('" << fName << "');";

  unique_lock<mutex> lk(fileMutex);
  sqlite3_stmt* iStmt;
  if( SQLITE_OK != sqlite3_prepare_v2(db,sqls.str().c_str(),-1,&iStmt,0) ) {
    cerr << "could not insert file [" <<sqls.str().c_str() << "]" << endl;
    return FileProcessorErrors::SQL_INSERT;
  }

  sqlite3_step(iStmt);
  sqlite3_finalize(iStmt);

  uint32_t pk = sqlite3_last_insert_rowid(db);
  lk.unlock();

  char word[MAX_WORD_SZ];
  word[0] = '\0';
  int j = 0;
  int line = 1;
  forward_list<Token> toks;

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
        Token t;
        t.word = string(word);
        t.line = line;
        toks.push_front(t);
      }
    }
  }
#undef c
  munmap(file,sb.st_size);

  lock_guard<mutex> loopLk(inMutex);
  while( !toks.empty() ) {
    Token t = toks.front();
    toks.pop_front();

    auto f = ids.find(t.word);

    uint32_t fk_id;
    if( f == ids.end() ) { // not found
      ids.emplace(t.word,++id_key);
      fk_id = id_key;
    } else { // found
      fk_id = f->second;
    }

    Location l;
    l.fk_id = fk_id;
    l.fk_file = pk;
    l.line = t.line;
    locs.push_front(l);
  }
  return FileProcessorErrors::SUCCESS;
}

FileProcessorErrors
FileProcessor::storeIdentifiers
(
  sqlite3                         *database,
  unordered_map<string,uint32_t>  &ids
)
{
  const char *sql = "DROP TABLE Identifiers;";
  if( SQLITE_OK != sqlite3_exec(database,sql,0,0,0) ) {
    cerr << "drop tables" << endl;
  }

  sql = "CREATE TABLE Identifiers("
        "pk       INTEGER PRIMARY KEY,"
        "name     TEXT    UNIQUE  NOT NULL);";

  if( SQLITE_OK != sqlite3_exec(database,sql,0,0,0) ) {
    return FileProcessorErrors::SQL_CREATE;
  }

  sqlite3_stmt  *idStmt;
  if( SQLITE_OK != sqlite3_prepare_v2(database,"INSERT INTO Identifiers (pk,name)"
                                         "VALUES (?,?);",256,&idStmt,0) ) {
    return FileProcessorErrors::SQL_PREPARE;
  }

  sqlite3_exec(database,"BEGIN TRANSACTION",0,0,0);
  for( auto id: ids ) {
    sqlite3_bind_int(idStmt,1,id.second);
    sqlite3_bind_text(idStmt,2,id.first.c_str(),-1,SQLITE_STATIC);
    sqlite3_step(idStmt);
    sqlite3_clear_bindings(idStmt);
    sqlite3_reset(idStmt);
  }
  sqlite3_exec(database,"END TRANSACTION",0,0,0);

  return FileProcessorErrors::SUCCESS;
}

FileProcessorErrors
FileProcessor::storeLocations
(
  sqlite3                         *database,
  forward_list<Location>                  &locs
)
{

  const char *sql = "DROP TABLE Locations;";
  if( SQLITE_OK != sqlite3_exec(database,sql,0,0,0) ) {
    cerr << "drop tables" << endl;
  }

  sql = "CREATE TABLE Locations("
        "pk       INTEGER PRIMARY KEY,"
        "fk_id    INT             NOT NULL,"
        "fk_file  INT             NOT NULL,"
        "line     INT             NOT NULL);";

  if( SQLITE_OK != sqlite3_exec(database,sql,0,0,0) ) {
    return FileProcessorErrors::SQL_CREATE;
  }

  sqlite3_stmt  *locStmt;
  if( SQLITE_OK != sqlite3_prepare_v2(database,"INSERT INTO Locations (pk,fk_id,fk_file,line)"
                                               "VALUES (?,?,?,?);",256,&locStmt,0) ) {
    return FileProcessorErrors::SQL_PREPARE;
  }

  sqlite3_exec(database,"BEGIN TRANSACTION",0,0,0);
  for( Location l: locs ) {
    sqlite3_bind_int(locStmt,2,l.fk_id);
    sqlite3_bind_int(locStmt,3,l.fk_file);
    sqlite3_bind_int(locStmt,4,l.line);
    sqlite3_step(locStmt);
    sqlite3_clear_bindings(locStmt);
    sqlite3_reset(locStmt);
  }
  sqlite3_exec(database,"END TRANSACTION",0,0,0);

  return FileProcessorErrors::SUCCESS;
}
