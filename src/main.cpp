#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Initialize all pointers
BLEServer* pServer = NULL;                       
BLECharacteristic* pCharacteristic_1 = NULL;      
BLECharacteristic* pCharacteristic_2 = NULL;      
BLEDescriptor *pDescr_1;                          
BLE2902 *pBLE2902_1;                              
BLE2902 *pBLE2902_2;                              


bool deviceConnected = false;
bool oldDeviceConnected = false;


uint32_t value = 0;


// https://www.uuidgenerator.net/

#define SERVICE_UUID          "5fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_1 "ceb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_2 "2c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"

// Callback function that is called whenever a client is connected or disconnected

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic_1 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_1,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |  
                      BLECharacteristic::PROPERTY_NOTIFY
                    );                   

  pCharacteristic_2 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_2,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |                      
                      BLECharacteristic::PROPERTY_NOTIFY
                    );  

  // Create a BLE Descriptor  
  pDescr_1 = new BLEDescriptor((uint16_t)0x2901);
  pDescr_1->setValue("A very interesting variable");
  pCharacteristic_1->addDescriptor(pDescr_1);

  //BLE2902 Descriptors for  "PROPERTY_NOTIFY"
  pBLE2902_1 = new BLE2902();
  pBLE2902_1->setNotifications(true);                 
  pCharacteristic_1->addDescriptor(pBLE2902_1);

  pBLE2902_2 = new BLE2902();
  pBLE2902_2->setNotifications(true);
  pCharacteristic_2->addDescriptor(pBLE2902_2);

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    // notify changed value
    if (deviceConnected) {
      
      pCharacteristic_1->setValue(value);
      pCharacteristic_1->notify();
      value++;

      
      //get value from Cha 2
      std::string rxValue = pCharacteristic_2->getValue();
      Serial.print("Characteristic 2: ");
      Serial.println(rxValue.c_str());

  
      String txValue = "Server 2: " + String(random(1000));
      pCharacteristic_2->setValue(txValue.c_str());
      Serial.println("Characteristic 2: " + txValue);

     
      delay(5000);
    }
    
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); 
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}