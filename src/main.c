/*
	MIDI 2 WS MML conversion
	win32 application
	beatsgo (c) 2016
	note: the intent of this tool is to generate clean WonderSwan MML files
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <math.h>

//global struct for defining MML parameters
struct param
{
	char *songName;
	unsigned int format, trackRes, numTracks, beatsPerMin = 0, timeSigNum = 0, timeSigDem = 0, ticksPerQuart = 0;
};

char (*remove_ext(char* mystr, char dot, char sep))
{
	char *retstr, *lastdot, *lastsep;
	
	if(mystr == NULL)
		return NULL;
	if((retstr = malloc (strlen(mystr) + 1 )) == NULL)
		return NULL;
	
	strcpy (retstr, mystr);
	lastdot = strrchr(retstr, dot);
	lastsep = (sep == 0) ? NULL : strchr (retstr, sep);
	
	if (lastdot != NULL) {
        // and it's before the extenstion separator.

        if (lastsep != NULL) {
            if (lastsep < lastdot) {
                // then remove it.

                *lastdot = '\0';
            }
        } else {
            // Has extension separator with no path separator.

            *lastdot = '\0';
        }
    }

    // Return the modified string.

    return retstr;
}

//TODO: Parsing Data function
char *parsingData(unsigned char *dumpPtr, struct param *songVar)
{
	unsigned char *endTrkPtr = dumpPtr, *tempPtr;
	
	 
	/*
		
		tempPtr = &*dumpPtr;		
		while(tempPtr[0] != 0xFF || tempPtr[1] != 0x51 || tempPtr[2] != 0x03)
			tempPtr++;
	
	
	*/
	
	while(endTrkPtr[0] != 0xFF || endTrkPtr[1] != 0x2F || endTrkPtr[2] != 0x00)//endTrkPtr != &dumpPtr[*ptrLength])
		endTrkPtr++;
	
	
	while(dumpPtr != endTrkPtr)
	{
		dumpPtr++;
		if(*dumpPtr == 0xFF)
		{
			dumpPtr++;
			if(dumpPtr[0] == 0x58 && dumpPtr[1] == 0x04)
			{
				songVar->timeSigNum = dumpPtr[2];
				songVar->timeSigDem = dumpPtr[3];
				songVar->ticksPerQuart = dumpPtr[4];
			}
			else if(dumpPtr[0] == 0x51 && dumpPtr[1] == 0x03)
			{
				songVar->beatsPerMin = (unsigned int)((songVar->timeSigNum/songVar->timeSigDem) * (60000000/(float)((tempPtr[2] << 16)+(tempPtr[3] << 8)+tempPtr[4])));
			}
			
		}
		
	}
	if(songVar->beatsPerMin == 0)
	{
		printf("ERROR! Tempo not initiailized\nPress ENTER to exit");
	}
	dumpPtr += 3;	
	return &*dumpPtr;
}

int main(int argc, char *argv[])
{
	//struct mmlParam
	struct param mmlParam;
	
	//pointer variables
	unsigned int fileLength;
	unsigned char buffer[]="\0";
	unsigned char *buffPtr, *curPtr, *endPtr, *midiDump, *mmlOutput, *hexDumpOutput = "midi_hexdump.txt";
	FILE *inputPtr, *outputPtr;
	
	/*
		Step 1 | Read and verify MIDI file
	*/
	printf("Step 1: read and verify %s is a MIDI file\n", argv[1]);
	//Read file and check if it exist/valid 
	inputPtr = fopen(argv[1],"r");
	if(inputPtr == NULL)
	{
		printf("ERROR! NOT A VALID FILE\nPress ENTER to exit");
		fflush(stdout);
		getchar();
		exit(EXIT_FAILURE);
	}
	printf("...\n");
	
	//read file
	fseek(inputPtr, 0, SEEK_END); //goes to end of file
	fileLength = ftell(inputPtr); //"tells" where the pointer is at (at the end of the file)
	rewind(inputPtr); //goes back to the beginning of the file
	midiDump = (char *)malloc(sizeof(unsigned char)* (fileLength+1)); //allocate in memory size of the file + 1
	fread(midiDump, fileLength, 1, inputPtr);
	fclose(inputPtr);
	free(inputPtr);
	printf("...\n");
	
	//write hex dump to text file for logging purposes
	outputPtr = fopen(hexDumpOutput, "w+b"); //create/update save file in binary mode
	for(unsigned int idx = 0; idx < fileLength; idx++)
	{
		fwrite(&midiDump[idx], 1, 1, outputPtr);
	}
	fclose(outputPtr);
	free(outputPtr);
	printf("...\n\n");
	
	
	/*//==================
		Read Header data
	*///==================
	
	//I think this syntax is incorrect
	curPtr = &*midiDump;
	if(curPtr[0] != 0x4D || curPtr[1] != 0x54 || curPtr[2] != 0x68 || curPtr[3] != 0x64 ||
	curPtr[4] != 0x00 || curPtr[5] != 0x00 || curPtr[6] != 0x00 || curPtr[7] != 0x06 )
	{
		printf("ERROR! Not a MIDI file!\nPress ENTER to exit");
		fflush(stdout);
		getchar();
		free(midiDump);
		exit(EXIT_FAILURE);
	}	
	
	//Copy filename to local array, remove extension
	strcpy(buffer, argv[1]);
	buffPtr = remove_ext(buffer, '.', 0);
	memcpy(&mmlParam.songName, buffPtr, strlen(buffPtr));
	printf("Song Name: %s\n", &mmlParam.songName);
	
	//Read format
	mmlParam.format = (unsigned int)(curPtr[9]);
	switch(mmlParam.format)
	{
		case 0:
			printf("Format: Single Track\n");
			break;
		case 1:
			printf("Format: Multi-Track, Synchronous\n");
			break;
		case 2:
			printf("Format: Multi-Track, Asynchronous\n");
			break;
		default:
			printf("ERROR! Not a MIDI file!\nPress ENTER to exit");
			fflush(stdout);
			getchar();
			free(midiDump);
			exit(EXIT_FAILURE);
			break;
	}
	
	//Read number of tracks
	mmlParam.numTracks = (unsigned int)(curPtr[11]);
	printf("Number of Tracks: %u\n", mmlParam.numTracks);
	
	//Read track resolution
	mmlParam.trackRes = (unsigned int)(curPtr[13]);
	printf("Track Resolution: %u\n\n\n", mmlParam.trackRes);
	
	/*
		TODO: Step 2 | Parse Data For Format 0,1,201
	*/
	
	printf("Step 2: Parse Data\n");
	
	
	//Move pointer to MTrk entry 
	endPtr = &curPtr[fileLength];
	
	printf("Current position of endPtr: %p\n", &*endPtr);
	getchar();
	/*
		Format 1
	*/
	
	while(curPtr != endPtr)
	{
		//detect "MTrk"
		if(curPtr[0] == 0x4D && curPtr[1] == 0x54 && curPtr[2] == 0x72 && curPtr[3] == 0x6B)
		{
			printf("MTrk detected!\n");
			curPtr += 4;
			curPtr = parsingData(curPtr, &mmlParam);
		}
		else
			curPtr++;
	}
	
	printf("Initial values of the following values\n");
	printf("Time Signature: %u/%u\n", mmlParam.timeSigNum, mmlParam.timeSigDem);
	printf("Ticks Per Quarter Note: \%u\n", mmlParam.ticksPerQuart);
	printf("BPM: %u\n", mmlParam.beatsPerMin);
	getchar();
	/*
		TODO: Step 3 | Write Parsed Data to MML file
	*/
	
	
	printf("DONE! Press ENTER to exit");
	//fflush(stdout);
	getchar();
	free(midiDump);
	
	return 0;
	
}