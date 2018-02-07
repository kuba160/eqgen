// The second approach to eqgen


// Structure for resposiveness
typedef struct freqresp{
	float frequency;
	float responsiveness;
	float difference;
};

typedef struct equalizer{
	float frequency;
	float value;
};

typedef struct foobar_eq{
	const float freq[18] = {55.0,77.0,110.0,156.0,220.0,311.0,440.0,622.0,880.0,1200.0,1800.0,2500.0,3500.0,5000.0,7000.0,10000.0,14000.0,20000.0};
	unsigned char value[18];
};

typedef struct deadbeef_eq{
	const float freq[18] = {55.0,77.0,110.0,156.0,220.0,311.0,440.0,622.0,880.0,1200.0,1800.0,2500.0,3500.0,5000.0,7000.0,10000.0,14000.0,20000.0};
	float value[18];
};

// Calculate best value to equalize for frequency
// Add all values which are between these 2 values and divide them by sum
// You will get avergave for specific frequency
typedef struct range{
	const float min = {20.0,66.0,98.0,137.5,192.5,270.0,375.5,531.0,751.0,1040.0,1500.0,2150.0,3000.0,4250.0,6000.0,8500.0,12000.0,17000.0};
	const float max = {66.0,98.0,137.5,192.5,270.0,375.5,531.0,751.0,1040.0,1500.0,2150.0,3000.0,4250.0,6000.0,8500.0,12000.0,17000.0,20000.0};
};

#include <stdio.h>
#include <libarg.h>

#define FREQ_COUNT_MAX 1024

int main (int argc, char * argv[]) {
	LibARG();
	char * filename = FreeMandatoryStringCase("Filename", "FILE");
	int make_foobar_eq = SelectCase('f', "--foobar", "Generate foobar eq");
	int make_deadbeef_eq = SelectCase('d', "--deadbeef", "Generate deadbeef eq");
	LibARG();

	FILE * pFile;
	pFile = fopen(filename,"r");
	if(!pFile){
		printf ("Error while opening file. %s\n",ferror(pFile));
		exit (1);
	}

	// File loaded, read values of it

	struct freqresp freq_table[FREQ_COUNT_MAX];
	int err = ReadValues(pFile,&freq_table);
	if (err) {
		printf ("Error while reading values from file.\n");
		exit (1);
	}
	fclose (pFile);

	// We have values in freq_table[], first normalize them and then make an equalizer

	Normalize (freq_table);

	struct foobar_eq foobareq = GenFoobarEQ (freq_table);

	printf("Foobar eq data:\n");

	if(make_foobar_eq){
		char * outfilename = MakeFileName(fiename,"feq");
		FILE * pFileOut;
		pFileOut = fopen(outfilename,"w");
		if(!pFileOut){
			printf ("Error while opening output file: %s\n",outfilename);
			exit (1);
		}
		for (i = 0; i < 18; i++){
			printf("%.0fHz: %d\n",foobareq.freq[i],foobareq.value[i]);
			fprintf(pFileOut,"%d\n",foobareq.value[i]);
		}
		fclose(pFileOut);
	}
}

int ReadValues (FILE * File, freqresp & freqtable) {

	char buffer[1024];
	int n=0,i=0;
 	while (fgets(buffer, sizeof(buffer), File)) {
 		if(!strncmp(buffer,";;",2))
 			break;
		char *pch = strtok (buffer,";");
		int j;
		if (pch != NULL){
			for(j=0;pch[j] != '\0';j++){
			if (pch[j] == ',')
				pch[j] = '.';
			}
			float temp = strtof(pch,NULL);
			if(temp == 0.0)
				continue;
			freqtable[n].frequency = temp;
		}
		pch = strtok (NULL, ";");
		if (pch != NULL){
			for(j=0;pch[j] != '\0';j++){
			if (pch[j] == ',')
				pch[j] = '.';
			}
			float temp = strtof(pch,NULL);
			if(temp == 0.0)
				continue;
			freqtable[n].responsiveness = temp;
		}
		n++;
		if (n > FREQ_COUNT_MAX) {
			printf ("Value count overflow!\n");
			freqtable[n] = (freqresp) {0,0};
			return -1;
		}
    }
    freqtable[n] = (freqresp) {0,0};

    return 0;
}

char * MakeFileName(char * inputfile, char * ending) {
	char * outfilename = malloc(strlen(inputfile)+strlen(ending)+1);
	strcpy(outfilename,inputfile);
	char * dotpointer = strrchr(outfilename,'.');
	if(!dotpointer){
		dotpointer = strrchr(outfilename,'\0');
		if(ending[0] == '.')
			strcpy(dotpointer,ending);
		else {
			dotpointer[0] = '.';
			strcpy(dotpointer+1,ending);
		}
	}
	else {
		if(ending[0] == '.')
			strcpy(dotpointer,ending);
		else
			strcpy(dotpointer+1,ending);
	}

	return outfilename;
}

int Normalize (freqresp & values) {
	// Move to zero range
	//#define USE_AVERAGE
	#define USE_MEDIAN

	float diffvalue = 0;
	#ifdef USE_AVERAGE
	float average = 0;
	for (i = 0; i < n; i++){
		average += values[i].responsiveness;
	}
	diffvalue = average/n;
	#elif defined USE_MEDIAN
	float median_min = 100000, median_max = -100000;
	for (i = 0; i < n; i++){
		if(values[i].responsiveness == 0.0)
			continue;
		if (median_min > values[i].responsiveness)
			median_min = values[i].responsiveness;
		if (median_max < values[i].responsiveness)
			median_max = values[i].responsiveness;
	}
	diffvalue = (median_min+median_max)/2;
	#endif

	printf("Calculated volume: %.2fdB\n",diffvalue);
	for (i = 0; i < n; i++){
		values[i].difference = values[i].responsiveness - diffvalue;
		//printf("%f %f\n",values[i].frequency,values[i].responsiveness);
	}

	return 0;
}


struct foobar_eq GenFoobarEQ () {
	struct foobar_eq equalizer;
	// Convert 
	for (i = 0; i < 18; i++){
		int low_num=0;
		while (values[low_num].frequency < equalizer.freq[i])
			low_num++;
		if (low_num == 0){
			printf("Not enough data to equalize %.0fHz\n",equalizer.freq[i]);
			equalizer.value[i] = 0;
			continue;
		}

		freqresp lower_freq = values[low_num-1];
		freqresp higher_freq =values[low_num];
		//printf("Lowest value for %f is %f (%f)\n",foobar_freq[i],lower_freq.frequency,lower_freq.responsiveness);
		//printf("Highest value is for %f is %f (%f)\n",foobar_freq[i],higher_freq.frequency,higher_freq.responsiveness);
		// calculate slope (a) delta y divided by delta x
		{
			float slope = (higher_freq.responsiveness-lower_freq.responsiveness)/(higher_freq.frequency-lower_freq.frequency);
			//printf("calculated slope: %f\n",slope);
			float freq_diff = equalizer.freq[i]-lower_freq.frequency;
			float freq_diff_responsiveness = lower_freq.responsiveness + freq_diff*slope;
			//printf("calculated responsiveness: %f\n",freq_diff_responsiveness);
			int rounding = round(freq_diff_responsiveness);
			//printf("rounded: %d\n",rounding);
			equalizer.value[i] = 0-rounding;
		}
	}

	return equalizer;
}