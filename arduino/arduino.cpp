#define PT_USE_TIMER
#define PT_USE_SEM
#include <pt.h>

#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN         0          
#define SS_PIN          5

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
const char* ssid     = "wifiForEsp8266";
const char* password = "ILOVEYOU";

#include <EEPROM.h>
char cardNum = 0;
char cards[6][4];

static struct pt thread1;
static struct pt ReadCardThread;
static struct pt HttpServerThread;
static struct pt_sem sem_mfrc522;
char m = 0,n = 0;

MFRC522 mfrc522(SS_PIN, RST_PIN);

ESP8266WebServer server(80);

void ReadEEPROM();
void handleNotFound();
void handleInterFace()
{
    Serial.println("Enter Interface");
    String content;
    char count = 0;
    content = "<?xml version='1.0' encoding='ISO-8859-1'?><cards>";
    if(server.hasArg("cmd"))
    {
        if(server.arg("cmd")=="getUidPer")
        {
          Serial.println("getUidPer");
            for(int i=0;i<cardNum;i++)
            {
                content += "<card>";
                for(int j=0;j<4;j++)
                {
                    content +="<";
                    content += (char)(j+65);
                    content +=">";
                    content +=(int)(cards[i][j]);
                    content +="</";
                    content += (char)(j+65);
                    content +=">";
                }
                content += "</card>";
            } 
        }
        if(server.arg("cmd")=="readUid")
        {
          Serial.println("readUid");
            //ÑÓÊ±5sµÈ´ý¶ÁÈ¡¿¨Æ¬
            while(!mfrc522.PICC_IsNewCardPresent()&&count < 5)
            {
                delay(500);
                count++;
            }
            if ( ! mfrc522.PICC_ReadCardSerial())
            {
                content += "<card>";
                for(int j = 0;j<4;j++)
                {
                    content +="<";
                    content += (char)(j+65);
                    content +=">";
                    content +=(int)(-1);
                    content +="</";
                    content += (char)(j+65);
                    content +=">";
                }
                content += "</card>";
            }
            else
            {
                Serial.println(mfrc522.uid.uidByte[0]);
                Serial.println(mfrc522.uid.uidByte[1]);
                Serial.println(mfrc522.uid.uidByte[2]);
                Serial.println(mfrc522.uid.uidByte[3]);
                content += "<card>";
                for(int j = 0;j<4;j++)
                {
                  content +="<";
                  content += (char)(j+65);
                  content +=">";
                  content +=(int)(mfrc522.uid.uidByte[j]);
                  content +="</";
                  content += (char)(j+65);
                  content +=">";
                }
                content += "</card>";
            }
        }
        {
            
            if(server.arg("cmd")=="deleteUid")
            {
              Serial.println("deleteuID");
                char key[4];
                char i=0,j=0;
                key[0] = server.arg("A").toInt();
                key[1] = server.arg("B").toInt();
                key[2] = server.arg("C").toInt();
                key[3] = server.arg("D").toInt();
                for(i=0;i<cardNum;i++)
                {
                    for(j=0;j<4;j++)
                    {
                        if(cards[i][j]!=key[j])
                            break;
                    }
                    if(j==4)
                        break;
                }
                if(j==4)
                {
                    EEPROM.write(0,cardNum-1);
                    for(int k=i;k<cardNum-1;k++)
                    {
                        EEPROM.write(k*4+1,EEPROM.read(4*k+5));
                        EEPROM.write(k*4+2,EEPROM.read(4*k+6));
                        EEPROM.write(k*4+3,EEPROM.read(4*k+7));
                        EEPROM.write(k*4+4,EEPROM.read(4*k+8));
                    }
                    EEPROM.commit();
                    ReadEEPROM();
                    content += "<card><A>1</A></card>";
                }
                else
                    content += "<card><A>2</A></card>";
            }
        }
        {
            
            if(server.arg("cmd")=="addUid")
            {
              Serial.println("addUid");
                char key[4];
                char i=0,j=0;
                key[0] = server.arg("A").toInt();
                key[1] = server.arg("B").toInt();
                key[2] = server.arg("C").toInt();
                key[3] = server.arg("D").toInt();
                for(i=0;i<cardNum;i++)
                {
                    for(j=0;j<4;j++)
                    {
                        if(cards[i][j]!=key[j])
                            break;
                    }
                    if(j==4)
                        break;
                }
                if(j==4)
                {
                    content += "<card><A>2</A></card>";
                }
                else
                {
                    EEPROM.write(0,cardNum+1);
                    for(int k=0;k<4;k++)
                        EEPROM.write(cardNum*4+k+1,key[k]);
                    EEPROM.commit();
                    ReadEEPROM();
                    content += "<card><A>1</A></card>";
                    
                }
            }
        }
        content+="</cards>";
        server.send(200, "text/html", content);
    }
}
//root page can be accessed only if authentification is ok
void handleRoot(){
  Serial.println("Enter handleRoot");
  String content;
  content = "<html><head><script src='https://devoutprayer.github.io/TheDoor/load.js'></script><script>ajaxGetHTML('https://devoutprayer.github.io/TheDoor/thedoor.html');</script></head></html>";
  server.send(200, "text/html", content);
}

void setup() {
  //Init Serial
  Serial.begin(9600);
  while(!Serial);
  //Init EEPROM
  EEPROM.begin(1024);
  ReadEEPROM(); 
  
  //Init MFRC522 series
  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();
  
  pinMode(LED_BUILTIN, OUTPUT);
  //Init HttpServer series
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.on("/InterFace",handleInterFace);
  server.onNotFound(handleNotFound);
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent","Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize );
  server.begin();
  Serial.println("HTTP server started");
  Serial.println(cardNum+10);
  //Init Threads
  PT_SEM_INIT(&sem_mfrc522,1);
  PT_INIT(&thread1);
  PT_INIT(&ReadCardThread);
  PT_INIT(&HttpServerThread);
}

void loop() {
  // put your main code here, to run repeatedly:
  Task1(&thread1);
  ReadCard(&ReadCardThread);
  HttpServer(&HttpServerThread);
}
//ReadCardThread
static int ReadCard(struct pt *pt){
  PT_BEGIN(pt);
  PT_SEM_WAIT(pt,&sem_mfrc522);
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    goto A;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    goto A;
  }
  for(m = 0; m < cardNum; m++)
  {
    for(n = 0; n < 4; n++)
      if(mfrc522.uid.uidByte[n]!=cards[m][n])
        break;
    if(n == 4)
      break;
  }
  if(n == 4)
  {
    Serial.println("OK!");
  }
  else
  {
    Serial.println(mfrc522.uid.uidByte[0]);
    Serial.println(mfrc522.uid.uidByte[1]);
    Serial.println(mfrc522.uid.uidByte[2]);
    Serial.println(mfrc522.uid.uidByte[3]);
  }
A:
  PT_SEM_SIGNAL(pt,&sem_mfrc522);
  PT_YIELD(pt); 
  PT_TIMER_DELAY(pt,1000);
  PT_END(pt);
}
//HttpServerThread
static int HttpServer(struct pt *pt){
  PT_BEGIN(pt);
  server.handleClient();
  PT_END(pt);
}
//BLink
static int Task1(struct pt *pt){
  PT_BEGIN(pt);
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
                                    // but actually the LED is on; this is because 
                                    // it is acive low on the ESP-01)
  PT_TIMER_DELAY(pt,1000);                      // Wait for a second
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  PT_TIMER_DELAY(pt,1000);                      // Wait for two seconds (to demonstrate the active low LED)

  PT_END(pt);
}
//no need authentification
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
//read cards from EEPROM
void ReadEEPROM(){
  int addr = 0;
  cardNum = EEPROM.read(addr++);
  for(int i=0;i<cardNum;i++)
  {
    for(int j=0;j<4;j++)
      cards[i][j] = EEPROM.read(addr++);
  }
}