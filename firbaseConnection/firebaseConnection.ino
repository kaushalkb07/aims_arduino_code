#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Token
// 🔹 Define Pins for RFID RC522
#define SS_PIN D4
#define RST_PIN D3

MFRC522 mfrc522(SS_PIN, RST_PIN);  // RFID reader
LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD with I2C address 0x27

// 🔹 WiFi Credentials
const char* WIFI_SSID = "ssid";
const char* WIFI_PASSWORD = "pw";

// 🔹 Firebase Credentials
#define FIREBASE_HOST "your_db_host"
#define FIREBASE_AUTH "your_auth_key"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// 🔹 Function to Connect WiFi
void connectWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("🔹 Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\n✅ Connected to WiFi!");
}

// 🔹 Function to Get Timestamp
String getCurrentTime() {
    return String(millis());  // Using millis() as a simple timestamp
}

// 🔹 Function to Store Data in Firebase
bool storeDataInFirebase(String tagID) {
    FirebaseJson json;
    json.set("timestamp", getCurrentTime());  // Store timestamp
    json.set("tagID", tagID);                 // Store tag ID

    String entryPath = "/products/" + tagID;

    Serial.print("🔹 Writing to Firebase: ");
    Serial.println(entryPath);

    if (Firebase.setJSON(fbdo, entryPath.c_str(), json)) {  
        Serial.println("✅ Data stored successfully in Firebase!");
        return true;
    } else {
        Serial.print("❌ Firebase Error: ");
        Serial.println(fbdo.errorReason());
        return false;
    }
}

void setup() {
    Serial.begin(115200);
    SPI.begin();
    mfrc522.PCD_Init();
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Scan RFID Card");

    connectWiFi();  // Connect to WiFi

    // 🔹 Setup Firebase
    config.host = FIREBASE_HOST;
    config.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    Serial.println("✅ System Ready!");
}

void loop() {
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    String tagID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        tagID += String(mfrc522.uid.uidByte[i], HEX);
    }

    Serial.print("\n📌 Scanned RFID Tag: ");
    Serial.println(tagID);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Product Scanned!");
    lcd.setCursor(0, 1);
    lcd.print("Tag ID: " + tagID);

    // 🔹 Store Scanned Data in Firebase
    storeDataInFirebase(tagID);

    delay(2000);
}
