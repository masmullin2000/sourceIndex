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
  sqlite3_exec(db, "PRAGMA journal_mode = OFF", NULL, NULL, NULL);
  

  char const *sql = "DROP TABLE Files;";
  if( SQLITE_OK != sqlite3_exec(db,sql,0,0,0) ) {
    cerr << "drop tables" << endl;
  }

  sql = "CREATE TABLE Files("
        "pk     INTEGER PRIMARY KEY,"
        "name   TEXT            NOT NULL);";

  if( SQLITE_OK != sqlite3_exec(db,sql,0,0,0) ) {
    cerr << "create table" << endl;
    return 1;
  }

  sql = "DROP TABLE Identifiers;";
  if( SQLITE_OK != sqlite3_exec(db,sql,0,0,0) ) {
    cerr << "drop tables" << endl;
  }

#ifdef SIZE
  sql = "CREATE TABLE Identifiers("
        "pk       INTEGER PRIMARY KEY,"
        "name     TEXT    UNIQUE  NOT NULL);";

  if( SQLITE_OK != sqlite3_exec(db,sql,0,0,0) ) {
    cerr << "create table" << endl;
    return 1;
  }
  
  if( SQLITE_OK != sqlite3_prepare_v2(db,"INSERT INTO Identifiers (pk,name)"
                                         "VALUES (?,?);",256,&idStmt,0) ) {
    cerr << "prepare problem" << endl;
    return 1;
  }
#endif

  sql = "DROP TABLE Locations;";
  if( SQLITE_OK != sqlite3_exec(db,sql,0,0,0) ) {
    cerr << "drop tables" << endl;
  }

  sql = "CREATE TABLE Locations("
        "pk       INTEGER PRIMARY KEY,"
#ifdef SIZE
        "fk_id    INT             NOT NULL,"
#else
        "name     TEXT            NOT NULL,"
#endif
        "fk_file  INT             NOT NULL,"
        "line     INT             NOT NULL);";

  if( SQLITE_OK != sqlite3_exec(db,sql,0,0,0) ) {
    cerr << "create table" << endl;
    return 1;
  }


#ifdef SIZE
  if( SQLITE_OK != sqlite3_prepare_v2(db,"INSERT INTO Locations (pk,fk_id,fk_file,line)"
                                         "VALUES (?,?,?,?);",256,&locStmt,0) ) {
    cerr << "prepare problem loc size" << endl;
    return 1;
  }
#else
  if( SQLITE_OK != sqlite3_prepare_v2(db,"INSERT INTO Locations (pk,name,fk_file,line)"
                                         "VALUES (?,?,?,?);",256,&locStmt,0) ) {
    cerr << "prepare problem locations" << endl;
    return 1;
  }
#endif

  static FileList fl;
  fl.setFile( argv[1] );

  static ThreadPool tp(threadAmt);

  sqlite3_exec(db,"BEGIN TRANSACTION",0,0,0);

  ids.clear();
  string file = fl.getNextFile();
  if( file.length() == 0 ) {
    cerr << "File: " << argv[1] << " does not exist" << endl;
    return 1;
  }
  cout << "Start Reading Files" << endl;
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
  
  tp.WaitAll();
  cout << "files processed is " << count << endl;

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
  
  struct locs {
  uint32_t id;
  uint32_t file;
  uint16_t line;
  locs(int i, int f, int l) {
    id = i;
    file = f;
    line = l;
  }
  bool operator<( const locs& rhs ) const {
    if( id < rhs.id ) return true;
    if( file < rhs.file ) return true;
    if( line < rhs.line ) return true;
    else return false;
  }
  };
  
  list<locs> loc_list;
  
  while( !mainlst.empty() ) {
    Identifier cur = mainlst.front();
    mainlst.pop_front();


    auto fId = used.find(cur);
    
    int id_pk;
    const char* id_name;
    
    int loc_id;
    int loc_file = cur.file_key;
    int loc_line = cur.line_num;
    
    if( fId == used.end() ) {
      id_pk = ++i;
      id_name = cur.word.c_str();
      loc_id = i;

#ifdef SIZE
      sqlite3_bind_int(idStmt,1,id_pk);
      sqlite3_bind_text(idStmt,2,id_name,-1,SQLITE_STATIC);
      sqlite3_step(idStmt);
      sqlite3_clear_bindings(idStmt);
      sqlite3_reset(idStmt);
#endif
      cur.file_key = i;
      used.insert(std::move(cur));
    } else {
      loc_id = fId->file_key;
    }
    
#ifdef SIZE
    //sqlite3_bind_int(locStmt,2,loc_id);
    locs lkj(loc_id,loc_file,loc_line);
    loc_list.push_back(lkj);
#else
    sqlite3_bind_text(locStmt,2,id_name,-1,SQLITE_STATIC);

    sqlite3_bind_int(locStmt,3,loc_file);
    sqlite3_bind_int(locStmt,4,loc_line);
    sqlite3_step(locStmt);
    sqlite3_clear_bindings(locStmt);
    sqlite3_reset(locStmt);
#endif
  }
  cout << "finished with ids" << endl;
  used.clear();
#ifdef SIZE
  for( locs l: loc_list ) {
    sqlite3_bind_int(locStmt,2,l.id);
    sqlite3_bind_int(locStmt,3,l.file);
    sqlite3_bind_int(locStmt,4,l.line);
    sqlite3_step(locStmt);
    sqlite3_clear_bindings(locStmt);
    sqlite3_reset(locStmt);
  }
#endif
  tp.JoinAll();
  sqlite3_exec(db,"END TRANSACTION",0,0,0);

  return 0;
}