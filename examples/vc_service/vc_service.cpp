#include <iostream>
#include <sstream>
#include <websocketpp/common/md5.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "json/json.h"
#include "session_manager.h"
#include "string_helper.h"

typedef websocketpp::server<websocketpp::config::asio> server;
typedef server::message_ptr message_ptr;

using namespace websocketpp::md5;
using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

// pull out the type of messages sent by our config
using namespace std;

string GetSessionId(void* p) {
  string result;
  stringstream stringtest;
  stringtest.clear();
  stringtest << p;
  string temp;
  stringtest >> temp;
  result = md5_hash_hex(temp);
  return result;
}

bool send_msg(server* s, server::connection_ptr con, string msg) {
  websocketpp::connection_hdl hdl = con->get_handle();
  string connection_id = GetSessionId(con.get());
  try {
    s->send(hdl, msg, websocketpp::frame::opcode::text);
  } catch (websocketpp::exception const& e) {
    std::cout << "send request (" << msg << ") for connection " << connection_id
              << "failed because : "
              << "(" << e.what() << ")" << std::endl;
    return false;
  }
  return true;
}

void send_thread(server* s, server::connection_ptr con) {
  websocketpp::connection_hdl hdl = con->get_handle();
  string connection_id = GetSessionId(con.get());
  shared_ptr<Session> session;
  string err_msg = "";
  shared_ptr<SessionManager> session_manager =
      SessionManager::GetSessionManager();
  if (!session_manager->GetSession(connection_id, session)) {
    err_msg = "Error: no existing session for " + connection_id;
    std::cout << err_msg << std::endl;
    Json::Value root;
    root["status"] = "failed";
    root["err_msg"] = err_msg;
    Json::FastWriter fast_writer;
    string response_json = fast_writer.write(root);
    try {
      s->send(hdl, response_json, websocketpp::frame::opcode::text);
    } catch (websocketpp::exception const& e) {
      std::cout << "send request for connection " << connection_id
                << "failed because : "
                << "(" << e.what() << ")" << std::endl;
    }
    return;
  }
  shared_ptr<SessionResponse> response;
  bool finish = false;
  while (!finish) {
    session->GetResponse(response, finish);
    // generate a response and send back
    Json::Value root;
    root["status"] = "succeed";
    root["err_msg"] = err_msg;

    string encoded_data;
    base64_encode((const uint8*)response->buffer, response->buffer_len,
                  encoded_data);
    root["data"] = encoded_data;
    Json::FastWriter fast_writer;
    string response_json = fast_writer.write(root);
    try {
      s->send(hdl, response_json, websocketpp::frame::opcode::text);
    } catch (websocketpp::exception const& e) {
      std::cout << "send request for connection " << connection_id
                << "failed because : "
                << "(" << e.what() << ")" << std::endl;
    }
  }
  std::cout << "all data has been sent with connection: " << connection_id
            << std::endl;
  // clear the session
  session_manager->ClearSession(connection_id);
  return;
}

// Define a callback to handle incoming messages
void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
  std::cout << "on_message called with hdl: " << hdl.lock().get()
            << " and message length: " << msg->get_payload().length()
            << std::endl;

  // check for a special command to instruct the server to stop listening so
  // it can be cleanly exited.
  server::connection_ptr con = s->get_con_from_hdl(hdl);
  string connection_id = GetSessionId(con.get());

  string request_json_str = msg->get_payload();
  shared_ptr<SessionRequest> request = make_shared<SessionRequest>();
  Json::Reader reader;
  Json::Value json_object;
  string err_msg = "";
  if (!reader.parse(request_json_str, json_object)) {
    err_msg = "Error: parse json string (" + request_json_str + ") failed";
    std::cout << err_msg << std::endl;
    send_msg(s, con, err_msg);
    return;
  }
  string data = json_object["data"].asString();
  string sample_rate_str = json_object["sample_rate"].asString();
  if (data.empty() || sample_rate_str.empty()) {
    err_msg = "Error: no data or sample rate field found in json string (" +
              request_json_str + ")";
    std::cout << err_msg << std::endl;
    send_msg(s, con, err_msg);
    return;
  }
  int sample_rate;
  StringToInt(sample_rate_str, sample_rate);

  // base64 parse
  request->buffer = new unsigned char[data.length() * 2];
  base64_decode((const uint8*)data.c_str(), data.length(), request->buffer,
                request->buffer_len);
  request->sample_rate = sample_rate;
  shared_ptr<SessionManager> session_manager =
      SessionManager::GetSessionManager();
  shared_ptr<Session> session;
  // construct a session request and add to session
  bool new_session = false;
  if (session_manager->GetSession(connection_id, session)) {
    new_session = false;
  } else {
    // create a new session and pass it to manager
    new_session = true;
    session = make_shared<Session>();
    session_manager->AddSession(connection_id, session);
  }

  session->AddToRequestQueue(request);
  if (new_session) {
    // start a thread to send back response
    session->StartProcess();
    std::thread(&send_thread, s, con);
  }
}

void on_close(server* s, websocketpp::connection_hdl hdl) {
  shared_ptr<SessionManager> session_manager =
      SessionManager::GetSessionManager();
  server::connection_ptr con = s->get_con_from_hdl(hdl);
  shared_ptr<Session> session;
  string connection_id = GetSessionId(con.get());
  if (!session_manager->GetSession(connection_id, session)) {
    std::cout << "Error on_close: no existing session for " << connection_id
              << std::endl;
    return;
  }
  session->StopProcess();
}

int main() {
  // Create a server endpoint
  server echo_server;
  try {
    // Set logging settings
    echo_server.set_access_channels(websocketpp::log::alevel::all);
    echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

    // Initialize Asio
    echo_server.init_asio();

    // Register our message handler
    echo_server.set_message_handler(
        bind(&on_message, &echo_server, ::_1, ::_2));
    echo_server.set_close_handler(bind(&on_close, &echo_server, ::_1));

    // Listen on port 9002
    echo_server.listen(websocketpp::lib::asio::ip::tcp::v4(), 9002);

    // Start the server accept loop
    echo_server.start_accept();

    // Start the ASIO io_service run loop
    echo_server.run();
  } catch (websocketpp::exception const& e) {
    std::cout << e.what() << std::endl;
  } catch (...) {
    std::cout << "other exception" << std::endl;
  }
}
