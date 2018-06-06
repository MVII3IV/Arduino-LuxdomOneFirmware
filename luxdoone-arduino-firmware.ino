#include <ArduinoJson.h>
String deviceId = "0013A200_40EAE365";
int numberOfRelays = 0;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

//SERIAL CONFIGURATIONS
int BAUDRATE = 9600;

//RELAYS
int  output01 = 4; //0
int  output02 = 5; //1
int  output03 = 6; //2
int  output04 = 7; //3


int relays[] = {
	output01,
	output02,
	output03,
	output04
};



void setup() {

	Serial.begin(BAUDRATE);
	// reserve 200 bytes for the inputString:
	inputString.reserve(200);

	numberOfRelays = sizeof(relays) / sizeof(int);

	Serial.println("Luxdom system loaded..");
	Serial.println("Xbee ID : " + deviceId);


	for (int i = 0; i < numberOfRelays; i++) {
		pinMode(relays[i], OUTPUT);
		digitalWrite(relays[i], LOW);
	}

	Serial.println("Number of relays : " + String(numberOfRelays));
}

void loop() {
	if (stringComplete) {

		//{"deviceId":"0013A200_40EAE365","relay":1,"instruction":0}
		//{"id":"0013A200_40EAE365","rel":0,"ins":1,"typ":0}
		StaticJsonBuffer<100> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(inputString);
		String id = root["id"];
		int relay = root["rel"];
		int instruction = root["ins"];
		int type = root["typ"];

		// clear the string:
		inputString = "";
		stringComplete = false;


		if (!root.success()) {
			Serial.println("parseObject() failed");
			return;
		}

		if (deviceId != id) {
			Serial.println("It's not my duty");
			return;
		}

		if (relay < 0) {
			Serial.println("Relay not valid : " + String(relay));
			Serial.println("Number of relays : " + String(numberOfRelays));
			return;
		}

		Serial.println("\n");
		Serial.println("DeviceID : " + id);
		Serial.println("Relay : " + String(relay));
		Serial.println("Instruction : " + String(instruction));
		Serial.println("type : " + String(type));

		//turn on all relays
		if (relay == 100) {
			for (int x = 0; x < numberOfRelays; x++) {
				digitalWrite(relays[x], 1);
				delay(100);
			}
		}
		//turn off all relays
		else if (relay == 200) {
			for (int x = 0; x < numberOfRelays; x++) {
				digitalWrite(relays[x], 0);
				delay(100);
			}
		}

		switch (type) {
			case 1 : standardDeviceOrder(relay, instruction); break;
			case 2: pulseDeviceOrder(relay); break;
		}
		
		sendResponse(relay, instruction, type);
	}
	delay(100);
}

void standardDeviceOrder(int relay, int instruction) {
	digitalWrite(relays[relay], instruction);
}

void pulseDeviceOrder(int relay) {
	digitalWrite(relays[relay], true);
	delay(500);
	digitalWrite(relays[relay], false);
}

void sendResponse(int relay, int state, int type) {
	StaticJsonBuffer<200> jsonBuff;

	JsonObject& root = jsonBuff.createObject();
	root["id"] = deviceId;
	root["rel"] = relay;
	root["ins"] = state;
	root["typ"] = type;

	root.printTo(Serial);
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
	while (Serial.available()) {
		// get the new byte:
		char inChar = (char)Serial.read();
		// add it to the inputString:
		inputString += inChar;
		// if the incoming character is a newline, set a flag
		// so the main loop can do something about it:
		if (inChar == '?') {
			stringComplete = true;
		}
	}
}


