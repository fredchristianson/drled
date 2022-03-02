
#ifndef DRHTTP_SERVER_H
#define DRHTTP_SERVER_H


#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <functional>
#include <memory>
#include <functional>
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>
#include "../log/logger.h"
#include "./wifi.h"


namespace DevRelief {
    typedef ESP8266WebServer Request;
    typedef ESP8266WebServer Response;

using HttpHandler = std::function<void(Request*, Response*)> ;

class HttpServer {
    public:


        HttpServer() {
            SET_LOGGER(HttpServerLogger);
            m_logger->debug(LM("HttpServer created"));

            m_server = new ESP8266WebServer(80);

            m_logger->info(LM("Listening to port 80 on IP %s"),WiFi.localIP().toString().c_str());

           
        }


        void begin() {
            m_logger->info(LM("HttpServer listening"));
            m_server->begin();
        }

        void handleClient() {
            if (m_server->client()) {
                m_server->client().keepAlive();
            }
            m_server->handleClient();
        }

        void routeNotFound(HttpHandler httpHandler){
            m_logger->debug(LM("routing Not Found handler"));
            auto server = m_server;
            auto handler = httpHandler;
            m_server->onNotFound([this,handler,server](){
                if (m_server->method() == HTTP_OPTIONS){
                    m_logger->debug(LM("send CORS for HTTP_OPTIONS"));
                    this->cors(m_server);
                    m_server->send(204);
                    return;
                }
                this->cors(server);
                handler(server,server);
            });
        }

        void routeBraces(const char * uri, HttpHandler httpHandler){
            m_logger->debug(LM("routing to Uri %s"),uri);
            auto server = m_server;
            auto handler = httpHandler;
            m_server->on(UriBraces(uri),[this,handler,server](){
                this->cors(server);
                m_logger->debug("returning uri found");
                handler(server,server);
            });
        }

        void routeBracesGet(const char * uri, HttpHandler httpHandler){
            m_logger->debug(LM("routing GET to Uri %s"),uri);
            auto server = m_server;
            auto handler = httpHandler;
            m_server->on(UriBraces(uri),HTTP_GET,[this,handler,server](){
                this->cors(server);
                //m_logger->debug("uri found");
                handler(server,server);
            });
        }

        void routeBracesPost(const char * uri, HttpHandler httpHandler){
            m_logger->debug(LM("routing POST to Uri %s"),uri);
            auto server = m_server;
            auto handler = httpHandler;
            m_server->on(UriBraces(uri),HTTP_POST,[this,handler,server](){
                this->cors(server);
                m_logger->debug(LM("uri found"));
                handler(server,server);
            });
        }

        void routeBracesDelete(const char * uri, HttpHandler httpHandler){
            m_logger->debug(LM("routing DELETE to Uri %s"),uri);
            auto server = m_server;
            auto handler = httpHandler;
            m_server->on(UriBraces(uri),HTTP_DELETE,[this,handler,server](){
                this->cors(server);
                m_logger->debug("uri found");
                handler(server,server);
            });
        }

        void route(const char * uri, HttpHandler httpHandler){
            m_logger->debug(LM("routing %s"),uri);
            auto server = m_server;
            auto handler = httpHandler;
            m_server->on(uri,[this,handler,server,uri](){
                this->cors(server);
                m_logger->debug(LM("path found %s"),uri);

                handler(server,server);
            });
        }
        void route(const char * uri, HTTPMethod method, HttpHandler httpHandler){
            m_logger->debug(LM("routing %s"),uri);
             auto server = m_server;
            auto handler = httpHandler;
            m_server->on(uri,method, [this,handler,server](){
                this->cors(server);
                handler(server,server);
            });
        }

        void send(const char * type, const char * value) {

            m_server->send(200,type,value);
        }

        void cors(ESP8266WebServer * server) {
            server->sendHeader("Access-Control-Allow-Origin","*");
            server->sendHeader("Access-Control-Allow-Headers","Content-Type");
        }

    private:
        DECLARE_LOGGER(); 
        ESP8266WebServer * m_server;
    };
}
#endif 