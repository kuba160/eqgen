#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

typedef struct freqresp{
	float frequency;
	float responsiveness;
} freqresp;

int compare (const void * a, const void * b)
{

  freqresp *A = (freqresp *)a;
  freqresp *B = (freqresp *)b;

  return ( A->frequency - B->frequency );
}

int
main (int argc, char * argv[]){
	#define VALUES_MAX 1000
	struct freqresp values[VALUES_MAX];
	
	FILE * pFile;
	char buffer[1024];
	pFile = fopen("Bose.csv","r");
	if(!pFile){
		printf ("error opening file\n");
		exit (1);
	}
	int n=0,i=0;
 	while (fgets(buffer, sizeof(buffer), pFile)) {
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
			values[n].frequency = temp;
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
			values[n].responsiveness = temp;
		}
		n++;
    }
    values[n] = (freqresp) {0,0};
	/*while(!feof(pFile)){
		pch = strtok (str," ,.-");
		while (pch != NULL)
		{
			printf ("%s\n",pch);
		    pch = strtok (NULL, " ,.-");
		}
		int err = fscanf (pFile, "%f;%f", &(values[n].frequency), &(values[n].responsiveness));
		printf("fscanf returned %d\n",err);
		n++;
		if(n >= VALUES_MAX){
			printf("values overflow");
			exit (1);
		}
	}*/
	fclose(pFile);
	/*
	printf ("FREQ RESP\n");
	int n = 0, i = 0;
	for(;;){
		int err = scanf ("%d %d",&(values[n].frequency),&(values[n].responsiveness));
		if (err != 2){
			printf ("invalid values, retry?\n");
			if ( getchar() == 'y')
				continue;
			else
				break;
		}
		n++;
	}
	*/
	qsort (values, n, sizeof(freqresp), compare);
	printf("Read %d values\n",n);
	/*
	for (i = 0; i < n; i++){
		printf("%.02f %.02f\n",values[i].frequency,values[i].responsiveness);
	}
	*/
	float diffvalue = 0;
	// Move to zero range
	//#define USE_AVERAGE
	#define USE_MEDIAN
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
		values[i].responsiveness -= diffvalue;
		//printf("%f %f\n",values[i].frequency,values[i].responsiveness);
	}

	const float foobar_freq[18] = {55.0,77.0,110.0,156.0,220.0,311.0,440.0,622.0,880.0,1200.0,1800.0,2500.0,3500.0,5000.0,7000.0,10000.0,14000.0,20000.0};
	int foobar_eq[18];
	// Convert 
	for (i = 0; i < 18; i++){
		int low_num=0;
		while (values[low_num].frequency < foobar_freq[i])
			low_num++;
		if (low_num == 0){
			printf("Not enough data to equalize %.0fHz\n",foobar_freq[i]);
			foobar_eq[i] = 0;
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
			float freq_diff = foobar_freq[i]-lower_freq.frequency;
			float freq_diff_responsiveness = lower_freq.responsiveness + freq_diff*slope;
			//printf("calculated responsiveness: %f\n",freq_diff_responsiveness);
			int rounding = round(freq_diff_responsiveness);
			//printf("rounded: %d\n",rounding);
			foobar_eq[i] = 0-rounding;
		}
	}

	printf("Foobar eq data:\n");
	FILE * pFileOut;
	pFileOut = fopen("test.feq","w");
	if(!pFileOut){
		printf ("error opening file\n");
		exit (1);
	}
	for (i = 0; i < 18; i++){
		printf("%.0fHz: %d\n",foobar_freq[i],foobar_eq[i]);
		fprintf(pFileOut,"%d\n",foobar_eq[i]);
	}
	fclose(pFileOut);
	return 0;
}