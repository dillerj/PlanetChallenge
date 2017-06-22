/*******************************************************************************
* File name   : packet_parser.cpp
* Authors     : Jed Diller
* Project     : NA
* Copyright   : NA
* Created     : 6/18/2017
* 
* Description:
*   - The program should read from stdin and write to stdout in the formats described below
*   - Status or debugging messages may be reported on stderr. There are no restrictions on the format of the status/debug messages.
*   - Messages consist of a header and a /variable/ length payload. Any data that does not conform to the message format should be discarded.
*       - The payload is an array of bytes, and may contain any 8-bit byte (0-255). No escaping is performed.
*       - what would escaping be? 
*   - The program should terminate cleanly when EOF is reached.
*       - while not eof
*   - The program should handle inputs of (reasonably) large size. 
*       -What is reasonable?
*
*   Message Format
*   ===============
*   Offset  Value    Type     Meaning
*   0       0x21     (byte)   start marker 0
*   1       0x22     (byte)   start marker 1
*   2       (length) (uint8)  length of payload
*   3-258   (array)  (bytes)  payload*
*
*   Output format
*   =============
*   Report the length of each packet's payload as an integer enclosed in curly braces (right justified), 
*   and report the packet payload as series of hexadecimal numbers corresponding to each byte and separated 
*   by spaces. Separate each packet with a newline. An example of the expected output is show below:
*
*   {  3} 41 42 43
*   {  4} 64 65 66 67
*
*   Note that the only acceptable characters in the output stream are " 012345689{}ABCDEF" and newline ("\n").
*   The contents of packet headers and any invalid data should not be reported. Incomplete packets should not be reported.
*
* Noteworthy Examples:
*   - 
* Todos:
*   - 
******************************************************************************/

/*****************************************************************************
* Includes
******************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
 
/*****************************************************************************
* Definitons
******************************************************************************/
#define MARKER_1 0x21
#define MARKER_2 0x22

#define BYTES_PER_PAYLOAD   256 // encodable payload length 0-255
#define PACKET_LINE_LEN     (5 + 1 + (BYTES_PER_PAYLOAD+1)*2 + 1) // "{###} ## ## ...\n"

#define DEBUG_PRINT_DATA    0
#define DEBUG_PRINTS        0

/*****************************************************************************
* Private variables not included in header
******************************************************************************/

/*****************************************************************************
* Functions
******************************************************************************/

/*****************************************************************************
* Main
******************************************************************************/
int main( int argc, char *argv[] )
{
    unsigned char buffer[1];                    // Read in 1 byte at a time
    unsigned char payload_len           = 0;    // payload len byte 
    unsigned char num_payload_bytes     = 0;    // assuming < 256 bytes in packet
    unsigned char payload_byte          = 0;    // payload len byte 
    unsigned char temp_byte_1           = 0;    
    unsigned char temp_byte_2           = 0;    


    unsigned int num_packets    = 0;    // Keeping track of metadata while parsing 
    unsigned int byte_num       = 0;

    char s_packet_line[PACKET_LINE_LEN];    // "{###} XX XX XX..." or not per packet
    char s_payload_len[4];                  // "###"
    char s_line_head[6];                    // "{###}"
    char s_payload_byte[4];                 // " XX"

    // Decide file source, STDIN for production
    FILE *fptr = stdin;

    if (fptr == NULL)
    {
        fprintf(stderr, "Unable to open source file\n");
        return -1;
    }

    // REMOVE ME! 
    #if DEBUG_PRINT_DATA
        printf("Input byte stream:\n");
        char print_buff[1];
        int raw_num_bytes = 0;
        int raw_num_packets = 0;
        while(fread(print_buff, sizeof(print_buff), 1, fptr) == 1) 
        {
            printf("%0.2X", print_buff[0]);
            raw_num_bytes++;
            if (print_buff[0]==MARKER_1)
                if (fread(print_buff, sizeof(print_buff), 1, fptr) == 1)
                {    raw_num_bytes++;
                    if (print_buff[0]==MARKER_2)
                        raw_num_packets++;
                }

        }
        printf("\n");
        printf("Raw number of bytes     : %i\n", raw_num_bytes);
        printf("Raw number of packets   : %i\n\n", raw_num_packets);
        return 0;
    #endif

    /* 
     * Read file 1 byte at a time looking for MARKER 1
     * Because reads are done in packet until next MARKER 1, 
     * buffer[0] has already been read 
     */
    while( buffer[0]== MARKER_1 || (fread(buffer, sizeof(buffer), 1, fptr) == 1) )  
    {
        printf("%0.2X", buffer[0]);
        byte_num++;
        /* Look for MARKER_1 */ 
        if ( buffer[0] == MARKER_1 ) 
        {
            /* Look for MARKER_2 at next byte, after MARKER_1 */ 
            if (fread(buffer, sizeof(buffer), 1, fptr) == 1 )
            {
                printf("%0.2X", buffer[0]);
                byte_num++;
                if ( buffer[0] == MARKER_2 )
                {
                    /*
                     * Read payload length and build up packet line string. If the 
                     * packet is not mallformed, print the sting
                     */            
                    if (fread(&payload_len, sizeof(payload_len), 1, fptr) == 1)
                    {
                        printf("%0.2X", buffer[0]);
                        byte_num++;
                        /* 
                         * Create constant length payload length string
                         * Payload length is 0-255, can only be 3 chars across
                         */ 
                        sprintf(s_payload_len, "%u", payload_len);    // int to str
                        sprintf(s_line_head, "{%3s}", s_payload_len); // fixed width
                        
                        /* 
                         * Append to packet line string 
                         */
                        strcat(s_packet_line, s_line_head);
                        
                        /* 
                         * Read until MARKER_1 and MARKER_2 found
                         * If just MARKER_1 found, keep reading bytes
                         * Check number of bytes read, see if it matches amount expected (encoded)
                         */
                        num_payload_bytes = 0;
                        while ( (fread(buffer, sizeof(buffer[0]), 1, fptr) == 1) )
                        {
                            printf("%0.2X", buffer[0]);
                            byte_num++;

                            // Check for MARKER_1 and MARKER_2 
                            // If found, break, else keep printing
                            if (buffer[0] == MARKER_1)
                            {
                                temp_byte_1 = buffer[0]; 
                                if (fread(buffer, sizeof(buffer[0]), 1, fptr) == 1)
                                {
                                    if (buffer[0] == MARKER_2)
                                    {

                                    }
                                } else {
                                    break;
                                }
                                   

                            { 
                            }



                            sprintf(s_payload_byte, " %0.2X", buffer[0]); // Make hex byte string 
                            strcat(s_packet_line, s_payload_byte); // Append byte to packet string   
                            num_payload_bytes++;
                        }

                        /* 
                         * Check see if payload length makes sense
                         * Print payload if it does, otherwise do not and write to stderr 
                         */
                        if (num_payload_bytes == payload_len)
                        {
                            // In range, print packet string 
                            strcat(s_packet_line, "\n");
                            fprintf(stdout, "%s", s_packet_line);
                        } else {
                            // What resetting needed?
                            fprintf(stderr,"MALFORMED packet by length! encoded len: %u actual len: %u packet number: %u\n", 
                                                                        payload_len, num_payload_bytes, num_packets);

                        }

                        /*
                         * Resets for next packet, reset strings
                         */
                        memset(s_packet_line, 0, PACKET_LINE_LEN);
                        memset(s_payload_len, 0, 4);
                        memset(s_line_head, 0, 6);
                        memset(s_payload_byte, 0, 4);

                    } else {
                        break; // EOF while reading MARKER 1
                    }
                } else {
                    // Second BYTE not MARKER 2
                    // Malformed packet 
                    fprintf(stderr,"MALFORMED packet, missing MARKER_2 after MARKER_1\n");
                    continue;
                }   
                num_packets++;
            } else {
                break; // EOF while reading MARKER 2
            }
        } else {
            // Byte is not MARKER 1, keep reading 
            byte_num++;
        }
    }

    printf("Num packets found: %i\n", num_packets);
    // Clean up
    fclose(fptr);
    return 0;
}
















