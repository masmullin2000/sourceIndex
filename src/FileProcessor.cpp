#include "FileProcessor.h"
#include "FileList.h"
#include "i_si.h"
#include "ThreadPool.hpp"
#include "FileParser.h"

#include <iostream>
#include <cstring>
#include <memory>
#include <sqlite3.h>

using namespace std;
using namespace concurrent;

FileProcessor::FileProcessor() noexcept
{
  sql = new SqliteAdapterInsert(FILE_DATABASE,IDENT_DATABASE,LOCS_DATABASE);
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
  bool                              idx
) noexcept
{
  ThreadPool tp(threads);

  FileList fl;
  fl.setFile( fList );

  unordered_map<string,uint32_t>  ids;
  forward_list<Location>          locs;
  uint32_t                        id_key = -1;

  sql->startBulk(SqliteAdapter::FBASE);
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
  sql->endBulk(SqliteAdapter::FBASE);

  tp.AddJob( [this,&ids]() noexcept {
      storeIdentifiers(ids);
  });
  if( idx ) {
    sql->indexLocations();
  }

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
  unique_lock<mutex> lk(fileMutex);
  uint32_t pk = sql->storeFile(fName);
  lk.unlock();

  auto parser = Make_Parser(fName);
  auto toks = parser->parse();

  lock_guard<mutex> loopLk(inMutex);

  while( toks != nullptr && !toks->empty() ) {
    Token t = toks->front();
    toks->pop_front();

    auto f = ids.find(t.word);

    uint32_t fk_id;
    if( f == ids.end() ) { // not found
      ids.emplace(t.word,++id_key);
      fk_id = id_key;
    } else { // found
      fk_id = f->second;
    }

    Location l(fk_id,pk,t.line);
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
