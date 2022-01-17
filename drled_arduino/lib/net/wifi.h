#ifndef DRWIFIF_H
#define DRWIFIF_H
#include <ESP8266WiFi.h>
#include "../log/logger.h"
#include "../util/drstring.h"

namespace DevRelief {
    const char* ssid = "c22-2.4"; //replace this with your WiFi network name
    const char* password = "Dolphin#22"; //replace this with your WiFi network password
    
    class DRWiFi {
    public:
        static DRWiFi* get() {
            if (DRWiFi::drWiFiSingleton == NULL) {
                DRWiFi::drWiFiSingleton = new DRWiFi();
                DRWiFi::drWiFiSingleton->initialize();
            }
            return DRWiFi::drWiFiSingleton;
        }
    protected:
        DRWiFi(const char * hostname="dr_unnamed") {
            m_logger = new Logger("wifi",WARN_LEVEL);
            m_logger->debug("wifi created");
            m_hostname = hostname;
        }

        void initialize() {
            m_logger->info("WiFi initializing");
            WiFi.begin(ssid, password);
            
            while(WiFi.status() != WL_CONNECTED) {
                m_logger->info("waiting for wifi connection");
                delay(500);
            }
            WiFi.hostname(m_hostname.text());
            m_ipAddress = WiFi.localIP().toString().c_str();
            
            m_logger->info("WiFi connected %s",WiFi.localIP().toString().c_str());
        }

        bool isConnected() {
            return WiFi.status() == WL_CONNECTED;
        }

        const char * getIPAddress() { return m_ipAddress.text();}

    private:
        static DRString m_hostname;
        static DRString m_ipAddress;
        static Logger * m_logger;    
        static DRWiFi*  drWiFiSingleton;
    };

    DRWiFi * DRWiFi::drWiFiSingleton;
    Logger*  DRWiFi::m_logger;
    DRString DRWiFi::m_hostname;
    DRString DRWiFi::m_ipAddress;
}


#endif 