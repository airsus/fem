#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "cJSON.h"

#define StartTest -4E-7
#define StopTest 4E-7

readExpData(cJSON *probeInfo, int samplingFrequencyHz)
{
cJSON *impulseCmd;
cJSON *time, *times;
cJSON *voltage, *voltages;
double *newTimeValues, *timeValues, *voltageValues;
double timeAtMaxIntensity;
double minTime = DBL_MIN;
double maxTime = -DBL_MIN;
double maxVoltage = -DBL_MIN;
double stepSize;
int i, maxVoltageIndex, numPnts = 0, numSteps;
int startIndex, stopIndex;

	impulseCmd = cJSON_GetObjectItem(probeInfo, "impulseResponse");
	fprintf(stderr, "%s\n", cJSON_GetObjectItem(impulseCmd, "wavetype")->valuestring);

	times = cJSON_GetObjectItem(impulseCmd, "time");

	fprintf(stderr, "getting times\n");
/*
 * we don't know ahead of time how many data points there are, so I'll make
 * an initial pass through the data to get a count. interestly, it doesn't
 * look like I need to reset the 'times' variable to do the second count.
 */

	cJSON_ArrayForEach(time, times) {
		numPnts++;
/* 		if (numPnts < 30) printf("%e\n", time->valuedouble); */
		}

	fprintf(stderr, "num points %d\n", numPnts);

	if ((timeValues = (double *)malloc(sizeof(double) * numPnts)) == NULL) {
		fprintf(stderr, "in readExpData, couldn't allocate space for times\n");
		return(0);
		}

	if ((voltageValues = (double *)malloc(sizeof(double) * numPnts)) == NULL) {
		fprintf(stderr, "in readExpData, couldn't allocate space for voltages\n");
		return(0);
		}

	i = 0;
	cJSON_ArrayForEach(time, times) {
		timeValues[i++] = time->valuedouble;
/* 		if (i < 20) printf("%e\n", time->valuedouble); */
		}

	voltages = cJSON_GetObjectItem(impulseCmd, "voltage");

	fprintf(stderr, "getting voltages\n");

	i = 0;
	cJSON_ArrayForEach(voltage, voltages) {
		voltageValues[i++] = voltage->valuedouble;
/* 		printf("%e\n", voltage->valuedouble); */
		}


/* find the max voltage */

	for (i = 0; i < numPnts; i++) {
		if (maxVoltage < voltageValues[i]) {
			maxVoltage = voltageValues[i];
			maxVoltageIndex = i;
			}
		}

	fprintf(stderr, "index %d max voltage %e\n", maxVoltageIndex, maxVoltage);

/* normalize the voltages */

	for (i = 0; i < numPnts; i++)
		voltageValues[i] /= maxVoltage;


/*
 * center the time axis around the max intensity. the matlab code recalculated
 * the index of the maximum value, but the normalization shouldn't change
 * that, so I skipped it. note that I have to save the time associated with
 * the max intensity, because in the time array, it goes to 0 as I do the
 * centering.
 */

	timeAtMaxIntensity = timeValues[maxVoltageIndex];

	for (i = 0; i < numPnts; i++)
		timeValues[i] = timeValues[i] - timeAtMaxIntensity;

/* now find the min and max times */

	for (i = 0; i < numPnts; i++) {
		if (minTime > timeValues[i]) minTime = timeValues[i];

		if (maxTime < timeValues[i]) maxTime = timeValues[i];
		}

	fprintf(stderr, "min time %e max time %e\n", minTime, maxTime);
/* re-sample the data to match the Field II sampling frequency */

	fprintf(stderr, "sampling %d\n", samplingFrequencyHz);
	stepSize = 1.0 / samplingFrequencyHz;

	fprintf(stderr, "diff %e\n", maxTime - minTime);
	numSteps = (int )ceil(((maxTime - minTime) * samplingFrequencyHz));

	fprintf(stderr, "step size %e numSteps %d\n", stepSize, numSteps);

	if ((newTimeValues = (double *)malloc(sizeof(double) * numSteps)) == NULL) {
		fprintf(stderr, "in readExpData, couldn't allocate space for new times\n");
		return(0);
		}

	for (i = 0; i < numSteps; i++) {
		newTimeValues[i] = minTime + i * stepSize;
		}

/* find the indices of NewTime to use for the Field II impulse response */

	for (i = 0; i < numSteps; i++) {
		if (newTimeValues[i] > StartTest) {
			startIndex = i;
			break;
			}
		}

	for (i = 0; i < numSteps; i++)
		if (newTimeValues[i] > StopTest) {
			stopIndex = i;
			break;
			}

	fprintf(stderr, "startIndex %d, stopIndex %d\n", startIndex, stopIndex);
}
