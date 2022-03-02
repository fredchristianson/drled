

#ifndef DRHTTP_CLIENT_H
#define DRHTTP_CLIENT_H


#include <WiFiClientSecure.h>
#include <lwip/dns.h>

#include <functional>
#include <memory>
#include <functional>
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>
#include "../log/logger.h"
#include "./wifi.h"


namespace DevRelief {

void dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    DRLogger log("DNSCallback",DEBUG_LEVEL);
    log.debug("DNSCallback %s %x",name,ipaddr);
}

class HttpUrl {
    public:
        HttpUrl() {

        }
    
        int port;
        DRString protocol;
        DRString host;
        DRString path;

        bool isSecure() const { 
            DRLogger log("isSecure",DEBUG_LEVEL);
            log.debug("check isSecure %x",this);
            log.debug("protocol %s",protocol.text());
            return Util::equal(protocol.text(),"https");
        }

        static HttpUrl* parse(const char* url) {
            DRLogger log("HttpUrl",DEBUG_LEVEL);
            HttpUrl* urlParts = NULL;
            if (url == 0 || url[0] != 'h') { 
                log.debug("invalid url %s",url ? url : "empty");
                return NULL;
            }
            const char * pos = url;
            if (Util::startsWith(url,"https://")) {
                urlParts = new HttpUrl();
                urlParts->protocol = "https";
                urlParts->port = 443;
                pos += 8;
                log.debug(LM("HTTPS url %s"),pos);

            } else if (Util::startsWith(url,"http://")){
                urlParts =  new HttpUrl();
                urlParts->protocol = "http";
                urlParts->port = 80;
                pos += 7;
                log.debug(LM("HTTP url %s"),pos);
            } else {
                log.debug(LM("invalid url %s"),url);
                return NULL;
            }
            log.debug("find port and path %s",pos);
            const char * colon = strchr(pos,':');
            const char * slash = strchr(pos,'/');
            int hostLen = strlen(pos);
            log.debug("found %d %d %d",colon,slash,hostLen);
            if (slash != NULL) {
                log.debug("got path %s",slash);
                urlParts->path = slash;
                hostLen = slash-pos;
                log.debug("hostLen %d",hostLen);

            }
            if (colon != NULL && colon < slash && isdigit(colon[1])){
                log.debug("parse port %s",colon+1);
                urlParts->port = atoi(colon+1);
                hostLen = colon-pos;
                log.debug("hostLen %d",hostLen);
            } else if (slash == NULL) {
                urlParts->host = pos;
            }
            log.debug("set host %d %s",hostLen,pos);
            urlParts->host = DRString(pos,hostLen);
            log.debug("return urlParts %s",urlParts->host.text());

            return urlParts;
        };



}; 

typedef enum HTTPState {
    HTTP_INACTIVE,
    HTTP_CONNECTING,
    HTTP_READING_HEADER,
    HTTP_READING_BODY,
    HTTP_COMPLETE,
    HTTP_ERROR
};

class HttpClient {
    public:

        HttpClient() {
            SET_LOGGER(WifiLogger);
            m_httpUrl = NULL;
            m_client = NULL;
            m_state = HTTP_INACTIVE;
        }

        virtual ~HttpClient() { 
            delete m_httpUrl;
            delete m_client;
        }

        void setupClient(WiFiClient* client, const char * method = "GET", const char * acceptType = "application/json") {
            m_logger->debug("setupClient");
            m_client = client;
            m_method = method;
            m_acceptType = acceptType;
            m_contentLength = -1;
            m_logger->debug("setupClient done");
        }


        bool getJson(const char * url) {
            if (m_state != HTTP_INACTIVE) {
                m_logger->error(LM("getJson called but state is not HTTP_INACTIVE"));
                return false; // request already sent on WiFiClient;
            }
            HttpUrl * httpUrl = HttpUrl::parse(url);
            m_logger->debug("got HttpUrl %x",httpUrl);
            m_logger->debug("host %s",httpUrl->host.text());
            m_logger->debug("protocol %s",httpUrl->protocol.text());
            if (httpUrl == NULL) {
                m_logger->error(LM("parse url failed"));
                
                return false;
            }
            m_httpUrl = httpUrl;
            if (m_httpUrl->isSecure()){
                m_logger->debug("create secure client");
                setupClient(new WiFiClientSecure());
            } else {
                m_logger->debug("create http client");
                setupClient(new WiFiClient());
            }
            if (m_httpUrl) {
                m_logger->debug("send request");
                sendRequest();
            }
            return m_state == HTTP_CONNECTING;
        }

        bool isComplete() {
            if (m_state == HTTP_COMPLETE || m_state == HTTP_ERROR){
                return true;
            }
            if (m_state == HTTP_CONNECTING) {
                m_logger->debug("connecting...");
                if (m_client->connected()) {
                    m_logger->debug("send request");
                    m_client->printf("%s %s HTTP/1.1\r\n",m_method,m_httpUrl->path.text());
                    m_client->printf("Accept: %s",m_acceptType);
                    m_client->print("\r\n");
                    m_client->flush();
                    m_state = HTTP_READING_HEADER;
                    m_buffer.clear();
                    m_buffer.reserve(500);
                }
                return false;
            }
            if (!m_client->connected()) {
                m_state = HTTP_ERROR;
                return true;
            }
            
            if (m_state == HTTP_READING_HEADER) {
                m_logger->debug("reading header...");
                if (m_client->available()) {
                    int len = m_buffer.getMaxLength();
                    uint8_t * buf = m_buffer.reserve(len);
                    int rlen = m_client->readBytesUntil('\n',buf,len);
                    buf[rlen] = 0;
                    m_logger->debug("got: %.50s",buf);
                    if (buf[0] == '\r') {
                        m_logger->debug("end of headers");
                        m_buffer.setLength(0);
                        buf[0] = 0;
                        m_state = HTTP_READING_BODY;
                    } else {
                        const char * line = (const char *) buf;
                        if (Util::startsWith(line,"Content-Length:")){
                            m_contentLength = atoi(line+16);
                            m_logger->debug("got length header: %d",m_contentLength);
                        }
                    }
                }
                return false;
            }
            if (m_state == HTTP_READING_BODY) {
                int len = m_buffer.getLength();
                uint8_t * buf = m_buffer.reserve(len+100);
                int rlen = m_client->readBytes(buf+len,100);
                m_buffer.setLength(len+rlen);
                buf[len+rlen] = 0;
                m_logger->debug("read: %s",buf+len);
                if (m_buffer.getLength() >= m_contentLength) {
                    m_state = HTTP_COMPLETE;
                }
                    
            }
            return false;
        }

    private:
        void checkDNS() {
            const char * host = m_httpUrl->host.text();
            ip_addr_t addr;
            long aResult;
            m_logger->debug("call dns_gethostbyname");
            delay(1000);
            IPAddress wifidnsaddr = WiFi.dnsIP(0);
            ip_addr_t dnsaddr;
            IP4_ADDR(&dnsaddr,8,8,8,8);
            dns_setserver(0,&dnsaddr);
            err_t err = dns_gethostbyname(host, &addr, &dns_callback, &aResult);
            m_logger->debug("\tdns_gethostbyname done %x",(int)err);
        }
        bool sendRequest() {
            const char * host = m_httpUrl->host.text();
            m_logger->debug("Send HTTP request to %s:%d",host,m_httpUrl->port);
            //checkDNS();
            int result = m_client->connect(host,m_httpUrl->port);
            if (result) {
                m_logger->debug("connect succeeded.");
            } else {
                m_logger->debug("connect returned %d (0x%x)",result,result);
            } 
            m_logger->debug("connect called()");
            m_state = HTTP_CONNECTING;
            return true;
        }

        DECLARE_LOGGER(); 
        DRBuffer m_buffer;
        WiFiClient * m_client;
        HttpUrl* m_httpUrl;
        const char * m_method;
        const char * m_acceptType;
        HTTPState m_state;
        long m_contentLength;
};


}


#endif 