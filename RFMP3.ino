#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include <DFMiniMp3.h>

#define RST_PIN         9         
#define SS_PIN          10         

MFRC522 mfrc522(SS_PIN, RST_PIN);  

class Mp3Notify
{
public:
  static void PrintlnSourceAction(DfMp3_PlaySources source, const char* action)
  {
    if (source & DfMp3_PlaySources_Sd) 
    {
        Serial.print("SD Card, ");
    }
    if (source & DfMp3_PlaySources_Usb) 
    {
        Serial.print("USB Disk, ");
    }
    if (source & DfMp3_PlaySources_Flash) 
    {
        Serial.print("Flash, ");
    }
    Serial.println(action);
  }
  static void OnError(uint16_t errorCode)
  {
    Serial.println();
    Serial.print("Com Error ");
    Serial.println(errorCode);
  }
  static void OnPlayFinished(DfMp3_PlaySources source, uint16_t track)
  {
    Serial.print("Play finished for #");
    Serial.println(track);  
  }
  static void OnPlaySourceOnline(DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "online");
  }
  static void OnPlaySourceInserted(DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "inserted");
  }
  static void OnPlaySourceRemoved(DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "removed");
  }
};

SoftwareSerial secondarySerial(2, 3); // RX, TX
DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(secondarySerial);

void setup() 
{
  Serial.begin(115200);

  SPI.begin();      
  mfrc522.PCD_Init();   
  delay(100);      
  
  Serial.println("initializing...");
  
  mp3.begin();

  uint16_t volume = mp3.getVolume();
  delay(100);       

  Serial.println(volume);
  mp3.setVolume(10);
  
  uint16_t count = mp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
  delay(100);    
    
  Serial.print("files ");
  Serial.println(count);
  
  Serial.println("Ready...");
}

void waitMilliseconds(uint16_t msWait)
{
  uint32_t start = millis();
  
  while ((millis() - start) < msWait)
  {
    mp3.loop(); 
    delay(1);
  }
}

void loop() 
{
  GetSongIdFromCard();
}

void GetSongIdFromCard() {
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  Serial.print(F("UltraSerial"));
  MFRC522::StatusCode status;
  byte byteCount;
  byte buffer[18];
  byte i;

  Serial.println(F("Page  0  1  2  3"));
  byte page = 5;
  byteCount = sizeof(buffer);
  mfrc522.MIFARE_Read(page, buffer, &byteCount);
  String string = "";
  for (byte offset = 0; offset < 4; offset++) {
    i = page + offset;
    if (i < 10)
      Serial.print(F("  ")); // Pad with spaces
    else
      Serial.print(F(" ")); // Pad with spaces
    Serial.print(i);
    Serial.print(F("  "));
    for (byte index = 0; index < 4; index++) {
      i = 4 * offset + index;
      if (buffer[i] < 0x10)
        Serial.print(F(" 0"));
      else
        Serial.print(F(" "));
      Serial.print((char)buffer[i]);
      string += (char)buffer[i];
    }
    Serial.println();
  }
  Serial.println(string);
  if (string.indexOf("song/id") > 0)
  {
    byte start = string.indexOf("song/id") + 7;
    byte end = start + 4 ;
    String songid = string.substring(start, end);
    Serial.println("Playing Song " + songid);
    Serial.println(songid.toInt());
    mp3.playMp3FolderTrack(songid.toInt());
    waitMilliseconds(2000); 
  }
}
