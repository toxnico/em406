#ifndef _EM406_h
#define _EM406_h

#include <dmstring.h>

class EM406
{
private:
  bool _isFixed = false;

  bool _isValid = false;
  bool _processBuffer(char *buffer);

  char *_computeChecksum(char *s);
  bool _isMessageValid(char *s);

public:
  EM406(int rx, int tx);
  bool update();

  char _buffer[128];
  char _lastBuffer[128];

  char _lastGGA[128];
  char _lastGSA[128];
  char _lastRMC[128];
  char _lastGSV[128];

  void dispatchGGA(char *buffer);
  void dispatchGSA(char *buffer);
  void dispatchRMC(char *buffer);
  void dispatchGSV(char *buffer);

  char longitude[12];
  char longitudeEW[2];
  char latitude[12];
  char latitudeNS[2];
  int satellitesUsed;
  int mslAltitude;
  char utcTime[15];
  bool hasChanged = false;

  char utcTimeFormatted[10];

  char speedOverGroundKnotsStr[15];
  float speedOverGroundKnots;
  float speedOverGroundKmh;
  float courseOverGroundDeg;

  char googleMapsUrl[128];

  bool isFixed() { return _isFixed; }
};

#endif //_EM406_h