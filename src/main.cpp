#include <iostream>
#include <functional>

#define FILE_DATABASE "files.db"
#define IDENT_DATABASE "idents.db"
#define LOCS_DATABASE "locs.db"
#include <sqlite3.h>

#include "FileProcessor.h"
#include "FileList.h"
#include "ThreadPool.hpp"

using namespace std;
using namespace concurrent;

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
  sqlite3 *db_files;
  sqlite3 *db_idents;
  sqlite3 *db_locs;

  sqlite3_enable_shared_cache(1);
  if( sqlite3_open(FILE_DATABASE,&db_files) ) {
    return 1;
  }
  if( sqlite3_open(IDENT_DATABASE,&db_idents) ) {
    return 1;
  }
  if( sqlite3_open(LOCS_DATABASE,&db_locs) ) {
    return 1;
  }
  sqlite3_exec(db_files, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
  sqlite3_exec(db_files, "PRAGMA journal_mode = MEMORY", NULL, NULL, NULL);

  sqlite3_exec(db_idents, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
  sqlite3_exec(db_idents, "PRAGMA journal_mode = MEMORY", NULL, NULL, NULL);


  sqlite3_exec(db_locs, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
  sqlite3_exec(db_locs, "PRAGMA journal_mode = MEMORY", NULL, NULL, NULL);


  char const *sql = "DROP TABLE Files;";
  if( SQLITE_OK != sqlite3_exec(db_files,sql,0,0,0) ) {
    cerr << "drop tables" << endl;
  }

  sql = "CREATE TABLE Files("
        "pk     INTEGER PRIMARY KEY,"
        "name   TEXT            NOT NULL);";

  if( SQLITE_OK != sqlite3_exec(db_files,sql,0,0,0) ) {
    cerr << "create table" << endl;
    return 1;
  }

  static FileList fl;
  fl.setFile( argv[1] );

  ThreadPool tp(threadAmt);

  sqlite3_exec(db_files,"BEGIN TRANSACTION",0,0,0);

  string file = fl.getNextFile();
  if( file.length() == 0 ) {
    cerr << "File: " << argv[1] << " does not exist" << endl;
    return 1;
  }
  cout << "Start Reading Files" << endl;
  unordered_map<string,uint32_t>  ids;
  forward_list<Location>          locs;

  uint32_t id_key = 0;
  while( file.length() > 0 ) {
    tp.AddJob(
      [db_files,file,&ids,&locs,&id_key]() {
        FileProcessor fp;
        fp.run(db_files,file,ids,locs,id_key);
      }
    );
    file = fl.getNextFile();
  }
  tp.WaitAll();

  sqlite3_exec(db_files,"END TRANSACTION",0,0,0);

  tp.AddJob( [db_idents,&ids]() {
    FileProcessor::storeIdentifiers(db_idents,ids);
  });
  tp.AddJob( [db_locs,&locs]() {
    FileProcessor::storeLocations(db_locs,locs);
  });
  tp.JoinAll();

  ids.clear();
  locs.clear();

  return 0;
}