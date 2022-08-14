#include <stdio.h>
#include <err.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>

uint8_t readUChar(uint8_t* dataBuffer, int pointer) {
        //read 1 byte from the buffer with starting point "pointer"
        return (uint8_t)(dataBuffer[pointer]);
}
uint16_t readUShort(uint8_t* dataBuffer, int pointer) {
        //read 2 bytes from the buffer with starting point "pointer"
        return (uint16_t)(dataBuffer[pointer] | dataBuffer[pointer + 1] << 8);
}
uint32_t readUInt(uint8_t* dataBuffer, int pointer) {
        //read 4 bytes from the buffer with starting point "pointer"
        return (uint32_t)(dataBuffer[pointer] | dataBuffer[pointer + 1] << 8 | dataBuffer[pointer + 2] << 16 | dataBuffer[pointer + 3] << 24);
}
void printString(uint8_t* dataBuffer, int pointer) {
        //print the string from starting point "pointer" until it reaches the terminating byte (0x00)
        for (int i = pointer; dataBuffer[i] != 0x00; i++) {
                fprintf(stderr, "%c", dataBuffer[i]);
        }
}
void printStateSlots(uint16_t num) {
        //print state slots
        fprintf(stderr, "slots: ");
        for (int bit = 0; bit < (int)sizeof(num) * 8; bit++) {
                //compare the bits of the number and then shift it
                int signal = num & 0x01 ? 1 : 0;
                num = num >> 1;
                if (signal == 1) {
                        fprintf(stderr, "%i[x] ", bit + 1);
                }
                else {
                        fprintf(stderr, "%i[] ", bit + 1);
                }
        }
}
void writeFloat(uint8_t* dataBuffer, int pointer, float value) {
        //type-pun float to int and then write it as an int value in the buffer
        uint32_t* pointerInt = (uint32_t*)&value;
        uint32_t valueInt = *pointerInt;
        dataBuffer[pointer++] = (uint8_t)((valueInt >> 0) & 0xff);
        dataBuffer[pointer++] = (uint8_t)((valueInt >> 8) & 0xff);
        dataBuffer[pointer++] = (uint8_t)((valueInt >> 16) & 0xff);
        dataBuffer[pointer++] = (uint8_t)((valueInt >> 24) & 0xff);
}
void readSlot(uint8_t* databuffer) {
        int pointer = 2; //start from 2nd bytes
        unsigned char slotID = readUChar(databuffer, pointer); pointer++;
        fprintf(stderr, "<slot text> slot %i: ", slotID + 1);
        printString(databuffer, pointer);
}
void readState(uint8_t* dataBuffer) {
        int pointer = 2; //start from 2nd bytes
        unsigned short uShort = readUShort(dataBuffer, pointer); pointer += 2; //move the pointer with 2 positions after reading 2 bytes
        unsigned int tempInt = readUInt(dataBuffer, pointer); pointer += 4;    //move the pointer with 4 positions after reading 2 bytes
        float tempFloat = (float)tempInt / 100.0f - 273.15f;
        fprintf(stderr, "<state> temp: %.2f°C, ", tempFloat);
        printStateSlots(uShort);
}
void writeRecord(int fd, uint8_t* dataBuffer, int size) {
        //write the buffer to the file
        if (write(fd, dataBuffer, size) != size) {
                close(fd);
                err(3, "couldn't write in file\n");
                exit(3);
        }
}

void record(char* file) {
        //open file and validate
        int fd = open(file, O_WRONLY | O_TRUNC);
        if (fd == -1) {
                err(4, "couldn't open file\n");
                exit(4);                                        
        }      

        float time_used;                                        //float for converting the time spent after receiving the first message
        int pointer = 0;                                        //pointer for the current position of bytes writen in the buffer
        int msgSize = 0;                                        //size of the incoming message in bytes
        unsigned char currBuf = 0;                              //byte read from stdin
        unsigned char* dataBuffer = 0;                          //buffer for the bytes of the message
        struct timespec startTime, endTime;                     //calculate time after starting the message

        clock_gettime(CLOCK_MONOTONIC, &startTime);             //get the time of the first message

        while (read(0, &currBuf, sizeof(uint8_t)) > 0) {  //read bytes until the user has stoped the input from curl
                //validate if the starting point of the buffer is 0
                if (pointer == 0) {
                        clock_gettime(CLOCK_MONOTONIC, &endTime); //get the time after reading a message
                        //calculate time difference
                        time_used = (float)(endTime.tv_sec - startTime.tv_sec) * 1000.0f + (float)(endTime.tv_nsec - startTime.tv_nsec) / 1000000.0f;
                        time_used /= 1000;
                        free(dataBuffer); //free the dataBuffer
                        //check which type of message is received and save how much size it will need
                        if (currBuf == 0x01) {
                                msgSize = 8;
                        }
                        else if (currBuf == 0x02) {
                                msgSize = 16;
                        }
                        //allocate memory depending which type of message is received
                        dataBuffer = (uint8_t*)malloc(msgSize * sizeof(uint8_t) + sizeof(float));
                }
                dataBuffer[pointer++] = currBuf;  //write the current byte to the buffer
                //check if the buffer is full of the needed bytes
                if (pointer == msgSize) {
                        fprintf(stderr, "[%.3f] ", time_used);
                        //send the buffer to be read depending by the content of it
                        if (msgSize == 8 * sizeof(uint8_t)) {
                                readState(dataBuffer);
                        }
                        else if (msgSize == 16 * sizeof(uint8_t)) {
                                readSlot(dataBuffer);
                        }
                        pointer = 0; //start writing bytes from 0 in the buffer
                        fprintf(stderr, "\n");
                        writeFloat(dataBuffer, msgSize, time_used); //write in the buffer the time of the message
                        writeRecord(fd, dataBuffer, msgSize + sizeof(float)); //write the buffer in the file
                }
        }
        free(dataBuffer);
        close(fd); //close file
}
void replay(char* file) {
        //open file and validate
        int fd = open(file, O_RDONLY);
        if (fd == -1) {
                err(4, "couldn't open file\n");
                exit(4);
        }

        float previousTime = 0; //float for saving the previous time of previous message
        int read_size;                  //int for comparing the size of read bytes is the same and the size of written bytes
        int msgSize = 0;                //the size of the incoming message in bytes
        unsigned char currBuf = 0;              //byte read from stdin
        unsigned char* dataBuffer = 0;  //buffer for the bytes of the message
        unsigned char* timeBuffer = 0;  //buffer for reading the bytes for the time

        while (read(fd, &currBuf, sizeof(unsigned char)) > 0) {
                //check which type of message is received and save how much size it will need
                if (currBuf == 0x01) {
                        msgSize = 8;
                }
                else if (currBuf == 0x02) {
                        msgSize = 16;
                }
                free(dataBuffer); //free the dataBuffer
                //allocate memory depending which type of message is received
                dataBuffer = (uint8_t*)malloc(msgSize * sizeof(uint8_t));
                lseek(fd, -1, SEEK_CUR);        //move the cursor of the file one position back so it can save and the starting byte
                //read 8 or 16 bytes depending on the message
                if ((read_size = read(fd, dataBuffer, sizeof(uint8_t) * msgSize)) > 0) {
                        //write the dataBuffer to stdout and check for errors
                        if (write(1, dataBuffer, read_size) != read_size) {
                                close(fd);
                                err(5, "Error while writing");
                                exit(5);
                        }
                        free(timeBuffer); //free the time buffer
                        timeBuffer = (uint8_t*)malloc(4 * sizeof(uint8_t)); //allocate memory for the time buffer
                        //read 4 bytes for the float
                        if ((read(fd, timeBuffer, sizeof(uint8_t) * sizeof(uint32_t))) > 0) {
                                //read the bytes
                                unsigned int timeInt = readUInt(timeBuffer, 0);
                                //type-pun from Int to Float
                                float* pointerFloat = (float*)&timeInt;
                                float time = *pointerFloat;
                                //slow the print of message so it can match the speed which was sent
                                usleep((time - previousTime) * 1000000);
                                previousTime = time;
                                fprintf(stderr, "[%.3f] ", time);
                        }
                        //send the buffer to be read depending by the content of it
                        if (msgSize == 8) {
                                readState(dataBuffer);
                        }
                        else if (msgSize == 16) {
                                readSlot(dataBuffer);
                        }
                        fprintf(stderr, "\n");
                }
        }
        free(timeBuffer);
        free(dataBuffer);
        close(fd); //close file
}

int main(int argc, char** argv) {
        //input validation
        if (argc != 3) {
                errx(1, "Invalid number of arguments");
                exit(1);
        }
        if (strcmp(argv[1], "record") == 0) {
                record(argv[2]);
        }
        else if (strcmp(argv[1], "replay") == 0) {
                replay(argv[2]);
        }
        else {
                errx(2, "Invalid first argument!");
                exit(2);
        }
        exit(0);
}
