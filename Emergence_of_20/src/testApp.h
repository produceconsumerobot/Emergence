#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxSimpleSerial.h"


#define NUM_VIEWERS 5
#define NUM_MEMORIES 2
#define NUM_RECALL_BEATS 7

#include "Emergence.h"

class testApp : public ofBaseApp{

public:
        
    string location;
    string venue;
    
    EmergenceInstallation emergenceInstallation;

    EmergenceViewer viewers[NUM_VIEWERS];
    int viewerID;
    
    EmergenceLog log;
    EmergenceLog logtest;
    string logDir;

    EmergenceSerial serial;
    string message;
    
    ofxMidiOut midiout;    
        
    float beatTime;
    bool heartBeating;
    string fileTimeString;
    string screenTimeString;
    float loopTime;
    
    ofTrueTypeFont title;
    ofTrueTypeFont header;

    // Sound players
    //ofSoundPlayer heartSound;
    //ofSoundPlayer fakeSound;
    //ofSoundPlayer warmupSound;
    
    //BeatPlayer memorySounds[NUM_MEMORIES];
    MIDIBeatPlayer memorySounds[NUM_MEMORIES];
    int memorySoundsId;
    int memorySoundsValue;
    
    Recollection recollection;
    bool firstRecallBeat;

    
    //int heartRateMidiID;
    int warmupMidiChannel;
    int heartBeatMidiChannel;
    int fakeBeatMidiChannel;
    int viewerRateMidiChannel;
    int viewerAccelMidiChannel;
    int installationRateMidiChannel;
    int installationAccelMidiChannel;
    int touchLenMidiChannel;
    
    int warmupMidiId;
    int heartBeatMidiId;
    int fakeBeatMidiId;
    int viewerRateMidiId;
    int viewerAccelMidiId;
    int installationRateMidiId;
    int installationAccelMidiId;
    int touchLenMidiId;
    
    int warmupMidiValue;
    int heartBeatMidiValue;
    int fakeBeatMidiValue;
    int viewerRateMidiValue;
    int viewerAccelMidiValue;
    int installationRateMidiValue;
    int installationAccelMidiValue;
    int touchLenMidiValue;

    int viewerRateMidiControlMsg;
    int viewerAccelMidiControlMsg;
    int installationRateMidiControlMsg;
    int installationAccelMidiControlMsg;
    int touchLenMidiControlMsg;
    int fakeBeatMidiControlMsg;
    int fakeBeatRateMidiControlMsg;
    
    int viewerRateMidiControlValue;
    int viewerAccelMidiControlValue;
    int installationRateMidiControlValue;
    int installationAccelMidiControlValue;
    int touchLenMidiControlValue;

    float viewerAccelMidiMultiplier;
    float installationAccelMidiMultiplier;
    float touchLenMidiMultiplier;
    
    bool viewerRateMidiOn;
    bool viewerAccelMidiOn;
    bool installationRateMidiOn;
    bool installationAccelMidiOn;
    bool touchLenMidiOn;
    
    bool midiMapMode;
        
    // Image / Video
    bool cameraOn;
    ofVideoGrabber 		vidGrabber;
    unsigned char * 	videoInverted;
    ofTexture			videoTexture;
    int 				camWidth;
    int 				camHeight;
    ofImage             flickr;
    ofImage             pic;
    string              picPath;
    
    void setup();
    void onNewMessage(string & message);
    void update();
    void draw();
    void test();
    
    const void * testPtr;

    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);   
};

#endif
