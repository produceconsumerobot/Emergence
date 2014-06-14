//
//  EmergenceSerial.cpp
//  emptyExample
//
//  Created by Sean Montgomery on 7/17/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "Emergence.h"

/*-------------------------------------------------
 * EmergenceViewer
 * Object that represents the viewers of emergence and 
 * their heart rates, etc
 *-------------------------------------------------*/

EmergenceViewer::EmergenceViewer() {
    EmergenceViewer(-1);
}

EmergenceViewer::EmergenceViewer(long uid) {
	_uid = uid;
	_rate = -1;
	_prevRate = -1;
	_accel = -500;
	_lastBeatTime = -1;
	_minRate = MIN_HEART_RATE;
	_maxRate = MAX_HEART_RATE;
	_minAccel = _minRate - _maxRate;
	_maxAccel = _maxRate - _minRate;
	_numBeats = 0;
    _rateBeats = 0;
	_aveWinLen = 5;
}

/* EmergenceViewer::getRate()
 * Returns the heart rate of the viewer.
 */
float EmergenceViewer::getRate() {
	return _rate;
}

/* EmergenceViewer::getAccel()
 * Returns the heart acceleration of the viewer.
 */
float EmergenceViewer::getAccel() {
	return _accel;
}

int EmergenceViewer::getRateMidiControlValue() {
    return getRateMidiControlValue(1.f);
}

int EmergenceViewer::getRateMidiControlValue(float multiplier) {
    float ret = ((getRate() - getMinRate()) * multiplier) + getMinRate();
    //float ret = getRate() * multiplier;
    ret = ofClamp(ret, getMinRate(), getMaxRate());
    ret = ofMap(ret, getMinRate(), getMaxRate(), 1, 127);
    //printf("\nvr=%f, m=%f, minr=%f, maxr=%f, retf=%f, reti=%i\n", getRate(), multiplier, getMinRate(), getMaxRate(), ret, (int) ret);
    return (int) ret;
}

int EmergenceViewer::getAccelMidiControlValue() {
    return getAccelMidiControlValue(1.f);
}

int EmergenceViewer::getAccelMidiControlValue(float multiplier) {
    float ret = getAccel() * multiplier;
    ret = ofClamp(ret, getMinAccel(), getMaxAccel());
    ret = ofMap(ret, getMinAccel(), getMaxAccel(), 1, 127);
    //printf("\nva=%f, m=%f, mina=%f, maxa=%f, retf=%f, reti=%i\n", getAccel(), multiplier, getMinAccel(), getMaxAccel(), ret, (int) ret);
    return (int) ret;
}

long EmergenceViewer::getID()
{
    return _uid;
}

float EmergenceViewer::getMinRate()
{
    return _minRate;
}

float EmergenceViewer::getMaxRate()
{
    return _maxRate;
}

float EmergenceViewer::getMinAccel()
{
    return _minAccel;
}

float EmergenceViewer::getMaxAccel()
{
    return _maxAccel;
}

int EmergenceViewer::getNumBeats()
{
    return _numBeats;
}

/* EmergenceViewer::recordBeat()
 * Updates the heart rate and acceleration with the time
 * of the most recent heart beat.
 */
void EmergenceViewer::recordBeat(float beatTime) {
    float currentRate;
    float currentAccel;
    
    _numBeats++;
    // If we have a previous beat to calc rate
    if (_lastBeatTime > 0) {
        currentRate = 60.0f / (beatTime - _lastBeatTime);
        
        // If current rate is reasonable
        if ((currentRate > _minRate) && (currentRate < _maxRate)) {
                        
            // Calculate Rate
            _rateBeats++;
            if (_rateBeats < _aveWinLen) {
                _rate = (currentRate + (_rate * ((float) (_rateBeats - 1)))) / ((float) _rateBeats);
            } else {
                _rate = (currentRate + (_rate * ((float) (_aveWinLen - 1)))) / ((float) _aveWinLen);
            }
            
            // If we have a previous rate to calc accel
            if (_prevRate > 0) {
                // Calculate Acceleration
                currentAccel = currentRate - _prevRate;
                int accelBeats = _rateBeats - 1;
                if (accelBeats < _aveWinLen*2) {
                    _accel = (currentAccel + (_accel * ((float) (accelBeats - 1)))) / ((float) accelBeats);
                } else {
                    _accel = (currentAccel + (_accel * ((float) (_aveWinLen*2 - 1)))) / ((float) _aveWinLen*2);
                }          
            } 
            _prevRate = currentRate;         
        } else {
            printf("\n******** cr=%f *********\n", currentRate);
        }
    }
    _lastBeatTime = beatTime;
}



/*-------------------------------------------------
 * EmergenceInstallation
 * Object that represents the installation and it's 
 * interactions -- 
 *-------------------------------------------------*/
EmergenceInstallation::EmergenceInstallation() {
    EmergenceInstallation("Location", "Venue");
}

EmergenceInstallation::EmergenceInstallation(string location, string venue) {
	_aveWinLen = 40; // Change this number to change how far back rates are average
	_recentRate = -1.f;
	_recentAccel = -500.f;
	_numViewers = 0;
	_numViewerRates = 0;
	_numViewerAccels = 0;
	_touchLen = -1.f;
	_timeSinceTouch = -1.f;
	_touching = false;
	_lastTouchEnd = -1.f;
	_lastTouchStart -1.f;
    
    _minRate = MIN_HEART_RATE;
	_maxRate = MAX_HEART_RATE;
	_minAccel = _minRate - _maxRate;
	_maxAccel = _maxRate - _minRate;
    
    //printf("\nminr=%f, maxr=%f, mina=%f, maxa=%f\n", _emergenceViewer.getMinRate(), _emergenceViewer.getMaxRate(), _emergenceViewer.getMinAccel(), _emergenceViewer.getMaxAccel());
    //printf("\nminr=%f, maxr=%f, mina=%f, maxa=%f\n", _minRate, _maxRate, _minAccel, _maxAccel);
    
	_location = location; 
	_venue = venue; 
    //printf("\nn=%i, r=%f, a=%f",(int)_numViewerRates,_recentRate, _recentAccel);
}

/* EmergenceInstallation::touchBegin
 * Call this method when a viewer touches the installation to 
 * track length of the touch.
 */
void EmergenceInstallation::touchBegin(float currentTime) {
	_lastTouchStart = currentTime;
	_touching = true;
	_numViewers++;
}

/* EmergenceInstallation::touchEnd
 * Call this method when a viewer stops touching the installation to 
 * track the lenghth of touch and time since last touch.
 */
void EmergenceInstallation::touchEnd(float currentTime) {
    _lastTouchEnd = currentTime;
    _touching = false;
}

/* EmergenceInstallation::getTouchLen
 * Returns length of current touch if currently touching or 
 * -1 otherwise.
 */
float EmergenceInstallation::getTouchLen(float currentTime) {
    if (_touching) {
        return currentTime - _lastTouchStart;
    } else {
        return -1.f;
    }
}

/* EmergenceInstallation::getTimeSinceTouch
 * Returns time since the last touch if not currently touching or 
 * -1 otherwise.
 */
float EmergenceInstallation::getTimeSinceTouch(float currentTime) {
    if (_touching) {
        return -1.f;
    } else {
        return currentTime -_lastTouchEnd;
    }
}     

float EmergenceInstallation::getMinRate()
{
    return _minRate;
}

float EmergenceInstallation::getMaxRate()
{
    return _maxRate;
}

float EmergenceInstallation::getMinAccel()
{
    return _minAccel;
}

float EmergenceInstallation::getMaxAccel()
{
    return _maxAccel;
}

/* EmergenceInstallation::updateRate
 * Updates the recent rate calculation with a new rate.
 * Up to averageWindow rates are included in the calculation.
 */
void EmergenceInstallation::updateRate(float newRate) {

    if ((newRate >= _minRate) && (newRate <= _maxRate)) {
        _numViewerRates++;
        
        if (_recentRate > 0) {
            float newAccel = newRate - _recentRate;
            
            if ((newAccel >= _minAccel) && (newAccel <= _maxAccel)) {
            _numViewerAccels++;
            
            if (_numViewerAccels < _aveWinLen) {
                _recentAccel = (newAccel + (_recentAccel * ((float) (_numViewerAccels - 1)))) / ((float) _numViewerAccels);
            } else {
                _recentAccel = (newAccel + (_recentAccel * ((float) (_aveWinLen - 1)))) / ((float) _aveWinLen);
            }
            }
        }        
            
        
        if (_numViewerRates < _aveWinLen) {
            _recentRate = (newRate + (_recentRate * ((float) (_numViewerRates - 1)))) / ((float) _numViewerRates);
        } else {
            _recentRate = (newRate + (_recentRate * ((float) (_aveWinLen - 1)))) / ((float) _aveWinLen);
        }
    }
    //printf("\nn=%i, r=%f, a=%f",(int)_numViewerRates,_recentRate, _recentAccel);
}

/* EmergenceInstallation::getAveRate
 * Returns the average rate calculation. 
 * Up to averageWindow rates are included in the calculation.
 */
float EmergenceInstallation::getRate() {
    //printf("\nr=%f, a=%f",_recentRate, _recentAccel);
	return _recentRate;
}

/* EmergenceInstallation::getAveAccel
 * Returns the average accel calculation. 
 * Up to averageWindow accel values are included in the calculation.
 */
float EmergenceInstallation::getAccel() {
	return _recentAccel;
}

int EmergenceInstallation::getRateMidiControlValue() {
    return getRateMidiControlValue(1.f);
}

int EmergenceInstallation::getRateMidiControlValue(float multiplier) {
    float ret = getRate() * multiplier;
    ret = ofClamp(ret, getMinRate(), getMaxRate());
    ret = ofMap(ret, getMinRate(), getMaxRate(), 0, 127);
    //printf("\nir=%f, m=%f, minr=%f, maxr=%f, retf=%f, reti=%i\n", getRate(), multiplier, getMinRate(), getMaxRate(), ret, (int) ret);
    return (int) ret;
}

int EmergenceInstallation::getAccelMidiControlValue() {
    return getAccelMidiControlValue(1.f);
}

int EmergenceInstallation::getAccelMidiControlValue(float multiplier) {
    float ret = getAccel() * multiplier;
    ret = ofClamp(ret, getMinAccel(), getMaxAccel());
    ret = ofMap(ret, getMinAccel(), getMaxAccel(), 0, 127);
    //printf("\nia=%f, m=%f, mina=%f, maxa=%f, retf=%f, reti=%i\n", getAccel(), multiplier, getMinAccel(), getMaxAccel(), ret, (int) ret);
    return (int) ret;
}



/* EmergenceInstallation::getAveRate
 * Sets the window length for averaging the heart rates.
 */
void EmergenceInstallation::setAveWinLen(int newWinLen) {
    _aveWinLen = newWinLen;
}

/* EmergenceInstallation::getLocation
 * Returns the current location of the installation
 */
string EmergenceInstallation::getLocation() {
	return _location;
}

/* EmergenceInstallation::getVenue
 * Returns the current venue of the installation
 */
string EmergenceInstallation::getVenue() {
	return _venue;
}



/*-------------------------------------------------
 * EmergenceSerial
 * Parses serial input from Emergence and updates
 * getMessage() until clearMessage() is called
 *-------------------------------------------------*/
EmergenceSerial::EmergenceSerial() : ofxSimpleSerial()
{
    _message = NONE;
    buffInd = 0;
    _eventTime = ofGetElapsedTimef();
    
}

/* EmergenceSerial::getMessage
 * Returns the current message from the serial stream
 * messages: NONE, TOUCH_BEGIN, TOUCH_END, HEARTBEAT, FAKEBEAT
 */
EmergenceSerial::message EmergenceSerial::getMessage() 
{
    return _message;
}

/* EmergenceSerial::clearMessage
 * Clears the current message
 */
void EmergenceSerial::clearMessage() 
{
    _message = NONE;
}

void EmergenceSerial::parse(string message)
{    
    if (message.compare("1,5") == 0)
    {
        cout << "\n" << message << " TOUCH_BEGIN ";
        _message = TOUCH_BEGIN;
    }
    if (message.compare("0,0") == 0)
    {
         cout << "\n" << message << " TOUCH_END ";
        _message = TOUCH_END;
    }
    if (message.compare("1,1") == 0)
    {
        cout << "\n" << message << " HEARTBEAT ";
        _message = HEARTBEAT;
    }
    if (message.compare("0,1") == 0)
    {
        cout << "\n" << message << " FAKEBEAT ";
        _message = FAKEBEAT;
    }
    
    if (_message != NONE)
    {
        _eventTime = ofGetElapsedTimef();
    }
}

/* EmergenceSerial::parseInput
 * Parses the serial stream for messages from Emergence
 * messages: NONE, TOUCH_BEGIN, TOUCH_END, HEARTBEAT, FAKEBEAT
 */
void EmergenceSerial::parseInput() 
{
    
    if (available() > 0) {
        readBytes(&buff[buffInd], 1);
        if (buffInd == 0) {
            _eventTime = ofGetElapsedTimef();
        }
        //printf("%c", buff[buffInd],buffInd); 
        //printf("%c:%i\n", buff[buffInd],buffInd); 
        // New line indicates end of a line
        if (buff[buffInd] == '\n') {
            //printf("yay");
            if (buffInd != (BUFFSIZE - 1)) { // We got a '/n' without a full line
                
                printf("\nERROR: "); 
                for (int i=0; i<=buffInd; i++) {
                    printf("%c", buff[i]); 
                }
                buffInd = -1; // start over
                
            } else {
                // We have a full line, now parse it
                printf("\n"); 
                for (int i=0; i<BUFFSIZE; i++) {
                    //printf("%c:%i,", buff[i], i); 
                    printf("%c", buff[i]); 
                }
                //printf(" - "); 
                
                // Start Warmup
                if ((buff[0] == '1') && (buff[2] == '5')) {
                    cout << "TOUCH_BEGIN";
                    _message = TOUCH_BEGIN;
                 }
                
                // Stop Warmup
                if ((buff[0] == '0') && (buff[2] == '0')) {
                    cout << "TOUCH_END";
                    _message = TOUCH_END;
                }
                
                // Heart Beat
                if ((buff[0] == '1') && (buff[2] == '1')) {
                    cout << "HEARTBEAT";
                    _message = HEARTBEAT;
                }
                
                // Fake Beat
                if ((buff[0] == '0') && (buff[2] == '1')) {
                    cout << "FAKEBEAT";
                    _message = FAKEBEAT;
               }
            }
        }
        buffInd++;
        //printf("++");
        if (buffInd >= BUFFSIZE) {
            // Start buffInd over
            buffInd = 0;
            //printf("=0=");
        }
    } 
}

float EmergenceSerial::getEventTime() {
    return _eventTime;
}



/*-------------------------------------------------
 * EmergenceLog
 * Functions to read and write data from emergence in a log file
 *-------------------------------------------------*/
EmergenceLog::EmergenceLog()
{
    EmergenceLog("./logs/default/");
    ofSeedRandom();
}

EmergenceLog::EmergenceLog(string logDir)
{
    
    _baseDir = "";
    //_baseDir = "./logs/";
    _logDir = logDir;
    _fileName = "EmergenceLog.txt";
    _imagesDir = "images/";

    mkdir( ofToDataPath(_baseDir).c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir( ofToDataPath(getLogDirPath()).c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
    mkdir( ofToDataPath(getImagesDirPath()).c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
    ofSeedRandom();
    
}

string EmergenceLog::getLogDirPath()
{
    return ofToDataPath(_baseDir + _logDir);
}

string EmergenceLog::getImagesDirPath()
{
    return ofToDataPath(_baseDir + _logDir + _imagesDir);
}

/* EmergenceLog::write
 * Appends a line to the log file
 */
void EmergenceLog::write(string dateTimeString, long uid, string eventType)
{
    
    string fileName = getLogDirPath() + _fileName;
    
    ofstream mFile;
    mFile.open(fileName.c_str(), ios::out | ios::app);
    mFile << uid;
    mFile << ",";
    mFile << eventType;
    mFile << ",";
    mFile << dateTimeString;
    mFile << ",\n";
    mFile.close();
}

long EmergenceLog::getLastUID() {   
    string logLine = "-1,READFAIL,READFAIL,READFAIL,READFAIL,\n";
    string prevLogLine = "-1,READFAIL,READFAIL,READFAIL,READFAIL,\n";
    
    string fileName = getLogDirPath() + _fileName;
    
    std::ifstream mFile;
    
    mFile.open(fileName.c_str());
    if (mFile.is_open()) {
        while (mFile.good()) {
            // last line is garbage
            prevLogLine = logLine;
            getline(mFile, logLine);
        }
        mFile.close();
    }     
        
    return getUID(prevLogLine);
    
    //long len = length();
    //if (len < 0) {
    //    return -1;
    //}
    //return getUID(len);
    
    //string line = getLine(len);
    //string uid;
    //istringstream liness(line);
    //getline(liness, uid, ',');
    //return atol(uid.c_str());
}

long EmergenceLog::getUID(string logLine) {
    string uid;
    istringstream liness(logLine);
    getline(liness, uid, ',');
    // **** NEED ERROR HANDLING ****
    return atol(uid.c_str());
}

long EmergenceLog::getUID(long logLineNum) {
    string line = getLine(logLineNum);
    return getUID(line);
}

string EmergenceLog::screenDateTimeString(long logLineNum)
{
    string line = getLine(logLineNum);
    screenDateTimeString(line);
}

string EmergenceLog::fileDateTimeString(long logLineNum)
{
    string line = getLine(logLineNum);
    fileDateTimeString(line);
}

string EmergenceLog::screenDateTimeString(string line)
{    
    string uid, event, date, time, oftime;
    istringstream liness(line);
    getline(liness, uid, ',');
    getline(liness, event, ',');
    getline(liness, date, ',');
    getline(liness, time, ',');
    //getline(liness, oftime, ',');
    return date + "," + time;
}

string EmergenceLog::fileDateTimeString(string line)
{   string uid, event, date, time, oftime;
    istringstream liness(line);
    getline(liness, uid, ',');
    getline(liness, event, ',');
    getline(liness, date, ',');
    getline(liness, time, ',');
    getline(liness, oftime, ',');
    return date + "," + time + "," + oftime;
}

/* EmergenceLog::dateTimeString
 * Returns the current date/time in following format:
 * 2011_05_26_14_36_587
 * NEEDS TO BE WRITTEN
 */
string EmergenceLog::screenDateTimeString()
{
    string output = "";
    
    int year = ofGetYear();
    int month = ofGetMonth();
    int day = ofGetDay();
    int hours = ofGetHours();
    int minutes = ofGetMinutes();
    int seconds = ofGetSeconds();
    
    output = output + ofToString(year) + ".";
    if (month < 10) output = output + "0";
    output = output + ofToString(month) + ".";
    if (day < 10) output = output + "0";
    output = output + ofToString(day) + ", ";
    if (hours < 10) output = output + "0";
    output = output + ofToString(hours) + ".";
    if (minutes < 10) output = output + "0";
    output = output + ofToString(minutes) + ".";
    if (seconds < 10) output = output + "0";
    output = output + ofToString(seconds);
    
    return output;
}

/* EmergenceLog::dateTimeString
 * Returns the current date/time in following format:
 * 2011-05-26,14-36-587,28.469
 */
string EmergenceLog::fileDateTimeString(float beatTime)
{
    string output = "";
    
    int year = ofGetYear();
    int month = ofGetMonth();
    int day = ofGetDay();
    int hours = ofGetHours();
    int minutes = ofGetMinutes();
    int seconds = ofGetSeconds();
    
    output = output + ofToString(year) + ".";
    if (month < 10) output = output + "0";
    output = output + ofToString(month) + ".";
    if (day < 10) output = output + "0";
    output = output + ofToString(day) + ", ";
    if (hours < 10) output = output + "0";
    output = output + ofToString(hours) + ".";
    if (minutes < 10) output = output + "0";
    output = output + ofToString(minutes) + ".";
    if (seconds < 10) output = output + "0";
    output = output + ofToString(seconds) + ", ";
    output = output + ofToString(beatTime, 3);
    
    return output;
}

string EmergenceLog::getImageFileName(long logLineNum)
{
    return fileDateTimeString(logLineNum) + ".jpg";
}

string EmergenceLog::getImageFilePath(long logLineNum)
{
    return getImagesDirPath() + getImageFileName(logLineNum);
}

string EmergenceLog::getLine(long logLineNum)
{
    long count = 0;
    string out = "-1,READFAIL,READFAIL,READFAIL,READFAIL,\n";
    
    string fileName = getLogDirPath() + _fileName;
    
    std::ifstream mFile;
    
    mFile.open(fileName.c_str());
    if (mFile.is_open()) {
        while (mFile.good() && (count < logLineNum)) {
            getline(mFile, out);
            count++;
        }
        mFile.close();
    }     
    return out;
}

long EmergenceLog::getRandomLineNumber()
{
    long logLen = length();
    float rand = ofRandomuf();
    float temp = rand * ((float) logLen);
    long lineNum = (long) temp;
    if (lineNum < 1) {
        return 1;
    } else {
        return lineNum;
    }
}

int EmergenceLog::getFirstHBInfo(long uid, int nLines, long* lineNums, string * lineStrings)
{
    //printf("length()");
    //long len = length();
    //printf("done()");
    long i = 1;
    int count = 0;
    string line, ruid, event, date, time, oftime;
    
    string fileName = getLogDirPath() + _fileName;
    std::ifstream mFile;
    
    mFile.open(fileName.c_str());
    if (mFile.is_open()) {
        while (mFile.good() && (count < nLines))
        {
            getline(mFile, line);
            
            istringstream liness(line);
            getline(liness, ruid, ',');
            
            if (uid == atol(ruid.c_str()))
            {
                getline(liness, event, ',');
                
                if (event.compare("HB") == 0) {
                    printf("HB%i", count);

                    if (lineNums != NULL) {
                        lineNums[count] = i;
                    }
                    if (lineStrings != NULL) {
                        lineStrings[count] = line;
                    } 
                    count++;
                }
            }
            i++;
        }    
        mFile.close();
    }
    return count;
}
/*
int EmergenceLog::getFirstHBLineNums(long uid, int nLines, long* out)
{
    printf("length()");
    long len = length();
    printf("done()");
    long i = 1;
    int count = 0;
    string ruid, event, date, time, oftime;
    
    while ((i <= len) && (count < nLines))
    {
        string line = getLine(i);
        
        istringstream liness(line);
        getline(liness, ruid, ',');
        
        if (uid == atol(ruid.c_str()))
        {
            getline(liness, event, ',');
            
            if (event.compare("HB") == 0) {
                printf("HB");
                out[count] = i;
                count++;
            }
        }
        i++;
    }    
    return count;
}

int EmergenceLog::getFirstHBDateTimes(long uid, int nLines, string* out) 
{
    long len = length();
    long i = 1;
    int count = 0;
    string ruid, event, date, time, oftime;
    
    while ((i <= len) && (count < nLines))
    {
        string line = getLine(i);
        istringstream liness(line);
        getline(liness, ruid, ',');
       
        if (uid == atol(ruid.c_str())) 
        {
            getline(liness, event, ',');
            
            if  (event.compare("HB") == 0)
            {
                getline(liness, date, ',');
                getline(liness, time, ',');
                getline(liness, oftime, ',');
                
                out[count] = date + "," + time + "," + oftime;
                count++;
            }
        }
        i++;
    }    
    return count;
}
 */

/* EmergenceLog::length()
 * Returns number of lines in the log file.
 */
long EmergenceLog::length()
{
    long logLen = -1;
    
    string fileName = getLogDirPath() + _fileName;
    
    std::ifstream mFile;
    
    mFile.open(fileName.c_str());
    if (mFile.is_open()) {
        while (mFile.good()) {
            string s;
            getline(mFile, s);
            logLen++;
        }
        mFile.close();
    }
    
    return logLen;
}

EmergenceViewer EmergenceLog::getEmergenceViewer(long uid){
    // find lines in log files with corresponding user
    //int numFileLines = 0
    EmergenceViewer ev(uid);
    //float beatTimes[numFileLines] = getLineTimes();
    // for (int i=0; i<numFileLines; i++) {
    // ev.updateRateAccel(beatTimes[i]);
    //}
    return ev;
}


BeatPlayer::BeatPlayer() : ofSoundPlayer()
{
 	_lastBeatTime = -1.f;
	_interBeatInterval = -1.f;
}
void BeatPlayer::play() {
	_lastBeatTime = ofGetElapsedTimef();
	_beatsSincePlay++;
    ofSoundPlayer::play();
}
void BeatPlayer::stop() {
	_lastBeatTime = -1.f;
    _beatsSincePlay = 0;
	ofSoundPlayer::stop();
}
void BeatPlayer::setInterBeatInterval(float interBeatInterval) {
	_interBeatInterval = interBeatInterval;
}
float BeatPlayer::getInterBeatInterval() {
	return _interBeatInterval;
}
bool BeatPlayer::queryPlay(float currentTime) {
	if ((_lastBeatTime > 0.) && (_interBeatInterval > 0.)){
		// We've played beats already
		if ((currentTime - _lastBeatTime) > _interBeatInterval) {
            //printf("%f, %f, %f\n", currentTime, _lastBeatTime, _interBeatInterval);
			// Time to play another one
            play();
            return true;
		}
	} else {
        _lastBeatTime = currentTime;
    }
    return false;
    
}
long BeatPlayer::getBeatsSincePlay()
{
	return _beatsSincePlay;
}


MIDIBeatPlayer::MIDIBeatPlayer() {
    setChannel(-1);
    setID(-1);
    setValue(-1);
 	_lastBeatTime = -1.f;
	_interBeatInterval = -1.f;    
}
/*MIDIBeatPlayer::MIDIBeatPlayer(ofxMidiOut midiOut, int channel, int id, int value) 
{
    setMidiOut(midiOut);
    setChannel(channel);
    setID(id);
    setValue(value);
 	_lastBeatTime = -1.f;
	_interBeatInterval = -1.f;
}*/
void MIDIBeatPlayer::play() {
	_lastBeatTime = ofGetElapsedTimef();
	_beatsSincePlay++;
    _midiOut->sendNoteOff(_channel, _id, _value);
    _midiOut->sendNoteOn(_channel, _id, _value);
}
void MIDIBeatPlayer::stop() {
	_lastBeatTime = -1.f;
    _beatsSincePlay = 0;
    _midiOut->sendNoteOff(_channel, _id, _value);
}
void MIDIBeatPlayer::setMidiOut(ofxMidiOut * midiOut) {
    _midiOut = midiOut;
}
ofxMidiOut * MIDIBeatPlayer::getMidiOut() {
    return _midiOut;
}
void MIDIBeatPlayer::setChannel(int channel) {
    _channel = channel;
}
void MIDIBeatPlayer::setID(int id) {
    _id = id;
}
void MIDIBeatPlayer::setValue(int value) {
    _value = value;
}
int MIDIBeatPlayer::getChannel() {
    return _channel;
}
int MIDIBeatPlayer::getID() {
    return _id;
}
int MIDIBeatPlayer::getValue() {
    return _value;
}
void MIDIBeatPlayer::setInterBeatInterval(float interBeatInterval) {
	_interBeatInterval = interBeatInterval;
}
float MIDIBeatPlayer::getInterBeatInterval() {
	return _interBeatInterval;
}
bool MIDIBeatPlayer::queryPlay(float currentTime) {
	if (query(currentTime))
    {
        play();
        return true;
    } else {
        return false;
    }
}
bool MIDIBeatPlayer::query(float currentTime) {
	if ((_lastBeatTime > 0.) && (_interBeatInterval > 0.)){
		// We've played beats already
		if ((currentTime - _lastBeatTime) > _interBeatInterval) {
            //printf("%f, %f, %f\n", currentTime, _lastBeatTime, _interBeatInterval);
			// Time to play another one
            return true;
		}
	} else {
        _lastBeatTime = currentTime;
    }
    return false;
    
}
long MIDIBeatPlayer::getBeatsSincePlay()
{
	return _beatsSincePlay;
}


Recollection::Recollection() {
    _numRecallBeats = 0;
    _beatCount = _numRecallBeats + 1;
    _controlValueStart = 127;
    _controlValueEnd = 0;
    _screenDateTimeString = "";
    _fileDateTimeString = "";
    long _uid = -1;
    _logLineNums = NULL;
    //_logLineStrings = NULL;
}
Recollection::Recollection(EmergenceLog log, MIDIBeatPlayer beatPlayer, int numRecallBeats, int midiControlMsg) {
    _numRecallBeats = numRecallBeats;
    _beatCount = _numRecallBeats + 1;
    _controlValueStart = 127;
    _controlValueEnd = 0;
    _screenDateTimeString = "";
    _fileDateTimeString = "";
    long _uid = -1;
    _logLineNums = new long [numRecallBeats+1];
    //_logLineStrings = new string [numRecallBeats+1];
    _midiControlMsg = midiControlMsg;
    _log = log;
    _beatPlayer = beatPlayer;
    _rate;
}

Recollection::~Recollection()
{
    delete[] _logLineNums;
    //delete[] _logLineStrings;
}

void Recollection::setControlValueRange(int start, int end) 
{
    _controlValueStart = start;
    _controlValueEnd = end;
}

bool Recollection::isBeating() 
{
    if ((_numRecallBeats == 0) || (_uid == -1)) {
        return false;
    } else {
        return _beatCount <= _numRecallBeats;
    }
}

void Recollection::loadRecollection(bool lastViewer)
{    
    int nBeats = 0;
    int nAttempts = 0; // to avoid getting hung in while loop
    
    // load a recollection if we have enough log lines
    if (_log.length() > _numRecallBeats + 2) {
        if (lastViewer) {
            _uid = _log.getLastUID();
            _logLineStrings[0] = "happy";
            if (_uid != -1) {
                nBeats = _log.getFirstHBInfo(_uid, _numRecallBeats, _logLineNums, _logLineStrings);
            } else {
                printf(" ERROR: uid=-1");
            }
        } 
        while ((nBeats < _numRecallBeats) && (nAttempts < 100))
        {
            _uid = _log.getUID(_log.getRandomLineNumber());
            nAttempts++;
            if (_uid != -1) {
                nBeats = _log.getFirstHBInfo(_uid, _numRecallBeats, _logLineNums, _logLineStrings);
            } else {
                printf(" ERROR: uid=-1");
            }
        }
        
        // Check if we succeeded before hitting attempt limit
        if ((nAttempts == 100) || (nBeats > _numRecallBeats)) {
            printf(" ERROR: nAttempts=%i, nBeats=%i", nAttempts, nBeats);
            _uid = -1;
        } else {
            _beatPlayer.setInterBeatInterval(logLines2ibi(nBeats, _logLineStrings));
            _rate = logLines2rate(nBeats, _logLineStrings);
        
            _beatCount = 0;
            setDateTimeStrings();
        }
    
        printf("\nuid=%i, nBeats=%i, ln=",(int) _uid, nBeats);
        for (int i=0; i<nBeats; i++) {
            printf("\n%ld, %s", _logLineNums[i], _logLineStrings[i].c_str());
        }
    } else {
        printf(" ERROR: log.length=%i", (int) _log.length());
    }
}

void Recollection::stopBeating() 
{
    _beatCount = _numRecallBeats + 1;
    _beatPlayer.stop();
    
}

void Recollection::play()
{
    if(isBeating()) {
        _beatPlayer.play();
        _beatPlayer.getMidiOut()->sendControlChange(_beatPlayer.getChannel(), _midiControlMsg, _controlValueStart);
        setDateTimeStrings();
        _beatCount++;
    }
}

bool Recollection::queryPlay(float currentTime)
{
    if (isBeating()) {
        if (_beatPlayer.query(currentTime)) {
            if (_beatCount < _numRecallBeats) { // make picture stay on screen till end of beat
                float multiplier = ((float) (_numRecallBeats - 1 - _beatCount)) / ((float) _numRecallBeats - 1);
                multiplier = ofClamp(multiplier, 0.f, 1.f);
                int value = (int) ((((float) (_controlValueStart - _controlValueEnd)) * multiplier) + _controlValueEnd);
                _beatPlayer.getMidiOut()->sendControlChange(_beatPlayer.getChannel(), _midiControlMsg, value);
                _beatPlayer.play();
            }
            //printf("\nc=%i,id=%i,v=%i,b=%i,n=%i",_beatPlayer.getChannel(),_beatPlayer.getID(), value,_beatCount,_numRecallBeats-1);
            setDateTimeStrings();
            _beatCount++;
            return true;
        }
    }
    return false;
}

void Recollection::setDateTimeStrings()
{
    if(isBeating()) {
        _screenDateTimeString = _log.screenDateTimeString(_logLineStrings[_beatCount]);
        _fileDateTimeString = _log.fileDateTimeString(_logLineStrings[_beatCount]);
    }
}

string Recollection::getScreenDateTimeString()
{
    return _screenDateTimeString;
}

string Recollection::getFileDateTimeString()
{
    return _fileDateTimeString;
}

int Recollection::getRateMidiControlValue() {
    return getRateMidiControlValue(1.f);
}

int Recollection::getRateMidiControlValue(float multiplier) {
    float ret = ((getRate() - MIN_HEART_RATE) * multiplier) + MIN_HEART_RATE;
    ret = ofClamp(ret, MIN_HEART_RATE, MAX_HEART_RATE);
    ret = ofMap(ret, MIN_HEART_RATE, MAX_HEART_RATE, 1, 127);
    //printf("\nvr=%f, m=%f, minr=%f, maxr=%f, retf=%f, reti=%i\n", getRate(), multiplier, getMinRate(), getMaxRate(), ret, (int) ret);
        //cout << "\n" << ret;
    return (int) ret;
}


float Recollection::getRate()
{
    return _rate;
}

float Recollection::logLines2rate(int nLines, string logLines[])
{
    float ibi = logLines2ibi(nLines, logLines);
    return 60 / ibi;
}

float Recollection::logLines2ibi(int nLines, string logLines[])
{
    string date, time, oftime;
    
    istringstream liness1(_log.fileDateTimeString(logLines[0]));
    getline(liness1, date, ',');
    getline(liness1, time, ',');
    getline(liness1, oftime, ',');
    float firstTime = atof(oftime.c_str());
    //printf("\n%s, first=%s,%f", timeStrings[0].c_str(), oftime.c_str(), firstTime);
    
    istringstream linessN(_log.fileDateTimeString(logLines[nLines-1]));
    getline(linessN, date, ',');
    getline(linessN, time, ',');
    getline(linessN, oftime, ',');
    float lastTime = atof(oftime.c_str());
    //printf("\nlast=%s,%f", oftime.c_str(), lastTime);
    
    return (lastTime - firstTime) / nLines;    
}
/*
float Recollection::lineNums2ibi(int nLines, long lineNums[])
{
    string * timeStrings = new string [nLines];
    
    for (int i=0; i<nLines; i++) {
        timeStrings[i] = _log.fileDateTimeString(lineNums[i]);
    }
    
    return dateTimes2ibi(nLines, timeStrings);
    delete[] timeStrings;
}

float Recollection::dateTimes2ibi(int nTimes, string timeStrings[])
{
    string date, time, oftime;
    
    istringstream liness1(timeStrings[0]);
    getline(liness1, date, ',');
    getline(liness1, time, ',');
    getline(liness1, oftime, ',');
    float firstTime = atof(oftime.c_str());
    //printf("\n%s, first=%s,%f", timeStrings[0].c_str(), oftime.c_str(), firstTime);
    
    istringstream linessN(timeStrings[nTimes-1]);
    getline(linessN, date, ',');
    getline(linessN, time, ',');
    getline(linessN, oftime, ',');
    float lastTime = atof(oftime.c_str());
    //printf("\nlast=%s,%f", oftime.c_str(), lastTime);
    
    return (lastTime - firstTime) / nTimes;
}
 */


