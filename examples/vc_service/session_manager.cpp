#include "session_manager.h"

shared_ptr<SessionManager> session_manager;

shared_ptr<SessionManager> SessionManager::GetSessionManager() {
  if (!session_manager) {
    session_manager = make_shared<SessionManager>();
  }
  return session_manager;
}

bool SessionManager::GetSession(string session_id,
                                shared_ptr<Session> &session) {
  std::unique_lock<std::mutex> lck(map_mutex);
  if (session_map.count(session_id)) {
    session = session_map[session_id];
    return true;
  }
  printf("GetSession failed: no session exist with id:%s\n",
         session_id.c_str());
  return false;
}

bool SessionManager::AddSession(string session_id,
                                shared_ptr<Session> session) {
  std::unique_lock<std::mutex> lck(map_mutex);
  if (session_map.count(session_id)) {
    printf("AddSession failed: session alreay exist, id:%s\n",
           session_id.c_str());
    return false;
  }
  session_map[session_id] = session;
  printf("AddSession succeed, id:%s,s %d sessions now\n", session_id.c_str(),
         session_map.size());
  return true;
}

bool SessionManager::ClearSession(string session_id) {
  std::unique_lock<std::mutex> lck(map_mutex);
  auto iter = session_map.find(session_id);
  if (iter != session_map.end()) {
    session_map.erase(iter);
    printf("ClearSession succeed, id:%s,s %d sessions now\n",
           session_id.c_str(), session_map.size());
    return true;
  }
  printf("ClearSession failed: session not exist, id:%s\n", session_id.c_str());
  return false;
}
