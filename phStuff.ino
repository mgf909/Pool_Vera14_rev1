//pH stuff

float measurePHVolts()
{
	float phADC = analogRead(phPin);
	float phVolts = (phADC/1024.0)*5.0;
	return phVolts;
}

float getTempAdjusted4()
{
	//http://www.omega.com/Green/pdf/pHbasics_REF.pdf
	float adjustTemp = calibrationTempC;
	float difference = adjustTemp-25;
	float phAdjust = (0.009 * difference);
	float tempAdjusted4 = 4 + phAdjust;
	return tempAdjusted4;
}

float measurePH()
{

	float phVolt = measurePHVolts();

	float tempAdjusted4 = getTempAdjusted4();
	float voltsPerPH = (abs(volt7-volt4)) / (7-tempAdjusted4);
	float realPHVolt = (volt7 - phVolt);
	float phUnits = realPHVolt / voltsPerPH;
	float measuredPH = 7 + phUnits;

	return measuredPH;
}

float doPHTempCompensation(float PH, float temp)
{
	float difference = temp-25;
	float phAdjust = (0.009 * difference);
	float tempAdjustedPH = PH + phAdjust;
	return tempAdjustedPH;
}

