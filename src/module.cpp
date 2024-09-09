#include "module.h"

IPAddress ip;
IPAddress gateways;
IPAddress subnets;
IPAddress dns;

bool skip = false;

int LED_buildin = 13;

int MODE_AP = 0;
int MODE_STA = 1;
int MODE_APSTA = 2;

std::map<String, String> env_vars;

String MODES;
String AP_SSID;
String AP_PSK;
String STA_SSID;
String STA_PSK;
String mqtt_enable;
String mqtt_broker;
String mqtt_port;
String mqtt_topic;
String mqtt_username;
String mqtt_password;
String mqtt_interval;
String RTU_enable;
String RTU_deviceID;
String RTU_baudRate;
String TCP_wifi_enable;
String TCP_eth_enable;
String local_IP;
String gateway;
String subnet;
String dnss;

const char* ssid = "tanawut";
const char* password = "";

AsyncWebServer server(80);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_wifiAP(){
    if (!WiFi.softAPConfig(ip, gateways, subnets))
    {
        Serial.println("AP Config Failed");
    }

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    Serial.print("\nWiFi shared on SSID: ");
    Serial.print(ssid);
    Serial.print(" ,PSK: ");
    Serial.println(password);
    Serial.print("AP IP address: "); //Defaut192.168.4.1
    Serial.println(WiFi.softAPIP());
}


void Config_server(){
  if(!SPIFFS.begin(true)){
    Serial.print("Error Stratinf SPIFFS!!!");
    return;

  }

    /* ============================ อ่านไฟล์ .env ============================ */
    server.on("/env", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    File file = SPIFFS.open("/.env", "r");
    if (!file) {
        request->send(500, "text/plain", "Failed to open file");
        return;
    }

    String fileContent;
    while (file.available()) {
        fileContent += file.readStringUntil('\n') + "\n"; // Ensure newline characters are included
    }
    file.close();
    request->send(200, "text/plain", fileContent); });
    /* ==================================================================== */

  readEnvFile();
  configureNetwork(local_IP, gateway, subnet, dnss);
  setup_wifiAP();


if(!MDNS.begin("phonehumyai")){
  Serial.println("Error Starting DNS");
  return;
}


server.on("/styles.css",HTTP_GET, [](AsyncWebServerRequest *request)
          {request->send(SPIFFS, "/styles.css"); });

server.on("bscripts.js",HTTP_GET, [](AsyncWebServerRequest *request)
          {request->send(SPIFFS, "/bscripts.js"); });
        
server.on("/",HTTP_GET, [](AsyncWebServerRequest *request)
          {request->send(SPIFFS, "/index.html"); });

server.on("/buttons",HTTP_GET, [](AsyncWebServerRequest *request)
          {request->send(SPIFFS, "/buttons.html"); });

server.on("/info",HTTP_GET, [](AsyncWebServerRequest *request)
          {request->send(SPIFFS, "/info.html"); });

server.on("/Capture1img",HTTP_GET, [](AsyncWebServerRequest *request)
          {request->send(SPIFFS, "/Capture1.PNG"); });

server.on("/Capture2img",HTTP_GET, [](AsyncWebServerRequest *request)
          {request->send(SPIFFS, "/Capture2.PNG"); });

server.on("/122552",HTTP_GET, [](AsyncWebServerRequest *request)
          {request->send(SPIFFS, "/122552.jpg"); });

server.on("/122553",HTTP_GET, [](AsyncWebServerRequest *request)
          {request->send(SPIFFS, "/122553.jpg"); });

server.on("/networks",HTTP_GET, [](AsyncWebServerRequest *request)
          {request->send(SPIFFS, "/networks.html"); });

server.on("/saveConfig" , HTTP_POST, handleSaveConfig);

server.on("/scripts.js",HTTP_GET, [](AsyncWebServerRequest *request)
          {request->send(SPIFFS, "/scripts.js","application/javascript"); });
  
server.on("/networksConfig" , HTTP_POST, handleNetworksConfig);
  
  MDNS.addService("http","tcp",80);
  server.begin();        
}
void handleIndex(AsyncWebServerRequest *request){

}


void writeEnvFile()
{
    File file = SPIFFS.open("/.env", "w");
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    for (const auto &pair : env_vars)
    {
        file.println(pair.first + "=" + pair.second);
    }
    file.close();
}

void readEnvFile()
{
    Serial.println("Start reading ENV file");
    File file = SPIFFS.open("/.env", FILE_READ);
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }
    env_vars.clear();
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();

        int separatorIndex = line.indexOf('=');
        if (separatorIndex != -1)
        {
            String key = line.substring(0, separatorIndex);
            String value = line.substring(separatorIndex + 1);

            // Remove leading and trailing spaces from key and value
            key.trim();
            value.trim();

            // Store key-value pair in env_vars
            env_vars[key] = value;
        }
    }

    file.close();

    // Process specific variables
    MODES = env_vars["Mode"];
    AP_SSID = env_vars["AP_SSID"];
    AP_PSK = env_vars["AP_PSK"];
    STA_SSID = env_vars["STA_SSID"];
    STA_PSK = env_vars["STA_PSK"];

    Serial.print("STA Length: ");
    Serial.println(STA_SSID.length());

    if(STA_SSID.length() > 1){
        MODES = "1";
    }

    mqtt_enable = (env_vars["mqtt_enable"] == "true") ? "1" : "0";
    mqtt_broker = env_vars["mqtt_broker"];
    mqtt_port = env_vars["mqtt_port"];
    mqtt_topic = env_vars["mqtt_topic"];
    mqtt_username = env_vars["mqtt_username"];
    mqtt_password = env_vars["mqtt_password"];
    mqtt_interval = env_vars["mqtt_interval"];

    RTU_enable = (env_vars["RTU_enable"] == "true") ? "1" : "0";
    RTU_deviceID = env_vars["RTU_deviceID"];

    String baudRateStr = env_vars["RTU_baudRate"];
    if (baudRateStr == "0")
        RTU_baudRate = "2400";
    else if (baudRateStr == "1")
        RTU_baudRate = "4800";
    else if (baudRateStr == "2")
        RTU_baudRate = "9600";
    else if (baudRateStr == "3")
        RTU_baudRate = "115200";

    TCP_wifi_enable = (env_vars["TCP_wifi_enable"] == "true") ? "1" : "0";
    TCP_eth_enable = (env_vars["TCP_eth_enable"] == "true") ? "1" : "0";

    local_IP = env_vars["local_IP"];
    gateway = env_vars["gateway"];
    subnet = env_vars["subnet"];
    dnss = env_vars["dns"];


#ifdef DEBUG_ReadENV
    Serial.println("\n*====================================================================*\n*\n*");
    Serial.print("*\tEnergy: ");
    Serial.println(energy_kWh, 6);
    Serial.println("*\n*\n*====================================================================*");

    Serial.println("\n*====================================================================*\n*\n*");
    Serial.print("*\tMODES: ");
    Serial.println(MODES);
    Serial.print("*\tAP_SSID: ");
    Serial.println(AP_SSID);
    Serial.print("*\tAP_PSK: ");
    Serial.println(AP_PSK);
    Serial.print("*\tSTA_SSID: ");
    Serial.println(STA_SSID);
    Serial.print("*\tSTA_PSK: ");
    Serial.println(STA_PSK);
    Serial.println("*\n*\n*====================================================================*");

    Serial.println("\n*====================================================================*\n*\n*");
    Serial.print("*\tmqtt_enable: ");
    Serial.println(mqtt_enable);
    Serial.print("*\tmqtt_broker: ");
    Serial.println(mqtt_broker);
    Serial.print("*\tmqtt_port: ");
    Serial.println(mqtt_port);
    Serial.print("*\tmqtt_topic: ");
    Serial.println(mqtt_topic);
    Serial.print("*\tmqtt_username: ");
    Serial.println(mqtt_username);
    Serial.print("*\tmqtt_password: ");
    Serial.println(mqtt_password);
    Serial.print("*\tmqtt_interval: ");
    Serial.println(mqtt_interval);
    Serial.println("*\n*\n*====================================================================*\n\n\n");

    Serial.println("\n*====================================================================*\n*\n*");
    Serial.print("*\tRTU_enable: ");
    Serial.println(RTU_enable);
    Serial.print("*\tRTU_deviceID: ");
    Serial.println(RTU_deviceID);
    Serial.print("*\tRTU_baudRate: ");
    Serial.println(RTU_baudRate);
    Serial.print("*\tTCP_wifi_enable: ");
    Serial.println(TCP_wifi_enable);
    Serial.print("*\tTCP_eth_enable: ");
    Serial.println(TCP_eth_enable);
    Serial.println("*\n*\n*====================================================================*\n\n\n");

    Serial.println("\n*====================================================================*\n*\n*");
    Serial.print("*\tIPAddress ");
    Serial.println(local_IP);
    Serial.print("*\tGateway: ");
    Serial.println(gateway);
    Serial.print("*\tSubnet: ");
    Serial.println(subnet);
    Serial.print("*\tDNS: ");
    Serial.println(dnss);
    Serial.println("*\n*\n*====================================================================*");
#endif
}

void updateEnvVariable(const String &key, const String &value)
{
    env_vars[key] = value;
    writeEnvFile();
    readEnvFile();
}

IPAddress stringToIPAddress(String ipStr)
{
    int octets[4];
    int octetIndex = 0;
    int fromIndex = 0;

    for (int i = 0; i < ipStr.length(); i++)
    {
        if (ipStr.charAt(i) == '.' || i == ipStr.length() - 1)
        {
            if (i == ipStr.length() - 1)
            {
                i++;
            }
            String octet = ipStr.substring(fromIndex, i);
            octets[octetIndex] = octet.toInt();
            fromIndex = i + 1;
            octetIndex++;
        }
    }
    return IPAddress(octets[0], octets[1], octets[2], octets[3]);
}

bool configureNetwork(String IPAddressStr, String gatewayStr, String subnetStr, String dnsa)
{
    ip = stringToIPAddress(IPAddressStr);
    gateways = stringToIPAddress(gatewayStr);
    subnets = stringToIPAddress(subnetStr);
    dns = stringToIPAddress(dnsa);


    Serial.print("Configuring IP: ");
    Serial.println(ip);
    Serial.print("Configuring Gateway: ");
    Serial.println(gateways);
    Serial.print("Configuring Subnet: ");
    Serial.println(subnets);
    Serial.print("Configuring DNS: ");
    Serial.println(dns);

    return true;
}

void handleNetworksConfig(AsyncWebServerRequest *request)
{
    String IPStr, gwStr, subStr, dnsa;

    // Process each parameter if available
    if (request->hasParam("IPAddress", true))
    {
        IPStr = request->getParam("IPAddress", true)->value();
        updateEnvVariable("local_IP", IPStr); // Update local_IP environment variable
    }
    if (request->hasParam("gateway", true))
    {
        gwStr = request->getParam("gateway", true)->value();
        updateEnvVariable("gateway", gwStr); // Update gateway environment variable
    }
    if (request->hasParam("subnet", true))
    {
        subStr = request->getParam("subnet", true)->value();
        updateEnvVariable("subnet", subStr); // Update subnet environment variable
    }
    if (request->hasParam("dns", true))
    {
        dnsa = request->getParam("dns", true)->value();
        updateEnvVariable("dns", dnsa); // Update dns environment variable
    }

    // Configure network based on received parameters
    if (configureNetwork(IPStr.c_str(), gwStr.c_str(), subStr.c_str(), dnsa.c_str()))
    {
#ifdef DEBUG
        Serial.println("Setup new IP address done!");
#endif
        request->send(200, "text/plain", "Setup new IP address done!");
    }
    else
    {
#ifdef DEBUG
        Serial.println("Failed to update configuration");
#endif
        request->send(500, "text/plain", "Failed to update configuration");
    }
}
void handleSaveConfig(AsyncWebServerRequest *request)
{
  String action;
  if (request->hasParam("action", true))
  {
    action = request->getParam("action", true)->value();
    if (action == "complete")
    {
      skip = true;
      // Perform complete action
      request->send(200, "text/plain", "Complete action performed.");
    }
    else if (action == "restart")
    {
      request->send(200, "text/plain", "Restarting ESP.");
      delay(2000);
      ESP.restart();
    }
  }
}
