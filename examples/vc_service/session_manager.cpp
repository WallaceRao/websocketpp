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
  printf("AddSession succeed, id:%s,there is %d sessions now\n",
         session_id.c_str(), session_map.size());
  return true;
}

bool SessionManager::ClearSession(string session_id) {
  std::unique_lock<std::mutex> lck(map_mutex);
  auto iter = session_map.find(session_id);
  if (iter != session_map.end()) {
    session_map.erase(iter);
    printf("ClearSession succeed, id:%s, there is %d sessions now\n",
           session_id.c_str(), session_map.size());
    return true;
  }
  printf("ClearSession failed: session not exist, id:%s\n", session_id.c_str());
  return false;
}

bool SessionManager::AddThread(string session_id, std::thread &thread) {
  std::unique_lock<std::mutex> lck(send_mutex);
  if (send_threads.count(session_id)) {
    printf("AddThread failed: thread alreay exist, id:%s\n",
           session_id.c_str());
    return false;
  }
  send_threads[session_id] = std::move(thread);
  printf("AddThread succeed, id:%s,there is %d threads now\n",
         session_id.c_str(), send_threads.size());
  return true;
}

bool SessionManager::ClearThread(std::string session_id) {
  std::unique_lock<std::mutex> lck(send_mutex);
  auto iter = send_threads.find(session_id);
  if (iter != send_threads.end()) {
    send_threads.erase(iter);
    printf("ClearThread succeed, id:%s, there is %d threads now\n",
           session_id.c_str(), send_threads.size());
    return true;
  }
  printf("ClearThread failed: session not exist, id:%s\n", session_id.c_str());
  return false;
}
