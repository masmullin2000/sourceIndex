#include "FileProcessor.h"
#include "FileList.h"
#include "utils.h"
#include "i_si.h"
#include "ThreadPool.hpp"

#include <iostream>
#include <cstring>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sqlite3.h>

using namespace std;
using namespace concurrent;

#define MAX_WORD_SZ 128

FileProcessor::FileProcessor() noexcept
{
  sql = new SqliteAdapterInsert(FILE_DATABASE,IDENT_DATABASE,LOCS_DATABASE);
  massive_memory = false;
}

FileProcessor::~FileProcessor() noexcept
{
  delete sql;
}

FileProcessorErrors
FileProcessor::run
(
  string                           &fList,
  uint8_t                           threads,
  bool                              mm,
  bool                              idx
) noexcept
{
  massive_memory = mm;
  ThreadPool tp(threads);

  FileList fl;
  fl.setFile( fList );

  unordered_map<string,uint32_t>  ids;
  forward_list<Location>          locs;
  uint32_t                        id_key = -1;

  sql->startBulk(SqliteAdapter::FBASE);
  sql->startBulk(SqliteAdapter::IBASE);
  sql->startBulk(SqliteAdapter::LBASE);
  string file = fl.getNextFile();
  while( file.length() > 0 ) {
    tp.AddJob(
      [this,file,&ids,&locs,&id_key]() noexcept {
        processFile(file,ids,locs,id_key);
      }
    );
    file = fl.getNextFile();
  }
  tp.WaitAll();
  sql->endBulk(SqliteAdapter::LBASE);
  sql->endBulk(SqliteAdapter::IBASE);
  sql->endBulk(SqliteAdapter::FBASE);

  tp.AddJob( [this,&ids]() noexcept {
      storeIdentifiers(ids);
    });
  tp.AddJob( [this,&locs,idx]() noexcept {
    if( massive_memory ) {
      storeLocations(locs);
      if( idx ) {
        sql->indexLocations();
      }
    } else if( idx ) {
      sql->indexLocations();
    }
  }); 

  tp.JoinAll();

  ids.clear();
  locs.clear();

  return FileProcessorErrors::SUCCESS;
}

FileProcessorErrors
FileProcessor::processFile
(
  string                            fName,
  unordered_map<string,uint32_t>   &ids,
  forward_list<Location>           &locs,
  uint32_t                         &id_key
) noexcept
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

  unique_lock<mutex> lk(fileMutex);
  uint32_t pk = sql->storeFile(fName);
  lk.unlock();

  char word[MAX_WORD_SZ];
  word[0] = '\0';
  int j = 0;
  int line = 1;
  forward_list<Token> toks;

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
        toks.emplace_front(t);
      }
      if( c == '\n' ) line++;
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

    if( massive_memory )
      locs.emplace_front(l);
    else
      sql->storeLocation(l);
  }
  return FileProcessorErrors::SUCCESS;
}

FileProcessorErrors
FileProcessor::storeIdentifiers
(
  unordered_map<string,uint32_t>  &ids
) noexcept
{
  sql->startBulk(SqliteAdapter::IBASE);
  for( auto &id: ids ) {
    sql->storeIdentifier(id.first,id.second);
  }
  sql->endBulk(SqliteAdapter::IBASE);

  return FileProcessorErrors::SUCCESS;
}

FileProcessorErrors
FileProcessor::storeLocations
(
  forward_list<Location>          &locs
) noexcept
{
  sql->startBulk(SqliteAdapter::LBASE);
  for( Location &l: locs ) {
    sql->storeLocation(l);
  }
  sql->endBulk(SqliteAdapter::LBASE);
  return FileProcessorErrors::SUCCESS;
}
