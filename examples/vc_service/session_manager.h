#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <stdio.h>
#include <map>
#include <memory>
#include <mutex>
#include "session.h"

using namespace std;

class SessionManager {
 public:
  std::mutex map_mutex;
  map<string, shared_ptr<Session>> session_map;
  bool GetSession(string session_id, shared_ptr<Session> &session);
  bool AddSession(string session_id, shared_ptr<Session> session);
  bool ClearSession(string session_id);

  static shared_ptr<SessionManager> GetSessionManager();
};
#endif  // SESSION_MANAGER_H