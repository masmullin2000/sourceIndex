#include <iostream>
#include <list>
#include <unordered_set>
#include <functional>

#define DATABASE "test.db"
#include <sqlite3.h>

#include "FileProcessor.h"
#include "FileList.h"
#include "ThreadPool.hpp"

using namespace std;

using namespace concurrent;

sqlite3 *db;

extern list<list<Identifier>> ids;

extern bool operator<( const Identifier& lhs, const Identifier& rhs );

bool operator==( const Identifier& lhs, const Identifier& rhs ) {
  bool rc = true;
  if( lhs.word.compare(rhs.word) != 0 ) rc = false;

  return rc;
}

class ID_HASH
{
public:
  size_t operator()( const Identifier& id ) const
  {
    return hash<string>()(id.word);
  }
};

sqlite3_stmt *idStmt;
sqlite3_stmt *locStmt;

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
  sqlite3_enable_shared_cache(1);
  if( sqlite3_open(DATABASE,&db) ) {
    return 1;
  }
  sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);

  char const *sql = "DROP TABLE Files;";
  if( SQLITE_OK != sqlite3_exec(db,sql,0,0,0) ) {
    cerr << "drop tables" << endl;
  }

  sql = "CREATE TABLE Files("
        "pk     INTEGER PRIMARY KEY,"
        "name   TEXT            NOT NULL);";

  if( SQLITE_OK != sqlite3_exec(db,sql,0,0,0) ) {
    cerr << "create teable" << endl;
    return 1;
  }

  sql = "DROP TABLE Identifiers;";
  if( SQLITE_OK != sqlite3_exec(db,sql,0,0,0) ) {
    cerr << "drop tables" << endl;
  }

  sql = "CREATE TABLE Identifiers("
        "pk       INTEGER PRIMARY KEY,"
        "name     TEXT    UNIQUE  NOT NULL);";

  if( SQLITE_OK != sqlite3_exec(db,sql,0,0,0) ) {
    cerr << "create table" << endl;
    return 1;
  }

  sql = "DROP TABLE Locations;";
  if( SQLITE_OK != sqlite3_exec(db,sql,0,0,0) ) {
    cerr << "drop tables" << endl;
  }

  sql = "CREATE TABLE Locations("
        "pk       INTEGER PRIMARY KEY,"
        "fk_id    INT             NOT NULL,"
        "fk_file  INT             NOT NULL,"
        "line     INT             NOT NULL);";

  if( SQLITE_OK != sqlite3_exec(db,sql,0,0,0) ) {
    cerr << "create table" << endl;
    return 1;
  }

  if( SQLITE_OK != sqlite3_prepare_v2(db,"INSERT INTO Identifiers (pk,name)"
                                         "VALUES (?,?);",256,&idStmt,0) ) {
    cerr << "prepare problem" << endl;
    return 1;
  }

  if( SQLITE_OK != sqlite3_prepare_v2(db,"INSERT INTO Locations (pk,fk_id,fk_file,line)"
                                         "VALUES (?,?,?,?);",256,&locStmt,0) ) {
    cerr << "prepare problem" << endl;
    return 1;
  }

  static FileList fl;
  fl.setFile( argv[1] );

  static ThreadPool tp(threadAmt);

  sqlite3_exec(db,"BEGIN TRANSACTION",0,0,0);

  ids.clear();
  string file = fl.getNextFile();
  int count = 1;
  while( file.length() > 0 ) {
    tp.AddJob(
      [file]() {
        FileProcessor fp;
        fp.setFile( file );
        fp.run();
      }
    );
    file = fl.getNextFile();
    count++;
  }
  cout << "files processed is " << count << endl;
  
  tp.JoinAll();
  sqlite3_exec(db,"END TRANSACTION",0,0,0);

  sqlite3_exec(db,"BEGIN TRANSACTION",0,0,0);

  string currStr;
  int i = -1;

  auto it = ids.begin();
  list<Identifier> mainlst = *it;

  for( ; it != ids.end(); ++it ) {
    mainlst.splice(mainlst.begin(),*it);
  }

  unordered_set<Identifier,ID_HASH> used;
  used.clear();

  while( !mainlst.empty() ) {
    Identifier cur = mainlst.front();

    auto fId = used.find(cur);
    if( fId == used.end() ) {
      sqlite3_bind_int(idStmt,1,++i);
      sqlite3_bind_text(idStmt,2,cur.word.c_str(),-1,SQLITE_STATIC);
      sqlite3_step(idStmt);
      sqlite3_clear_bindings(idStmt);
      sqlite3_reset(idStmt);

      sqlite3_bind_int(locStmt,2,i);
      sqlite3_bind_int(locStmt,3,cur.file_key);

      cur.file_key = i;
      used.insert(std::move(cur));
    } else {
      sqlite3_bind_int(locStmt,2,fId->file_key);
      sqlite3_bind_int(locStmt,3,cur.file_key);
    }

    sqlite3_bind_int(locStmt,4,cur.line_num);
    sqlite3_step(locStmt);
    sqlite3_clear_bindings(locStmt);
    sqlite3_reset(locStmt);
    mainlst.pop_front();

  }
  sqlite3_exec(db,"END TRANSACTION",0,0,0);

  return 0;
}