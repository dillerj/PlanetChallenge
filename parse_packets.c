/*******************************************************************************
* File name   : packet_parser.cpp
* Authors     : Jed Diller
* Project     : Planet Code Challenge
* Copyright   : NA
* Created     : 6/20/2017
* 
* Description:
*   - The program should read from stdin and write to stdout in the formats described below
*   - Status or debugging messages may be reported on stderr. There are no restrictions on the format of the status/debug messages.
*   - Messages consist of a header and a /variable/ length payload. Any data that does not conform to the message format should be discarded.
*   - The payload is an array of bytes, and may contain any 8-bit byte (0-255). No escaping is performed.
*   - The program should terminate cleanly when EOF is reached.
*   - The program should handle inputs of (reasonably) large size. 
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
* Compilation:
*   $ gcc parse_packets.c -o runpp -Wall -Wextra
*
* Usage: 
*   Pipe file to program, read as stdin
*   $ ./runpp < 10_short_packets
*   $ ./runpp < 200_packets
*   $ ./runpp < 2_packets
*
* Todos:
*   - 
******************************************************************************/

/*****************************************************************************
* Includes
******************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
 
/*****************************************************************************
* Definitons
******************************************************************************/
#define MARKER_1 0x21
#define MARKER_2 0x22

#define BYTES_PER_PAYLOAD   256                     // encodable payload length: 0-255
#define PACKET_LINE_LEN     (4 * BYTES_PER_PAYLOAD) // assume malformed can be 4/3 size expected

/*****************************************************************************
* Functions
******************************************************************************/

/*****************************************************************************
* Main
******************************************************************************/
int main()
{
    uint8_t buffer[3];                      // [MARKER_1, MARKER_2, Packet Length]   
    uint8_t payload_len         = 0;        // payload len byte 
    uint8_t num_payload_bytes   = 0;        // assuming < 256 bytes in packet
    uint16_t num_packets        = 0;        // Keeping track of metadata while parsing 
    uint16_t num_malf_packets   = 0;        // Keeping track of metadata while parsing 
    
    char s_packet_line[PACKET_LINE_LEN];    // "{###} XX XX XX..." or not per packet
    char s_payload_len[4];                  // "###"
    char s_line_head[6];                    // "{###}"
    char s_payload_byte[4];                 // " XX"

    FILE *fptr = stdin;                     // Decide file source, STDIN for production

    /* 
     * Read file 1 byte at a time looking for MARKER 1
     */
    while (fread(buffer, sizeof(buffer[0]), 1, fptr) > 0)
    {
        if (buffer[0] == MARKER_1)
        {
            /* 
             * MARKER 1 found, back up file pointer and to read in both markers 
             * and the encoded length
             */
            fseek(fptr, -1, SEEK_CUR); // Back up 1 byte 
            
            if ( fread(buffer, sizeof(buffer), 1, fptr) > 0)
            {
                /* Check for presence of both MARKERS */ 
                if (buffer[0] == MARKER_1 && buffer[1] == MARKER_2)
                {
                    /* 
                     * Create constant length payload-length string
                     * Payload length is buffer[2], 0-255, and can only be 3 chars across
                     * E.g.: "{255}" or "{001}"
                     */ 
                    payload_len = buffer[2];
                    sprintf(s_payload_len, "%u", payload_len);        // int to str
                    sprintf(s_line_head, "{%3s}", s_payload_len);   // fixed width
                    strcat(s_packet_line, s_line_head);             // Append to packet line string 
                    
                    /*
                     * Read until MARKER_1 found in payload data 
                     * If just MARKER_1 found, keep reading bytes
                     * If MARKER_2 found, end of packet has been found
                     * Check number of bytes read, see if it matches amount expected (encoded)
                     */
                    num_payload_bytes = 0;
                    while ( (fread(buffer, sizeof(buffer[0]), 1, fptr) > 0) )
                    {
                        /* Check for MARKER_1 */ 
                        if (buffer[0] == MARKER_1)
                        {
                            /* Read next byte and check for MARKER 2 */
                            if (fread(&buffer[1], sizeof(buffer[1]), 1, fptr) > 0)
                            {
                                if (buffer[1] == MARKER_2)
                                {   
                                    /* 
                                     * New packet found! Rewind 2 bytes and break from this 
                                     * packet payload data reader  
                                     */
                                    fseek(fptr, -2, SEEK_CUR);
                                    break;
                                } else {
                                    /* 
                                     * Marker 2 not found, keep reading packet data 
                                     * back up pointer for next read
                                     */
                                    fseek(fptr, -1, SEEK_CUR);
                                }
                            } /* Failure to read caught by while loop */
                        } /* No MAKER 1, just another byte */ 
                        
                        /* Keep appending byte strings to packet line string */ 
                        sprintf(s_payload_byte, " %0.2X", buffer[0]);   // Make hex byte string 
                        strcat(s_packet_line, s_payload_byte);          // Append byte to packet string   
                        num_payload_bytes++;    
                    }
                    
                    /* 
                     * Check see if payload length makes sense, print full packet line 
                     * if it does, otherwise do not print and write to stderr 
                     */
                    if (num_payload_bytes == payload_len)
                    {
                        strcat(s_packet_line, "\n");            // end packet line str 
                        fprintf(stdout, "%s", s_packet_line);   // send to stdout
                    } else {
                        fprintf(stderr,"MALFORMED packet by length! encoded len: %u \tactual len: %u \tpacket number: %u\n", 
                                                                            payload_len, num_payload_bytes, num_packets);
                        num_malf_packets++;
                    }

                    /*
                     * Resets for next packet, reset strings
                     */
                    memset(s_packet_line, 0, PACKET_LINE_LEN);
                    memset(s_payload_len, 0, 4);
                    memset(s_line_head, 0, 6);
                    memset(s_payload_byte, 0, 4);
                
                    num_packets++;
                
                } /* Both markers not found, keep reading */
            } /* Failed to read 3 bytes for marker and len */ 
        } /* No marker 1 */
    } /* Not at EOF */ 

    // Sanity checks:    
    fprintf(stderr,"Number of packets found     : %i\n", num_packets);
    fprintf(stderr,"Number of malformed packets : %i\n", num_malf_packets);
    
    /* Clean up */ 
    fclose(fptr);
    
    return 0;
}


