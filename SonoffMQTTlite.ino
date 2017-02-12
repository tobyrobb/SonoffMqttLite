//Arduino IDE 1.6.12
//SonoffMqttLite.ino
//Toby Robb //working reliably jan 2017
//the minimum things i could think of to make my sonoff work with MQTT


//LOTS OF NOTES AT THE BOTTOM!!


// find and replace //Serial with Serial to output some debugging messages
// OR
// find and replace Serial with //Serial to improve speed and reliability

#include <PubSubClient.h>  //thanks for a great library!
#include <ESP8266WiFi.h>  //thanks for a great library!

//network details
#define ssid "wifiName"     //wifi to connect to
#define ssidPword "wifiPassword"   //wifi password
int mqttIp[] = {192,168,1,1};   //MQTT broker ip
#define mqttPort 8883 //the mqtt broker port
#define mqttUser "brokerUser"    //mqtt broker user name
#define mqttPword "brokerPassword"  //mqtt broker user password (your using a password protected broker right?) Easy to do , check notes below.
#define mqttDevice "FF00DD kettle"  //report in as the last 6 digits of the mac adress plus a descriptor for example

//topics to respond to
//you will need to add subscribe and publish calls within the code for each topic you want. eg search on commandTopic to see where you need to add
#define commandTopic "FF00DD/switch"        //one of many possible phrases that the device listens out for
#define commandTopic1 "kettle/switch"       // add plenty, if using google home or any sort of voice recongnition
#define commandTopic2 "the kettle/switch"   // who knows how your voice will be translated
#define commandTopic3 "Kettle/switch"       //google home decided to pronounce kettle with a capital K, why? who knows? (better listen out for that tho.)

//reporting details
#define stateTopic "FF00DD kettle/state"  //publish switch condition into this here topic.
#define messageConnected "FF00DD kettle Online" //let everyone know you're online, don't be shy.
#define messageSwitchON "FF00DD kettle ON"  //publish switch change 
#define messageSwitchOFF "FF00DD kettle OFF"  //publish switch change

//device physical details
#define buttonPin 0 //for the sonoff
#define relayPin 12 //for the sonoff
#define ledPin 13 //for the sonoff

boolean relayONstate = true;  //start in the on condition by default

//off we go!
IPAddress rasPi(mqttIp[0],mqttIp[1],mqttIp[2],mqttIp[3]);
WiFiClient wifiClient;
PubSubClient client(rasPi, mqttPort, wifiClient);

void setup()
{
digitalWrite(relayPin, relayONstate); //as soon as we are plugged in set the relay
//Serial.begin(115200);
pinMode(relayPin, OUTPUT);  //relay
pinMode(ledPin, OUTPUT);  //led
pinMode(buttonPin, INPUT_PULLUP);
client.setCallback(callback);
WiFi.begin(ssid, ssidPword);

 while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }
  //Serial.println("");
  //Serial.println("WiFi connected");  
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
  //Serial.print("MAC address: ");
  //Serial.println(WiFi.macAddress());
  
  if (client.connect(mqttDevice, mqttUser, mqttPword)) {
//include all your topic choices from initial configuration   
    client.subscribe(commandTopic);
    client.subscribe(commandTopic1);
    client.subscribe(commandTopic2);
    client.subscribe(commandTopic3);
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
//include all your topic choices from initial configuration   
      client.subscribe(commandTopic);
      client.subscribe(commandTopic1);
      client.subscribe(commandTopic2);
      client.subscribe(commandTopic3);
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

NOTES on MQTT, Sonoff(esp8266) and Google Home etc
//to enable ESP8266 eg sonoff support to arduino IDE
add http://arduino.esp8266.com/stable/package_esp8266com_index.json
to your preferences additional boards manager URL's
select tools, boards, then board manger and search on 8266, add the package
NEVER program sonoff whilst connected to power (death likely)
upload by holding down button for half a second whilst connecting the 4 x  3.3V! tolerant (ONLY!) pins to a serial to usb converter (set its jumper to 3.3v!) 

//add MQTT support libraries to the IDE
search on and download the pubsub client from the libraries manager
**specifically for sonoff we have to go to pubsubclient\src\PubSubClient.h
and Change MQTT_MAX_PACKET_SIZE from 128 to at least 400


to install a broker on raspi or linux etc:--

mosquitto broker client
 
here are some example pi commands
//sudo apt-get install mosquitto
//sudo apt-get install mosquitto-clients
cd to the mosquitto directory
//cd /home/mosquitto
create a user
//sudo mosquitto_passwd -c pwfile user

edit the mosquitto.conf file and replace with below 
//sudo nano mosquitto.conf

HERE IS THE mosquitto.conf file for raspi
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

to run the broker
//sudo mosquitto -p 8883 -c /etc/mosquitto/mosquitto.conf
to listen to a topic
//mosquitto_sub -h 192.168.1.1 -p 8883 -t topicname -u user -P password
to publish to a topic
//mosquitto_pub -p 8883 -u user -P password -m "hello world" -t topicname

There are lots of MQTT clients for Android, IOS, PC etc to make buttons and apps etc to control your stuff

To be able to control directly from the web and use Google Home
//sudo apt-get install appache2
//sudo apt-get install php5
find phpMQTT on git hub from bluerhinos
https://github.com/bluerhinos/phpMQTT
alternatively create phpMQTT.php text file with contents from the bottom of this post and place in www/var/html which is the apache root directory
one last thing..
create a text file in the apache directory var/www/html called mqtt.php with the following contents
///////////

<?php
require("phpMQTT.php");

$topic = $_GET["topic"];
$message = $_GET["message"];

$mqtt = new phpMQTT("192.168.1.1", 8883, "outside caller coming from a browser"); //Change client name to something unique

if ($mqtt->connect(true,NULL,"admin","AllTheThings")) {
        $mqtt->publish($topic,$message);
        $mqtt->close();
}

?>

/////////////


use like this http://192.168.1.1//mqtt.php?topic=kettle&message=1

this would post the message "1" to the topic "kettle"  
the kettle example above listens on a range of kettley subjects in case you or google use different phrasing to get the job done.

*use it with tasker on adroid to make buttons or a control panel that makes an app like interface to all your stuff
*use it with IFTT and its "google assistant" and "maker" channels to get google home to GET the webpage when you execute a voice command
eg if google home hears phrase " Turn "the fan" on"
then makers channel will request the web page http://192.168.1.40/mqtt.php?topic=the fan&message=1
goes to your raspi apache web server with php to mqtt script >> sending mqtt to mosquittto broker   

//EXTRAS: if you use your phone or anything else away from the house, forward port 80 etc (default browser requests) to your LANS raspberry pi IP and port 80


//THIS IS THE phpMQTT.php file text contents
////////////////////////////////////////////

//<?php
//
///*
//   phpMQTT
//  A simple php class to connect/publish/subscribe to an MQTT broker
// 
//*/
//
///*
//  Licence
//
//  Copyright (c) 2010 Blue Rhinos Consulting | Andrew Milsted
//  andrew@bluerhinos.co.uk | http://www.bluerhinos.co.uk
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//  
//*/
//
///* phpMQTT */
//class phpMQTT {
//
//  private $socket;      /* holds the socket */
//  private $msgid = 1;     /* counter for message id */
//  public $keepalive = 10;   /* default keepalive timmer */
//  public $timesinceping;    /* host unix time, used to detect disconects */
//  public $topics = array();   /* used to store currently subscribed topics */
//  public $debug = false;    /* should output debug messages */
//  public $address;      /* broker address */
//  public $port;       /* broker port */
//  public $clientid;     /* client id sent to brocker */
//  public $will;       /* stores the will of the client */
//  private $username;      /* stores username */
//  private $password;      /* stores password */
//
//  function __construct($address, $port, $clientid){
//    $this->broker($address, $port, $clientid);
//  }
//
//  /* sets the broker details */
//  function broker($address, $port, $clientid){
//    $this->address = $address;
//    $this->port = $port;
//    $this->clientid = $clientid;    
//  }
//
//  function connect_auto($clean = true, $will = NULL, $username = NULL, $password = NULL){
//    while($this->connect($clean, $will, $username, $password)==false){
//      sleep(10);
//    }
//    return true;
//  }
//
//  /* connects to the broker 
//    inputs: $clean: should the client send a clean session flag */
//  function connect($clean = true, $will = NULL, $username = NULL, $password = NULL){
//    
//    if($will) $this->will = $will;
//    if($username) $this->username = $username;
//    if($password) $this->password = $password;
//
//    $address = gethostbyname($this->address); 
//    $this->socket = fsockopen($address, $this->port, $errno, $errstr, 60);
//
//    if (!$this->socket ) {
//        if($this->debug) error_log("fsockopen() $errno, $errstr \n");
//      return false;
//    }
//
//    stream_set_timeout($this->socket, 5);
//    stream_set_blocking($this->socket, 0);
//
//    $i = 0;
//    $buffer = "";
//
//    $buffer .= chr(0x00); $i++;
//    $buffer .= chr(0x06); $i++;
//    $buffer .= chr(0x4d); $i++;
//    $buffer .= chr(0x51); $i++;
//    $buffer .= chr(0x49); $i++;
//    $buffer .= chr(0x73); $i++;
//    $buffer .= chr(0x64); $i++;
//    $buffer .= chr(0x70); $i++;
//    $buffer .= chr(0x03); $i++;
//
//    //No Will
//    $var = 0;
//    if($clean) $var+=2;
//
//    //Add will info to header
//    if($this->will != NULL){
//      $var += 4; // Set will flag
//      $var += ($this->will['qos'] << 3); //Set will qos
//      if($this->will['retain']) $var += 32; //Set will retain
//    }
//
//    if($this->username != NULL) $var += 128;  //Add username to header
//    if($this->password != NULL) $var += 64; //Add password to header
//
//    $buffer .= chr($var); $i++;
//
//    //Keep alive
//    $buffer .= chr($this->keepalive >> 8); $i++;
//    $buffer .= chr($this->keepalive & 0xff); $i++;
//
//    $buffer .= $this->strwritestring($this->clientid,$i);
//
//    //Adding will to payload
//    if($this->will != NULL){
//      $buffer .= $this->strwritestring($this->will['topic'],$i);  
//      $buffer .= $this->strwritestring($this->will['content'],$i);
//    }
//
//    if($this->username) $buffer .= $this->strwritestring($this->username,$i);
//    if($this->password) $buffer .= $this->strwritestring($this->password,$i);
//
//    $head = "  ";
//    $head{0} = chr(0x10);
//    $head{1} = chr($i);
//
//    fwrite($this->socket, $head, 2);
//    fwrite($this->socket,  $buffer);
//
//    $string = $this->read(4);
//
//    if(ord($string{0})>>4 == 2 && $string{3} == chr(0)){
//      if($this->debug) echo "Connected to Broker\n"; 
//    }else{  
//      error_log(sprintf("Connection failed! (Error: 0x%02x 0x%02x)\n", 
//                              ord($string{0}),ord($string{3})));
//      return false;
//    }
//
//    $this->timesinceping = time();
//
//    return true;
//  }
//
//  /* read: reads in so many bytes */
//  function read($int = 8192, $nb = false){
//
//    //  print_r(socket_get_status($this->socket));
//    
//    $string="";
//    $togo = $int;
//    
//    if($nb){
//      return fread($this->socket, $togo);
//    }
//      
//    while (!feof($this->socket) && $togo>0) {
//      $fread = fread($this->socket, $togo);
//      $string .= $fread;
//      $togo = $int - strlen($string);
//    }
//    
//  
//    
//    
//      return $string;
//  }
//
//  /* subscribe: subscribes to topics */
//  function subscribe($topics, $qos = 0){
//    $i = 0;
//    $buffer = "";
//    $id = $this->msgid;
//    $buffer .= chr($id >> 8);  $i++;
//    $buffer .= chr($id % 256);  $i++;
//
//    foreach($topics as $key => $topic){
//      $buffer .= $this->strwritestring($key,$i);
//      $buffer .= chr($topic["qos"]);  $i++;
//      $this->topics[$key] = $topic; 
//    }
//
//    $cmd = 0x80;
//    //$qos
//    $cmd += ($qos << 1);
//
//
//    $head = chr($cmd);
//    $head .= chr($i);
//    
//    fwrite($this->socket, $head, 2);
//    fwrite($this->socket, $buffer, $i);
//    $string = $this->read(2);
//    
//    $bytes = ord(substr($string,1,1));
//    $string = $this->read($bytes);
//  }
//
//  /* ping: sends a keep alive ping */
//  function ping(){
//      $head = " ";
//      $head = chr(0xc0);    
//      $head .= chr(0x00);
//      fwrite($this->socket, $head, 2);
//      if($this->debug) echo "ping sent\n";
//  }
//
//  /* disconnect: sends a proper disconect cmd */
//  function disconnect(){
//      $head = " ";
//      $head{0} = chr(0xe0);   
//      $head{1} = chr(0x00);
//      fwrite($this->socket, $head, 2);
//  }
//
//  /* close: sends a proper disconect, then closes the socket */
//  function close(){
//    $this->disconnect();
//    fclose($this->socket);  
//  }
//
//  /* publish: publishes $content on a $topic */
//  function publish($topic, $content, $qos = 0, $retain = 0){
//
//    $i = 0;
//    $buffer = "";
//
//    $buffer .= $this->strwritestring($topic,$i);
//
//    //$buffer .= $this->strwritestring($content,$i);
//
//    if($qos){
//      $id = $this->msgid++;
//      $buffer .= chr($id >> 8);  $i++;
//      $buffer .= chr($id % 256);  $i++;
//    }
//
//    $buffer .= $content;
//    $i+=strlen($content);
//
//
//    $head = " ";
//    $cmd = 0x30;
//    if($qos) $cmd += $qos << 1;
//    if($retain) $cmd += 1;
//
//    $head{0} = chr($cmd);   
//    $head .= $this->setmsglength($i);
//
//    fwrite($this->socket, $head, strlen($head));
//    fwrite($this->socket, $buffer, $i);
//
//  }
//
//  /* message: processes a recieved topic */
//  function message($msg){
//      $tlen = (ord($msg{0})<<8) + ord($msg{1});
//      $topic = substr($msg,2,$tlen);
//      $msg = substr($msg,($tlen+2));
//      $found = 0;
//      foreach($this->topics as $key=>$top){
//        if( preg_match("/^".str_replace("#",".*",
//            str_replace("+","[^\/]*",
//              str_replace("/","\/",
//                str_replace("$",'\$',
//                  $key))))."$/",$topic) ){
//          if(is_callable($top['function'])){
//            call_user_func($top['function'],$topic,$msg);
//            $found = 1;
//          }
//        }
//      }
//
//      if($this->debug && !$found) echo "msg recieved but no match in subscriptions\n";
//  }
//
//  /* proc: the processing loop for an "allways on" client 
//    set true when you are doing other stuff in the loop good for watching something else at the same time */  
//  function proc( $loop = true){
//
//    if(1){
//      $sockets = array($this->socket);
//      $w = $e = NULL;
//      $cmd = 0;
//      
//        //$byte = fgetc($this->socket);
//      if(feof($this->socket)){
//        if($this->debug) echo "eof receive going to reconnect for good measure\n";
//        fclose($this->socket);
//        $this->connect_auto(false);
//        if(count($this->topics))
//          $this->subscribe($this->topics);  
//      }
//      
//      $byte = $this->read(1, true);
//      
//      if(!strlen($byte)){
//        if($loop){
//          usleep(100000);
//        }
//       
//      }else{ 
//      
//        $cmd = (int)(ord($byte)/16);
//        if($this->debug) echo "Recevid: $cmd\n";
//
//        $multiplier = 1; 
//        $value = 0;
//        do{
//          $digit = ord($this->read(1));
//          $value += ($digit & 127) * $multiplier; 
//          $multiplier *= 128;
//          }while (($digit & 128) != 0);
//
//        if($this->debug) echo "Fetching: $value\n";
//        
//        if($value)
//          $string = $this->read($value,"fetch");
//        
//        if($cmd){
//          switch($cmd){
//            case 3:
//              $this->message($string);
//            break;
//          }
//
//          $this->timesinceping = time();
//        }
//      }
//
//      if($this->timesinceping < (time() - $this->keepalive )){
//        if($this->debug) echo "not found something so ping\n";
//        $this->ping();  
//      }
//      
//
//      if($this->timesinceping<(time()-($this->keepalive*2))){
//        if($this->debug) echo "not seen a package in a while, disconnecting\n";
//        fclose($this->socket);
//        $this->connect_auto(false);
//        if(count($this->topics))
//          $this->subscribe($this->topics);
//      }
//
//    }
//    return 1;
//  }
//
//  /* getmsglength: */
//  function getmsglength(&$msg, &$i){
//
//    $multiplier = 1; 
//    $value = 0 ;
//    do{
//      $digit = ord($msg{$i});
//      $value += ($digit & 127) * $multiplier; 
//      $multiplier *= 128;
//      $i++;
//    }while (($digit & 128) != 0);
//
//    return $value;
//  }
//
//
//  /* setmsglength: */
//  function setmsglength($len){
//    $string = "";
//    do{
//      $digit = $len % 128;
//      $len = $len >> 7;
//      // if there are more digits to encode, set the top bit of this digit
//      if ( $len > 0 )
//        $digit = ($digit | 0x80);
//      $string .= chr($digit);
//    }while ( $len > 0 );
//    return $string;
//  }
//
//  /* strwritestring: writes a string to a buffer */
//  function strwritestring($str, &$i){
//    $ret = " ";
//    $len = strlen($str);
//    $msb = $len >> 8;
//    $lsb = $len % 256;
//    $ret = chr($msb);
//    $ret .= chr($lsb);
//    $ret .= $str;
//    $i += ($len+2);
//    return $ret;
//  }
//
//  function printstr($string){
//    $strlen = strlen($string);
//      for($j=0;$j<$strlen;$j++){
//        $num = ord($string{$j});
//        if($num > 31) 
//          $chr = $string{$j}; else $chr = " ";
//        printf("%4d: %08b : 0x%02x : %s \n",$j,$num,$num,$chr);
//      }
//  }
//}
//
//?>

//////////////////////////////////////////
