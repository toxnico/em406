#include "em406.h"

#include <stdio.h>
#include <stdlib.h>
#include <driver/uart.h>
#include <string.h>
#include <esp_log.h>
#include "Arduino.h"

EM406::EM406(int rx, int tx)
{

    uart_config_t cfg;

    cfg.baud_rate = 4800;
    cfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    cfg.parity = UART_PARITY_DISABLE;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.data_bits = UART_DATA_7_BITS;

    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &cfg));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, 1024, 1024, 0, NULL, 0));

    strcpy(_buffer, "tac");
    strcpy(latitude, "undef");
    strcpy(longitude, "undef");
    strcpy(latitudeNS, "X");
    strcpy(longitudeEW, "X");
    strcpy(utcTime, "XX:XX:XX");
}

bool EM406::update()
{
    size_t availableLength;
    uart_get_buffered_data_len(UART_NUM_2, &availableLength);

    if (!availableLength)
        return false;

    uint8_t c = 0;
    uart_read_bytes(UART_NUM_2, &c, 1, 100 / portTICK_PERIOD_MS);
    //printf("%c", c);

//return true;
    //Serial.write(c);
    if (c == 13)
        return true;

    if (c == 10)
    {
        _processBuffer(_buffer);
        
        strcpy(_lastBuffer, _buffer);
        strcpy(_buffer, "");

        
        return true;
    }

    char ch[2];
    sprintf(ch, "%c", c);

    strcat(_buffer, ch);

    return true;
}

void EM406::dispatchGGA(char *gga)
{
    
    section(gga, ",", 4, longitude);
    section(gga, ",", 5, longitudeEW);
    section(gga, ",", 2, latitude);
    section(gga, ",", 3, latitudeNS);
    section(gga, ",", 1, utcTime);

    char buff[10];
    section(gga, ",", 7, buff);
    satellitesUsed = atoi(buff);

    section(gga, ",", 9, buff);
    mslAltitude = atoi(buff);

    //utc formatted:
    char h[3];
    char m[3];
    char s[3];
    substring(utcTime, 0, 2, h);
    substring(utcTime, 2, 2, m);
    substring(utcTime, 4, 2, s);
    sprintf(utcTimeFormatted, "%s:%s:%s", h, m, s);

    //google maps url

    int latDeg = substringInt(latitude, 0, 2);
    if (equals(latitudeNS, "S"))
        latDeg *= -1;

    char latMin[10];
    substring(latitude, 2, 7, latMin);

    int lonDeg = substringInt(longitude, 0, 3);
    if (equals(longitudeEW, "W"))
        lonDeg *= -1;

    char lonMin[10];
    substring(longitude, 3, 7, lonMin);

    sprintf(googleMapsUrl, "https://www.google.fr/maps/place/%d%s%s,%d%s%s", latDeg, "%20", latMin, lonDeg, "%20", lonMin);
}
void EM406::dispatchGSA(char *buffer)
{
}
void EM406::dispatchRMC(char *rmc)
{
    strcpy(_lastRMC, rmc);
    section(rmc, ",", 7, speedOverGroundKnotsStr);
    speedOverGroundKnots = atof(speedOverGroundKnotsStr);
    speedOverGroundKmh = speedOverGroundKnots * 1.852;

    char buff[15];
    section(rmc, ",", 8, buff);
    courseOverGroundDeg = atof(buff);
}
void EM406::dispatchGSV(char *buffer)
{
}

/**
 * Dispatches the last message into the right buffer variable
 */
bool EM406::_processBuffer(char *buffer)
{
    //Invalid message from EM406 ?
    if (!_isMessageValid(buffer))
    {
        //debug("Invalid buffer");
        return false;
    }

    if (startsWith(_buffer, "$GPGGA"))
    {
        strcpy(_lastGGA, _buffer);
        dispatchGGA(_buffer);
        //strcpy(_lastGGA, _buffer);

        //ESP_LOGI("processBuffer", "GGA : %s", _lastGGA);
        char fixed[3];
        section(_lastGGA, ",", 6, fixed);
        _isFixed = equals(fixed, "1");
        //ESP_LOGI("processBuffer", "Fixed : %s", fixed);
        return true;
    }

    if (startsWith(_buffer, "$GPGSA"))
    {
        strcpy(_lastGSA, _buffer);
        return true;
    }

    if (startsWith(_buffer, "$GPRMC"))
    {
        dispatchRMC(_buffer);
        // strcpy(_lastRMC, _buffer);
        return true;
    }

    if (startsWith(_buffer, "$GPGSV"))
    {
        strcpy(_lastGSV, _buffer);
        return true;
    }

    return false;
}

/**
 * Check the raw message's checksum
 * Returns true if the message s valid.
 */
bool EM406::_isMessageValid(char *str)
{
    if (!str)
        return false;
    if (equals(str, ""))
        return false;

    char withoutDollar[128];
    substring(str, 1, strlen(str), withoutDollar); //remove the first $
    int star = indexOf(withoutDollar, "*");        //index of *
    if (star == -1)
        return false;

    char givenChecksum[4];
    substring(withoutDollar, star + 1, strlen(withoutDollar), givenChecksum);

    //ESP_LOGI("isMessageValid", "----------");
    //ESP_LOGI("isMessageValid", "str = %s", str);
    //ESP_LOGI("isMessageValid", "s = %s", s);
    //ESP_LOGI("isMessageValid", "star = %d", star);

    //ESP_LOGI("isMessageValid", "givenChecksum = %s", givenChecksum);
    char messageBetween[120];
    substring(withoutDollar, 0, star, messageBetween);
    //ESP_LOGI("isMessageValid", "messageBetween = %s", messageBetween);

    char *myChecksum = _computeChecksum(messageBetween);
    //ESP_LOGI("isMessageValid", "myChecksum = %s", myChecksum);
    bool res = equals(myChecksum, givenChecksum);
    free(myChecksum);
    return res;
}

/**
 * Compute a message checksum
 */
char *EM406::_computeChecksum(char *s)
{
    //while (true)
    //{
    //ESP_LOGI("_computeChecksum", "s = %s", s);
    int out = 0;

    for (int i = 0; i < strlen(s); i++)
    {
        out = out ^ s[i];
    }

    //ESP_LOGI("_computeChecksum", "out = %X", out);

    char *hex = (char *)malloc(10);
    sprintf(hex, "%X", out);
    //ESP_LOGI("_computeChecksum", "hex = %s", hex);
    //continue;
    //}
    return hex;
}

//#define CATCH_CONFIG_MAIN

#ifdef CATCH_CONFIG_MAIN

#include "catch.hpp"
#include <string.h>

TEST_CASE("gga", "[gga]")
{
    EM406 gps;
    strcpy(gps._lastGGA, "$GPGGA,161229.487,3723.2475,N,12158.3416,W,1,07,1.0,9.0,M,,,,0000*18");
    gps.dispatchGGA(gps._lastGGA);

    REQUIRE(equals(gps.longitude, "12158.3416"));
    REQUIRE(equals(gps.longitudeEW, "W"));
    REQUIRE(equals(gps.latitude, "3723.2475"));
    REQUIRE(equals(gps.latitudeNS, "N"));
    REQUIRE(equals(gps.utcTime, "161229.487"));
    REQUIRE(equals(gps.utcTimeFormatted, "16:12:29"));
    REQUIRE(gps.mslAltitude == 9);
    REQUIRE(gps.satellitesUsed == 7);

    strcpy(gps._lastRMC, "$GPRMC,161229.487,A,3723.2475,N,12158.3416,W,0.13,309.62,120598,,*10");
    gps.dispatchRMC(gps._lastRMC);

    REQUIRE(gps.speedOverGroundKnots == 0.13f);
    REQUIRE(gps.speedOverGroundKmh == 0.24076f);


    REQUIRE(equals(gps.googleMapsUrl, "https://www.google.fr/maps/place/37%2023.2475,-121%2058.3416"));
}

#endif