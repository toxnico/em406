#ifndef _EM406_h
#define _EM406_h

class EM406
{
  private:
  public:
    char _lastGGA[128];
    char _lastGSA[128];
    char _lastRMC[128];
    char _lastGSV[128];

    void dispatchGGA(char *buffer);
    void dispatchGSA(char *buffer);
    void dispatchRMC(char *buffer);
    void dispatchGSV(char *buffer);

    
    char longitude[15];
    char longitudeEW[2];
    char latitude[15];
    char latitudeNS[2];
    int satellitesUsed;
    int mslAltitude;
    char utcTime[15];

    char utcTimeFormatted[10];

    char speedOverGroundKnotsStr[15];
    float speedOverGroundKnots;
    float speedOverGroundKmh;
    float courseOverGroundDeg;

    char googleMapsUrl[128];

    /*
    String getGoogleMapsUrl();

*/
    /*
    String getUTCTimeFormatted(){ 
        String raw = getUTCTime();
        String out = raw.substring(0,2) + ":" + raw.substring(2,4) + ":" + raw.substring(4,6);
        return out;
    }

    String getLatitudeFormatted();
    String getLongitudeFormatted();

    String getSpeedOverGroundKnotsStr(){ return section(_lastRMC, ",", 7);}
    float getSpeedOverGroundKnots(){ return section(_lastRMC, ",", 7).toFloat();}
    float getSpeedOverGroundKmh(){ return getSpeedOverGroundKnots() * 1.852; }
float getCourseOverGroundDeg(){ return section(_lastRMC, ",", 8).toFloat();}
*/
};

#endif //_EM406_h