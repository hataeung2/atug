#ifndef _ATUG2_SQLITE3EE_H_
#define _ATUG2_SQLITE3EE_H_
#include "atug2.h"
#include "adata.h"

#define SQLITE_HAS_CODEC 1
#include "sqlite3.h"
#if defined(_WINDOWS)
  #pragma comment(lib, "sqlite3.lib")
#endif

namespace atug2 {
  // sqlite 3 wrapper 
  #define ASQLITE_CALLBACK [](void* param, int argc, char **argv, char **col_name) -> int 

  // class interface
  class ASqlite {
  public:
    ASqlite() {}
    ~ASqlite() { Close(); }
  public:
    typedef Number SqlRes;
    const SqlRes Open(string _db_filename, string _key) {
      auto res = Open(_db_filename);
      if (SQLITE_OK EQ res) {
        //bool res = sqlite3_key_v2(db_, _db_filename.c_str(), _key.c_str(), _key.length());
        string qry = "PRAGMA key='" + _key + "'";
        res = sqlite3_exec(db_, qry.c_str(), nullptr, nullptr, nullptr);
        if (SQLITE_OK NE res) {
          _TRACE("ENCRYPTED DB OPEN FAILED.");
        }
      } 
      return res;
    }
    const SqlRes Open(string _db_filename) {
      if (_db_filename.length() AND '.' EQ _db_filename.at(0)) {
        _TRACE("You have to use absolute path to open database.");
        return SQLITE_ERROR;
      }
      int rc{ sqlite3_open(_db_filename.c_str(), &db_) };
      if (SQLITE_OK != rc) {
        _TRACE("DB OPEN FAILED.");
        sqlite3_close(db_);
      }
      return rc;
    }
    const SqlRes OpenV2(string _db_filename, const Number _flags = SQLITE_OPEN_READONLY) {
      if (_db_filename.length() AND '.' EQ _db_filename.at(0)) {
        _TRACE("You have to use absolute path to open database.");
        return SQLITE_ERROR;
      }
      int rc{ sqlite3_open_v2(_db_filename.c_str(), &db_, _flags, nullptr) };
      if (SQLITE_OK != rc) {
        _TRACE("DB OPEN FAILED.");
        sqlite3_close(db_);
      }
      return rc;
    }
    const SqlRes Close() {
      int rc{ SQLITE_OK };
      if (db_) {
        rc = sqlite3_close(db_);
        db_ = nullptr;
      }
      return rc;
    }
    const SqlRes Query(const string& _query, sqlite3_callback _callback = nullptr, void* _param = nullptr) {
      char* err_msg{ nullptr };
      const int err{ sqlite3_exec(db_, _query.c_str(), _callback, _param, &err_msg) };
      if (SQLITE_OK NE err) {
        _TRACE("QUERY FAILED. ERRCODE: " << err);
        last_err_msg_ = err_msg;
        sqlite3_free(err_msg);
      }
      return err;
    }
    const SqlRes Query(const wstring& _query, sqlite3_callback _callback = nullptr, void* _param = nullptr) {
      string query{ adata::UnicodeToUtf8(_query) };
      return Query(query, _callback, _param);
    }
    Void BeginTransaction() {
      sqlite3_exec(db_, "BEGIN"/*BEGIN TRANSACTION*/, nullptr, nullptr, nullptr);
    }
    Void EndTransaction() {
      sqlite3_exec(db_, "COMMIT"/*END TRANSACTION*/, nullptr, nullptr, nullptr);
    }
    Void SetJournalMode(const string& _mode = "MEMORY"/*DELETE(default), WAL, ...*/) {
      string mode_str = "PRAGMA journal_mode=" + _mode;
      sqlite3_exec(db_, mode_str.c_str(), nullptr, nullptr, nullptr);
    }
    Void SetBusyTimeout(const unsigned int _ms) {
      timeout_ = (100 < _ms) ? _ms : 100;
      sqlite3_busy_timeout(db_, timeout_);
    }
  private:
    sqlite3* db_;
    string last_err_msg_;
    unsigned int timeout_ = 100;
  };
}//!namespace atug2

#endif//!_ATUG2_SQLITE3EE_H_
