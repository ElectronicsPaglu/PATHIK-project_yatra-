//println() shows up on the console(laptop)
//showMessage() prints onto the oled screen
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>
#include <ArduinoJson.h>
#include <AudioFileSourceHTTPStream.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>

// =====================================================
// WIFI
// =====================================================

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// -----------------------------------------------------
// SERVER (plain HTTP - laptop or Raspberry Pi on same
// WiFi/hotspot as the ESP32)
// -----------------------------------------------------

const char* SERVER = "YOUR_IP_ADDRESS(changes every time you change wifi)";


// =====================================================
// OLED
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);


// =====================================================
// GPS
// =====================================================

TinyGPSPlus gps;

HardwareSerial GPSserial(2);

#define GPS_RX 4
#define GPS_TX 2

#define GPS_BAUD 9600

// =====================================================
// GPS FALLBACK
// =====================================================

const unsigned long GPS_FIX_TIMEOUT = 15000; // 15 sec to get a real fix
unsigned long gpsWaitStart = 0;
bool gpsFallbackUsed = false;

// Default fallback coordinates (used only if no real fix in time)
const double FALLBACK_LAT = 28.6139; // Delhi
const double FALLBACK_LNG = 77.2090;

// =====================================================
// BUTTONS
// =====================================================

#define BUTTON_HISTORY       14
#define BUTTON_TOURIST_SPOT  33
#define BUTTON_HIDDEN_GEM    13
#define BUTTON_FOOD          18
#define BUTTON_EXIT          19

// =====================================================
// I2S / MAX98357A
// =====================================================

#define I2S_DIN   27
#define I2S_BCLK  26
#define I2S_LRC   25


// =====================================================
// LOCATION
// =====================================================

double latitude = 0.0;
double longitude = 0.0;
AudioFileSourceHTTPStream *audioFile;
AudioGeneratorMP3 *audioGenerator;
AudioOutputI2S *audioOutput;

String currentState = "unknown";
bool locationReady = false;


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  // Buttons

  pinMode(BUTTON_HISTORY, INPUT_PULLUP);
  pinMode(BUTTON_TOURIST_SPOT, INPUT_PULLUP);
  pinMode(BUTTON_HIDDEN_GEM, INPUT_PULLUP);
  pinMode(BUTTON_FOOD, INPUT_PULLUP);
  pinMode(BUTTON_EXIT, INPUT_PULLUP);

  // OLED

  Wire.begin(21, 23);

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDRESS
      )) {

    Serial.println("OLED initialization failed!");

    while (true) {
      delay(1000);
    }
  }

  // GPS

  GPSserial.begin(
    GPS_BAUD,
    SERIAL_8N1,
    GPS_RX,
    GPS_TX
  );

  // Startup screen
  showMessage("PATHIK", "Starting...");

  // WiFi
  Serial.println("Attempting to start wifi....");
  connectWiFi();
  showMessage("PATHIK", "Ready!");
  delay(2000);
}


// =====================================================
// MAIN LOOP
// =====================================================

const unsigned long LOCATION_CHECK_INTERVAL = 10000; // 10 sec
unsigned long lastLocationCheck = 0;

void loop() {
  readGPS();   // Continuously read GPS

  // Get location only until we have a state
  if (!locationReady) {
    if (millis() - lastLocationCheck > LOCATION_CHECK_INTERVAL) {

      if (sendLocationToServer()) {
        locationReady = true;

        showMessage("Location:", currentState);
        delay(2000);

        showMessage("Select Category", "Hist/Spot/Gem/Food");
      }

      lastLocationCheck = millis();
    }
  }

  // Check buttons

  if (digitalRead(BUTTON_HISTORY) == LOW) {
    handleCategory("history");
    delay(500);
  }

  if (digitalRead(BUTTON_TOURIST_SPOT) == LOW) {
    handleCategory("tourist_spot");
    delay(500);
  }

  if (digitalRead(BUTTON_HIDDEN_GEM) == LOW) {
    handleCategory("hidden_gem");
    delay(500);
  }

  if (digitalRead(BUTTON_FOOD) == LOW) {
    handleCategory("food");
    delay(500);
  }
}

// =====================================================
// WIFI
// =====================================================

void connectWiFi() {
  showMessage("WiFi", "Connecting...");
  Serial.println();
  Serial.println("Connecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts = attempts + 1;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
    Serial.println(gps.location.isValid());

    showMessage("WiFi Connected", WiFi.localIP().toString());
    delay(2000);

  } else {
    Serial.println("WiFi connection failed!");
    showMessage("WiFi Error", "Check connection");
    delay(2000);
  }
}


// =====================================================
// GPS READING
// =====================================================

void readGPS() {
  while (GPSserial.available()) {
    gps.encode(GPSserial.read());
  }

  if (gps.location.isUpdated()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();

    Serial.print("Latitude: ");
    Serial.println(latitude, 6);
    Serial.print("Longitude: ");
    Serial.println(longitude, 6);
  }
}


// =====================================================
// SEND LOCATION TO FLASK
// =====================================================

bool sendLocationToServer() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected.");
    connectWiFi();
    if (WiFi.status() != WL_CONNECTED) {
      return false;
    }
  }

  if (!gps.location.isValid()) {

    if (gpsWaitStart == 0) {
      gpsWaitStart = millis();
    }

    if (millis() - gpsWaitStart < GPS_FIX_TIMEOUT) {
      Serial.println("GPS location invalid.");
      showMessage("GPS", "Looking for GPS...");
      return false;
    }

    // GPS timeout fallback for reliable demonstration/testing.
    // Delhi coordinates are used only when no valid GPS fix is available.
    Serial.println("GPS fix timed out. Using fallback location.");
    latitude = FALLBACK_LAT;
    longitude = FALLBACK_LNG;
    gpsFallbackUsed = true;

  } else {
    // A real GPS fix is available, so use it and clear the fallback timer.
    gpsWaitStart = 0;
    gpsFallbackUsed = false;
    latitude = gps.location.lat();
    longitude = gps.location.lng();
  }

  HTTPClient http;
  String url = String(SERVER) + "/api/location";

  Serial.print("Sending location to: ");
  Serial.println(url);

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<128> requestDoc;
  requestDoc["latitude"] = latitude;
  requestDoc["longitude"] = longitude;

  String json;
  serializeJson(requestDoc, json);

  Serial.println("JSON:");
  Serial.println(json);

  int responseCode = http.POST(json);

  Serial.print("HTTP response: ");
  Serial.println(responseCode);

  bool success = false;

  if (responseCode > 0) {
    String response = http.getString();
    Serial.println("Server response:");
    Serial.println(response);

    StaticJsonDocument<256> responseDoc;
    DeserializationError error = deserializeJson(responseDoc, response);

    if (!error) {
      const char* state = responseDoc["state"];

      if (state != nullptr) {
        currentState = String(state);
        Serial.print("Detected state: ");
        Serial.println(currentState);
        success = true;
      } else {
        Serial.println("No 'state' field in response.");
      }
    } else {
      Serial.print("JSON parse failed: ");
      Serial.println(error.c_str());
    }
  }

  http.end();
  return success;
}


// =====================================================
// CATEGORY HANDLER
// =====================================================

void handleCategory(String category) {
  Serial.println();
  Serial.println("======================");
  Serial.print("Category: ");
  Serial.println(category);

  showMessage("Checking GPS", "Please wait...");

  if (!locationReady) {
  bool locationOK = sendLocationToServer();

  if (!locationOK) {
    showMessage("GPS Error", "No location");
    delay(2000);
    return;
  }

  locationReady = true;
}

  if (currentState == "unknown") {
    showMessage("Unsupported", "region");
    delay(2000);
    return;
  }

  getTouristInfo(currentState, category);
  requestAudio(currentState, category);

  menuLoop(currentState);
}

// =====================================================
// GET TOURIST INFORMATION
// =====================================================

void getTouristInfo(String state, String category) {

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    if (WiFi.status() != WL_CONNECTED) {
      return;
    }
  }

  HTTPClient http;
  String url = String(SERVER) + "/api/info/" + state + "/" + category;

  Serial.print("Requesting info: ");
  Serial.println(url);

  http.begin(url);
  int responseCode = http.GET();

  if (responseCode > 0) {
    String response = http.getString();

    Serial.println("Information:");
    Serial.println(response);

    StaticJsonDocument<512> infoDoc;
    DeserializationError error = deserializeJson(infoDoc, response);

    if (!error && infoDoc.containsKey("title")) {
      const char* title = infoDoc["title"];
      const char* text  = infoDoc["text"];

      showMessage(String(title), String(text));
    } else {
      showMessage(category, "No info found");
    }

    delay(3000);
  } else {
    Serial.println("Failed to get information.");
    showMessage("Server Error", "No information");
    delay(2000);
  }

  http.end();
}

// =====================================================
// REQUEST AUDIO
// =====================================================

void requestAudio(String state, String category) {
  String url = String(SERVER) + "/api/audio/" + state + "/" + category;

  Serial.println();
  Serial.println("Audio URL:");
  Serial.println(url);

  showMessage("Audio", "Loading...");

  audioOutput = new AudioOutputI2S();

  audioOutput->SetPinout(
    I2S_BCLK,
    I2S_LRC,
    I2S_DIN
  );

  audioOutput->SetGain(2.0);

  audioFile = new AudioFileSourceHTTPStream(url.c_str());
  audioGenerator = new AudioGeneratorMP3();

  bool started = audioGenerator->begin(audioFile, audioOutput);

  if (!started) {
    Serial.println("Failed to start audio playback.");
    showMessage("Audio Error", "Playback failed");
    delay(2000);

    delete audioFile;
    delete audioGenerator;
    delete audioOutput;

    return;
  }

  showMessage("Audio", "Playing...");

  while (audioGenerator->isRunning()) {
    if (!audioGenerator->loop()) {
      audioGenerator->stop();
    }
  }

  Serial.println("Audio playback finished.");
  showMessage("Audio", "Done");
  delay(1000);

  delete audioFile;
  delete audioGenerator;
  delete audioOutput;
}

// =====================================================
// Menu loop (repeats after each narration until Exit)
// =====================================================

void menuLoop(String state) {
  bool exitRequested = false;

  while (!exitRequested) {
    showMessage("Select category:", "Hist/Spot/Gem/Food/Exit");
    bool choiceMade = false;

    while (!choiceMade) {
      if (digitalRead(BUTTON_HISTORY) == LOW) {

        getTouristInfo(state, "history");
        requestAudio(state, "history");

        choiceMade = true;

      } else if (digitalRead(BUTTON_TOURIST_SPOT) == LOW) {

        getTouristInfo(state, "tourist_spot");
        requestAudio(state, "tourist_spot");

        choiceMade = true;

      } else if (digitalRead(BUTTON_HIDDEN_GEM) == LOW) {

        getTouristInfo(state, "hidden_gem");
        requestAudio(state, "hidden_gem");

        choiceMade = true;

      } else if (digitalRead(BUTTON_FOOD) == LOW) {

        getTouristInfo(state, "food");
        requestAudio(state, "food");

        choiceMade = true;

      } else if (digitalRead(BUTTON_EXIT) == LOW) {

        exitRequested = true;
        choiceMade = true;
      }

      delay(50);
    }

    delay(500);
  }

  showMessage("Thank you!", "Safe travels");
  delay(2000);
}

// =====================================================
// OLED MESSAGE
// =====================================================

void showMessage(String line1, String line2) {

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println(line1);

  display.setCursor(0, 20);
  display.println(line2);

  display.display();
}