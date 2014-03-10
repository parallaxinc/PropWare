#ifndef LIBPROPELLER_GPSPARSER_H_
#define LIBPROPELLER_GPSPARSER_H_

#include "libpropeller/serial/serial.h"

/** Receives and parses NMEA0183 GPS streams into C strings.
 * 
 * Requires 1 cog to operate.
 * 
 * @warning The PGTOP sentences (if any) can't have any "$" characters in them. The parsing
 * would treat that as a new NMEA string. While technically not correct, it's a
 * bit easier to implement.
 * 
 * @author SRLM (srlm@srlmproductions.com)
 */
class GPSParser {
public:

    /** Create the parser and launch a new cog.
     * 
     * @param rx_pin the serial pin to receive data from the GPS.
     * @param tx_pin the pin to transmit data to the GPS. If not used, set to -1.
     * @param baud  the baud rate to use for tranmission and receiving.
     */
    bool Start(const int rx_pin, const int tx_pin, const int baud) {
        gps_serial_.Stop();
        next_character_position_ = 0;
        gps_serial_.Start(rx_pin, tx_pin, baud);

        recording_sentence_ = false;
        return true;
    }

    /** Stop the GPS parsing, and the cog that was started.
     */
    ~GPSParser() {
        gps_serial_.Stop();
    }

    /** Gets a NMEA string. Note that the returned string address is valid (will 
     * not be overwritten) until the next time GetStr() is called.
     * 
     * The returned string includes all characters from the GPS except for
     *  the \r and \n (<CR><LF>) at the end.
     * 
     * Partial sentences may be thrown away (if a sentence is not being 
     * currently recorded).
     * 
     * Ignores the PGTOP type sentence.
     * 
     * @returns NULL pointer if no string, null terminated string otherwise
     */
    char * Get() {
        return Get(internal_buffer_);
    }

    /** Same as @a Get(), but with the option of specifying a buffer to use 
     * instead of the internal buffer.
     * 
     * @warning If you want to switch between buffers, you must not switch 
     * until immediately  after gps.Get(s) != NULL (ie, right after it returns a 
     * string). Otherwise, part of the string will be stored in one buffer, and 
     * part of the string will be stored in the other.
     * 
     * @param string The buffer to use. Must be at least 85 characters long (the NMEA string length).
     * @param maxBytes The maximum number of bytes to record in this string. Defaults to maximum NMEA sentence length.
     * @returns NULL pointer if no string, null terminated string otherwise (in buffer @a s).
     */
    char * Get(char string[], const int maxBytes = kNmeaMaxLength) {
        for (;;) {

            int byte = gps_serial_.Get(0);
            if (byte == -1) return NULL;

            if (next_character_position_ == 6) {
                CheckForPGTOP(string);
            }


            if (recording_sentence_ == false && byte != kSentenceStartCharacter) {
                /* Do nothing */
            } else if (byte == '\r' || byte == '\n') {
                return TerminateString(string);
            } else {
                //Have a valid byte, now need to add to buffer
                recording_sentence_ = true;
                string[next_character_position_++] = byte;
            }

            if (next_character_position_ == maxBytes - 1) {
                return TerminateString(string);
            }
        }
    }

protected:
    Serial gps_serial_;

private:
    static const int kNmeaMaxLength = 85;
    static const int kBufferSize = kNmeaMaxLength;
    static const char kSentenceStartCharacter = '$';

    int next_character_position_;
    char internal_buffer_[kBufferSize]; //Holds 1 NMEA string
    bool recording_sentence_;

    char * TerminateString(char string[]) {
        string[next_character_position_] = 0; //Null terminator
        next_character_position_ = 0; //Reset nextCharacterPosition
        recording_sentence_ = false;
        return string; //Return pointer
    }

    void CheckForPGTOP(char string[]) {
        if (string[1] == 'P' &&
                string[2] == 'G' &&
                string[3] == 'T' &&
                string[4] == 'O' &&
                string[5] == 'P') {
            next_character_position_ = 0;
            recording_sentence_ = false;
        }
    }

    /** Get the underlying serial object.
     * @warning This function is for testing only!
     * @returns A pointer to the underlying serial object.
     */
    Serial * getSerial(void) {
        return &gps_serial_;
    }
public:
    friend class UnityTests;
};



#endif // LIBPROPELLER_GPSPARSER_H_
