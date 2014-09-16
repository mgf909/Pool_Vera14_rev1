
//send the Values to Vera - send every postingInterval
void sendData( float roofTemp,float poolTemp, float pH, int pTime,int acidTime) {
	gw.send(roofTempmsg.set(roofTemp, 1));
	gw.send(poolTempmsg.set(poolTemp, 1));
	gw.send(pHValuemsg.set(avgMeasuredPH, 1));
	gw.send(prtValuemsg.set((pTime/60 )));
	gw.send(hclValuemsg.set((acidTime/60)));
	// note the time that the connection was made or attempted:
	lastConnectionTime = millis();

}

void incomingMessage(const MyMessage &message) {
	// We only expect one type of message from controller. But we better check anyway.
	if (message.isAck()) {
		Serial.println("This is an ack from gateway");
	}

	if (message.type == V_LIGHT) {
		// Change relay state
		bool state = message.getBool();
	//	digitalWrite(RELAY_PIN, state?RELAY_ON:RELAY_OFF);
	//	// Store state in eeprom
	//	gw.saveState(CHILD_ID, state);
		
		// Write some debug info
		Serial.print("Incoming change for sensor:");
		Serial.print(message.sensor);
		Serial.print(", New status: ");
		Serial.println(state);
		
	}
}