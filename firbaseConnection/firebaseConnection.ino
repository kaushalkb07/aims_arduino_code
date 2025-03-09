#include <Wire.h>
#include <LiquidCrystal_PCF8574.h> // Use this instead of LiquidCrystal_I2C
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Define Pins for RFID RC522
#define SS_PIN D4
#define RST_PIN D3

MFRC522 mfrc522(SS_PIN, RST_PIN);  // RFID reader
LiquidCrystal_PCF8574 lcd(0x27);    // LCD with I2C address 0x27

// WiFi Credentials
const char* WIFI_SSID = "AdvancedCollege";
const char* WIFI_PASSWORD = "acem@123";

// Firebase Credentials
#define FIREBASE_HOST "https://aims-e842c-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "AIzaSyCrOXfw92OgJLIjDh4MVeVluf0hU8NqDS0"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Maximum 10 tags can be stored in memory (for ESP8266)
String tagIDs[10];  // Store tag IDs
bool tagStates[10]; // Store entry (true) or exit (false)
int tagCount = 0;   // Track number of stored tags

// Function to Connect WiFi
void connectWiFi() {
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\n✅ Connected to WiFi!");
}

// Function to Get Timestamp
String getCurrentTime() {
    return String(millis());  // Using millis() as a simple timestamp
}

// Function to Store Data in Firebase
bool storeDataInFirebase(String tagID, bool isEntry) {
    FirebaseJson json;
    String timestamp = getCurrentTime();
    json.set("tagID", tagID);
    json.set("timestamp", timestamp);

    String eventType = isEntry ? "products_entry" : "products_exit";
    String entryPath = "/" + eventType + "/" + tagID + "/" + timestamp;

    Serial.print("Writing to Firebase: ");
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

// Function to Display on LCD
void displayOnLCD(String tagID, String status) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(status);  // "Entry" or "Exit"
    lcd.setCursor(0, 1);
    lcd.print("Tag: " + tagID);
}

// Function to check and update tag state
bool updateTagState(String tagID) {
    for (int i = 0; i < tagCount; i++) {
        if (tagIDs[i] == tagID) {
            tagStates[i] = !tagStates[i]; // Toggle entry/exit
            return tagStates[i];
        }
    }
    
    // If tag is new and storage is available
    if (tagCount < 10) {
        tagIDs[tagCount] = tagID;
        tagStates[tagCount] = true; // First scan is always entry
        tagCount++;
        return true;
    }
    
    return true; // Default to entry (should never happen)
}

void setup() {
    Serial.begin(115200);
    SPI.begin();
    mfrc522.PCD_Init();
    lcd.begin(16, 2);
    lcd.setBacklight(255);
    lcd.setCursor(0, 0);
    lcd.print("Scan RFID Card");

    connectWiFi();  // Connect to WiFi

    // Setup Firebase
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

    // Read RFID tag ID
    String tagID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        tagID += String(mfrc522.uid.uidByte[i], HEX);
    }

    Serial.print("\n📌 Scanned RFID Tag: ");
    Serial.println(tagID);

    // Determine if it's entry or exit
    bool isEntry = updateTagState(tagID);

    // Display entry/exit status on LCD
    displayOnLCD(tagID, isEntry ? "Entry" : "Exit");

    // Store data in Firebase
    storeDataInFirebase(tagID, isEntry);

    // Halt scanning for a short time to allow new card detection
    delay(1000);
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}
