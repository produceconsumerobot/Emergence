//
//  EmergenceSerial.h
//  emptyExample
//
//  Created by Sean Montgomery on 7/17/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _EMERGENCE_SERIAL
#define _EMERGENCE_SERIAL

#define MIN_HEART_RATE 35
#define MAX_HEART_RATE 240

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>


#include "ofSerial.h"
#include "ofSoundPlayer.h"
#include "ofUtils.h"
#include "ofxMidi.h"
#include "ofMain.h"
#include "ofxSimpleSerial.h"
#include "ofxThread.h"

/*-------------------------------------------------
 * EmergenceViewer
 * Object that represents the viewers of emergence and 
 * their heart rates, etc
 *-------------------------------------------------*/
class EmergenceViewer
{
public:
    EmergenceViewer();
    EmergenceViewer(long uid);
    float getRate();
    float getAccel();
    int getRateMidiControlValue();
    int getRateMidiControlValue(float multiplier);
    int getAccelMidiControlValue();
    int getAccelMidiControlValue(float multiplier);
    void recordBeat(float beatTime);
    long getID();
    float getMinRate();
    float getMaxRate();
    float getMinAccel();
    float getMaxAccel();
    int getNumBeats();
    
protected:
    long _uid;
    float _rate;
    float _prevRate;
    float _accel;
    float _lastBeatTime;
    float _minRate;
    float _maxRate;
    float _minAccel;
    float _maxAccel;
    int _numBeats;
    int _rateBeats;
    int _aveWinLen;
};



/*-------------------------------------------------
 * EmergenceInstallation
 * Object that represents the installation and it's 
 * interactions -- 
 *-------------------------------------------------*/
class EmergenceInstallation
{
public:
    EmergenceInstallation();
    EmergenceInstallation(string location, string venue);
    void touchBegin(float currentTime);
    void touchEnd(float currentTime);
    float getTouchLen(float currentTime);
    float getTimeSinceTouch(float currentTime);
    void updateRate(float newRate);
    float getRate();
    float getAccel();
    int getRateMidiControlValue();
    int getRateMidiControlValue(float multiplier);
    int getAccelMidiControlValue();
    int getAccelMidiControlValue(float multiplier);
    float getMinRate();
    float getMaxRate();
    float getMinAccel();
    float getMaxAccel();
    void setAveWinLen(int newWinLen);
    string getLocation();
    string getVenue();
    
protected:
    int _aveWinLen;
    float _recentRate;
    float _recentAccel;
    long _numViewers;
    long _numViewerRates;
    long _numViewerAccels;
    float _touchLen;
    float _timeSinceTouch;
    bool _touching;
    float _lastTouchEnd;
    float _lastTouchStart;
    string _location;
    string _venue;
    EmergenceViewer _emergenceViewer;
    float _minRate;
    float _maxRate;
    float _minAccel;
    float _maxAccel;
};



/*-------------------------------------------------
 * EmergenceSerial
 * Parses serial input from Emergence and updates
 * getMessage() until clearMessage() is called
 *-------------------------------------------------*/
#define BUFFSIZE 4

class EmergenceSerial : public ofxSimpleSerial
{
public:
    enum message { NONE=0, TOUCH_BEGIN=1, TOUCH_END=2, HEARTBEAT=3, FAKEBEAT=4 };

    EmergenceSerial();
    void parse(string message);
    void parseInput();
    message getMessage();
    void clearMessage(); 
    float getEventTime();
    
protected:
    
    message _message;  
    int buffInd;
    float _eventTime;
    unsigned char buff[BUFFSIZE];
};



/*-------------------------------------------------
 * EmergenceLog
 * Functions to write data from emergence in a log file
 *-------------------------------------------------*/
class EmergenceLog
{
public:
    EmergenceLog();
    EmergenceLog(string logDirPath);
    string getLogDirPath();
    string getImagesDirPath();
    void write(string dateTimeString, long uid, string eventType);
    static string fileDateTimeString(float beatTime);
    string fileDateTimeString(long logLineNum);
    string fileDateTimeString(string line);
    static string screenDateTimeString();
    string screenDateTimeString(long logLineNum);
    string screenDateTimeString(string line);
    EmergenceViewer getEmergenceViewer(long uid);
    long length();
    long getRandomLineNumber();
    string getLine(long logLineNum);
    long getUID(long logLineNum);
    long getUID(string logLine);
    long getLastUID();
    string getImageFileName(long logLineNum);
    string getImageFilePath(long logLineNum);
    //string getDateTimeString(long logLineNum);
    //int getFirstHBDateTimes(long uid, int nLines, string* out);
    //int getFirstHBLineNums(long uid, int nLines, long* out);
    int getFirstHBInfo(long uid, int nLines, long* lineNums, string* lineStrings);
  
private:
    string _baseDir;
    string _logDir;
    string _fileName;
    string _imagesDir;
};

class BeatPlayer : public ofSoundPlayer
{
public:
    BeatPlayer();
    void play();
    void stop();
    void setInterBeatInterval(float interBeatInterval);
    float getInterBeatInterval();
    bool queryPlay(float currentTime);
    long getBeatsSincePlay();
    
protected:
    float _lastBeatTime; // in seconds since of openned
    float _interBeatInterval; // interbeat interval in seconds
    long _beatsSincePlay;
};


class MIDIBeatPlayer 
{
public:
    MIDIBeatPlayer();
    //MIDIBeatPlayer(ofxMidiOut midiOut, int channel, int id, int value);
    void play();
    void stop();
    void setMidiOut(ofxMidiOut * midiOut);
    ofxMidiOut * getMidiOut();
    void setChannel(int channel);
    void setID(int id);
    void setValue(int value);
    int getChannel();
    int getID();
    int getValue();
    void setInterBeatInterval(float interBeatInterval);
    float getInterBeatInterval();
    bool queryPlay(float currentTime);
    bool query(float currentTime);
    long getBeatsSincePlay();
    
protected:
    ofxMidiOut * _midiOut;
    int _channel;
    int _id;
    int _value;
    float _lastBeatTime; // in seconds since of openned
    float _interBeatInterval; // interbeat interval in seconds
    long _beatsSincePlay;
};


class Recollection
{
public:
    Recollection();
    Recollection(EmergenceLog log, MIDIBeatPlayer beatPlayer, int numRecallBeats, int midiControlMsg);
    ~Recollection();
    void setControlValueRange(int start, int end);
    void loadRecollection(bool lastViewer);
    void setDateTimeStrings();
    string getScreenDateTimeString();
    string getFileDateTimeString();
    void stopBeating();
    bool isBeating();
    void play();
    bool queryPlay(float currentTime);
    float logLines2ibi(int nTimes, string logLines[]);
    float logLines2rate(int nTimes, string logLines[]);
    //float lineNums2ibi(int nLines, long lineNums[]);
    //float dateTimes2ibi(int nTimes, string timeStrings[]);
    float getRate();
    int getRateMidiControlValue(float multiplier);
    int getRateMidiControlValue();
    
    
private:
    EmergenceLog _log;
    MIDIBeatPlayer _beatPlayer;
    int _numRecallBeats;
    int _beatCount;
    int _controlValueStart;
    int _controlValueEnd;
    long * _logLineNums;
    string _logLineStrings[50]; // This should be dynamically allocated
    int _midiControlMsg;
    string _screenDateTimeString;
    string _fileDateTimeString;
    long _uid;
    float _rate;
};



#endif