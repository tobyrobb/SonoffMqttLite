//the minimum things i could think of to make my sonoff work with MQTT
//some more notes at the end

#include <PubSubClient.h>
#include <ESP8266WiFi.h>

//fill in the details below

#define ssid "yourwifi"
#define ssidPword "yourpassword*
#define mqttDevice "the downstairs lights"
#define mqttUser "ausername"
#define mqttPword "auserpassword" 
#define mqttPort 8883
#define commandTopic "the downstairs lights/switch"
#define stateTopic "the downstairs lights/state"
#define messageConnected "the downstairs lights Online"
#define messageSwitchON "the downstairs lights ON"
#define messageSwitchOFF "the downstairs lights OFF"
#define buttonPin 0
#define relayPin 12
#define ledPin 13

boolean relayONstate = true;

//byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress rasPi(192, 168, 1, 10);

WiFiClient wifiClient;

PubSubClient client(rasPi, mqttPort, wifiClient);

void setup()
{
////Serial.begin(115200);
  pinMode(relayPin, OUTPUT);  //relay
  pinMode(ledPin, OUTPUT);  //led
  pinMode(buttonPin, INPUT_PULLUP);
  
client.setCallback(callback);
WiFi.begin(ssid, ssidPword);

 while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    ////Serial.print(".");
  }
  //Serial.println("");
  //Serial.println("WiFi connected");  
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
  
  if (client.connect(mqttDevice, mqttUser, mqttPword)) {
    
    client.subscribe(commandTopic);
    //Serial.println("Connected OK");
  if (client.publish(stateTopic,messageConnected)) {
      //Serial.println("Publish ok");
    }
    else {
      //Serial.println("Publish failed");
    }
  }
  else {
    //Serial.println("MQTT connect failed");
    //Serial.println("Will reset and try again...");
    abort();
  }
}

void loop(){
  //Serial.print("The relayONstate value is: ");
  //Serial.println(relayONstate);
  digitalWrite(ledPin, !relayONstate);
  digitalWrite(relayPin, relayONstate);
  
  if(digitalRead(buttonPin)==LOW){
     //Serial.println("Button pushed");
     relayONstate = !relayONstate;
     //Serial.println("Turning relay on");
       }
  
    if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqttDevice, mqttUser, mqttPword)) {
      //Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(stateTopic,messageConnected);
      // ... and resubscribe
      client.subscribe(commandTopic);
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void callback(char* topic, byte* payload, unsigned int length) {
  //Serial.print("Message arrived [");
  //Serial.print(topic);
  //Serial.print("] ");
  for (int i=0;i<length;i++) {
    //Serial.print((char)payload[i]);
  }
  //Serial.println();

  // Switch on the relay if an 1 was received as first character
  if ((char)payload[0] == '1') {
      // Turn the relay on
  relayONstate = true;
  if (client.publish(stateTopic,messageSwitchON)) {
      //Serial.println("Switching ON");
    }
    else {
      //Serial.println("Publish failed");
    }
  }
  
  else {
      // Now turn the relay off
 relayONstate = false;
  
  if (client.publish(stateTopic,messageSwitchOFF)) {
      //Serial.println("Switching OFF");
    }
    else {
      //Serial.println("Publish failed");
    }
  }
  
}

/*
 * 
to install a broker on raspi or linux etc:--

mosquitto broker client
 
// here are some example pi commands
//sudo apt-get install mosquitto
//sudo apt-get install mosquitto-clients
//sudo mosquitto_passwd -c pwfile user
//sudo mosquitto -p 8883 -c /etc/mosquitto/mosquitto.conf
//mosquitto_sub -h 192.168.1.40 -p 8883 -t test -u user -P raspberry
//mosquitto_pub -p 8883 -u user -P password -m "hello world" -t test

//HERE IS THE mosquitto.conf file for raspi
------------
# Place your local configuration in /etc/mosquitto/conf.d/
# A full description of the configuration file is at
# /usr/share/doc/mosquitto/examples/mosquitto.conf.example
persistence true
persistence_location /var/lib/mosquitto/
persistence_file mosquitto.db
log_dest syslog
log_dest stdout
log_dest topic
log_type error
log_type warning
log_type notice
log_type information
connection_messages true
log_timestamp true
allow_anonymous false
password_file /etc/mosquitto/pwfile
------

using this with IFTT and google home, 
if
google assistant hears phrase " Turn "the downstairs lights" on"
then makers channel will request the web page http://192.168.1.40/mqtt.php?topic=the downstairs lights&message=1
goes to your raspi apache web server 
with php to mqtt script for sending mqtt to mosquittto broker   

*/
