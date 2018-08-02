#include "em406.h"
#include "../stringlib/dmstring.cpp"

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

#define CATCH_CONFIG_MAIN

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