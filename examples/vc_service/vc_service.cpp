#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "json/json.h"
#include "session_manager.h"
#include "string_helper.h"

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;

bool send_msg(server* s, server::connection_ptr con, string msg) {
  websocketpp::connection_hdl hdl = con->get_handle();
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
  string connection_id = con.get();
  shared_ptr<Session> session;
  string err_msg = "";
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
    string encoded_data = Base64Encode(response->buffer, response->buffer_len);
    root["data"] = "";
    Json::FastWriter fast_writer;
    string response_json = fast_writer.write(root);
    try {
      s->send(hdl, response_json, frame::opcode::text);
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
            << " and message: " << msg->get_payload() << std::endl;

  // check for a special command to instruct the server to stop listening so
  // it can be cleanly exited.
  server::connection_ptr con = m_server.get_con_from_hdl(hdl);
  string connection_id = con.get();

  shared_ptr<SessionManager> session_manager =
      SessionManager::GetSessionManager();
  shared_ptr<Session> session;
  bool new_session = false;
  if (session_manager->GetSession(connection_id, session)) {
    new_session = false;
  } else {
    // create a new session and pass it to manager
    new_session = true;
    session = make_shared<Session>();
    session_manager->AddSession(connection_id, session);
  }

  // construct a session request and add to session
  string request_json_str = msg->get_payload();
  shared_ptr<SessionRequest> request = make_shared<SessionRequest>();
  Json::Reader reader;
  Json::Value json_object;
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
  string sample_rate_str = json_object["sample_rate"].asString();
  int sample_rate;
  StringToInt(sample_rate_str, sample_rate);

  // base64 parse
  string token = user_name + std::to_string((int)t);
  string md5_token, hex_token;
  int out_byte = 0;
  string buf = Base64Decode(data.data(), data.length(), out_byte);
  request->buffer_len = buf.length();
  request->buffer = new unsigned char[request->buffer_len];
  request->sample_rate = sample_rate;
  memcpy(request->buffer, buf.data(), request->buffer_len);
  session.AddToRequestQueue(request);
  if (new_session) {
    // start a thread to send back response
    session->StartProcess();
    std::thread(&send_thread, s, con);
  }
}

void on_close(connection_hdl hdl) {
  server::connection_ptr con = m_server.get_con_from_hdl(hdl);
  string connection_id = con.get();
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
    m_server.set_close_handler(bind(&on_close, &echo_server, ::_1);

    // Listen on port 9002
    echo_server.listen(9002);

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
