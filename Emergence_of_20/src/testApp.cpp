#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    
    // Set up MIDI port
    midiout.listPorts();
    midiout.openPort(0);
    
    
    // **** OPTIONS **** //
    
    cameraOn = true;
    midiMapMode = false;
    
    location = "Queens, NY";
    venue = "SensorStar Labs";
    //location = "test";
    //venue = "test";
    
    // **** END OPTIONS **** //
    
    
    // **** COMPUTER SPECIFIC VARIABLES **** //
    
    logDir = "/Volumes/Macintosh HD2/Emergence/logs/";
    //logDir = "./logs/";
    
    // Set up Serial devices
    
    //serial.setup("/dev/cu.usbserial-A70064Yu", 115200); // Sean's Arduino Decimila
    serial.setup("/dev/cu.usbserial-A800f7P4", 115200); // Emergence Arduino Demu
    //serial.setup("/dev/cu.usbserial-A700dDnj", 115200); // Diego's box
    
    // **** END COMPUTER SPECIFIC VARIABLES **** // 
    
    
    /**** DIEGO'S VARIABLES ****/
    
    warmupMidiChannel = 1;
    heartBeatMidiChannel = 2;
    fakeBeatMidiChannel = 3;
    viewerRateMidiChannel = 4;
    viewerAccelMidiChannel = 5;
    installationRateMidiChannel = 6;
    installationAccelMidiChannel = 7;
    touchLenMidiChannel = 8;
    
    warmupMidiId = 60;
    heartBeatMidiId = 60;
    fakeBeatMidiId = 60;
    viewerRateMidiId = 60;
    viewerAccelMidiId = 60;
    installationRateMidiId = 60;
    installationAccelMidiId = 60;
    touchLenMidiId = 60;
    
    warmupMidiValue = 100;
    heartBeatMidiValue = 101;
    fakeBeatMidiValue = 103;
    viewerRateMidiValue = 100;
    viewerAccelMidiValue = 100;
    installationRateMidiValue = 100;
    installationAccelMidiValue = 100;
    touchLenMidiValue = 100;

    viewerRateMidiControlMsg = 16;
    viewerAccelMidiControlMsg = 17;
    installationRateMidiControlMsg = 50;
    installationAccelMidiControlMsg = 19;
    touchLenMidiControlMsg = 48;
    fakeBeatMidiControlMsg = 49;
    fakeBeatRateMidiControlMsg = 18;

    viewerRateMidiControlValue = 63;
    viewerAccelMidiControlValue = 63;
    installationRateMidiControlValue = 63;
    installationAccelMidiControlValue = 63;
    touchLenMidiControlValue = 63;
    
    viewerAccelMidiMultiplier = 50.f;
    installationAccelMidiMultiplier = 5.f;
    touchLenMidiMultiplier = 5.f;
    
    viewerRateMidiOn = false;
    viewerAccelMidiOn = false;
    installationRateMidiOn = false;
    installationAccelMidiOn = false;
    touchLenMidiOn = false;
        
    // Init Memory Sounds
    memorySoundsId = 60;
    memorySoundsValue = 127;
    for (int i=0; i<NUM_MEMORIES; i++) {
        memorySounds[i].setMidiOut(&midiout);
        memorySounds[i].setChannel(16-i);
        memorySounds[i].setID(memorySoundsId);
        memorySounds[i].setValue(memorySoundsValue);
    }
    /**** END DIEGO'S VARIABLES ****/
    
    
    // List serial devices
    serial.enumerateDevices();
    serial.startContinuesRead();
	ofAddListener(serial.NEW_MESSAGE,this,&testApp::onNewMessage);
    message = "";

        
    emergenceInstallation = EmergenceInstallation(location, venue);
    
    viewerID = -1;  
    heartBeating = false;
    // Init viewers with -1 ID
    for (int i=0; i<NUM_VIEWERS; i++) {
        viewers[i] = EmergenceViewer(viewerID);
    }
    
    // Set up image capture
    if (cameraOn) {
        camWidth 		= 800;	// try to grab at this size. 
        camHeight 		= 600;
        vidGrabber.setVerbose(true);
        vidGrabber.initGrabber(camWidth,camHeight);
        videoTexture.allocate(camWidth,camHeight, GL_RGB); 
        picPath = "";
    }
    flickr.loadImage("images/flickr_logo_150.jpg");
       
    // important for ofSoundStream?
	ofSetFrameRate(250);

    // Set up font for flickr image title
    title.loadFont("verdana.ttf", 18);
    header.loadFont("verdana.ttf", 32);
    
    // Measures speed of loop
    loopTime = ofGetElapsedTimef();
        
    // Set up the logger to write in a Location_Venue directory
    log = EmergenceLog(logDir + location + ", " + venue + "/");
    
    logtest = EmergenceLog(logDir + "test" + ", " + "test" + "/");
    logtest.write("test", 999, "joy");
    printf("\nloglen=%i, log=%s",(int) logtest.length(),logtest.getLine( logtest.length()).c_str());
    
    recollection = Recollection(log, memorySounds[0], NUM_RECALL_BEATS, fakeBeatMidiControlMsg);

}

void testApp::test()
{
    cout << "testing";
}

void testApp::onNewMessage(string & message)
{
    // Only do the most time-sensitive things in onNewMessage
    serial.parse(message);

    if (!midiMapMode) {
        
        // TOUCH_BEGIN
        if (serial.getMessage() == serial.TOUCH_BEGIN) {
            midiout.sendNoteOff(fakeBeatMidiChannel, fakeBeatMidiId, fakeBeatMidiValue);
            midiout.sendNoteOn(warmupMidiChannel, warmupMidiId, warmupMidiValue );
            //warmupSound.play();
            
            // If the viewer is initialized and we got enough beats to calculate acceration
            if ((viewers[0].getID() != -1) && (viewers[0].getRate() > 0) && (viewers[0].getAccel() != -500)) {
                // Shift the past viewers array
                for (int i=NUM_VIEWERS-1; i>0; i--) {
                    viewers[i] = viewers[i-1];
                }
            }
            // Add new viewer
            viewerID = log.getLastUID() + 1;
            viewers[0] = EmergenceViewer(viewerID);
            
            //float touchTime = ofGetElapsedTimef();        
            float touchTime = serial.getEventTime();        
            emergenceInstallation.touchBegin(touchTime);
            
            recollection.stopBeating();
            firstRecallBeat = true;
            
            // Write event to the log
            log.write(EmergenceLog::fileDateTimeString(touchTime), viewers[0].getID(), "TB");
        }
        
        // TOUCH_END
        if (serial.getMessage() == serial.TOUCH_END) {            
            midiout.sendNoteOff(heartBeatMidiChannel, heartBeatMidiId, heartBeatMidiValue);        
            midiout.sendNoteOff(warmupMidiChannel, warmupMidiId, warmupMidiValue);
            //warmupSound.stop();
            
            // Stop the sound generator
            //ofSoundStreamStop();
            
            //float touchTime = ofGetElapsedTimef();    
            float touchTime = serial.getEventTime();  
            emergenceInstallation.touchBegin(touchTime);
            
            // Write event to the log
            log.write(EmergenceLog::fileDateTimeString(touchTime), viewers[0].getID(), "TE");
            
            for (int i=0; i<NUM_MEMORIES; i++) {
                memorySounds[i].stop();
            }
            //midiout.sendNoteOff(heartRateMidiChannel, targetFrequency/2, 50);   
            firstRecallBeat = true;
        }
        
        // HEARTBEAT
        if (serial.getMessage() == serial.HEARTBEAT) {
            // Get the time of the beat
            //beatTime = ofGetElapsedTimef();
            beatTime = serial.getEventTime();  
            fileTimeString = EmergenceLog::fileDateTimeString(beatTime);
            screenTimeString = EmergenceLog::screenDateTimeString();
            
            midiout.sendNoteOff(warmupMidiChannel, warmupMidiId, warmupMidiValue);
            midiout.sendNoteOff(heartBeatMidiChannel, heartBeatMidiId, heartBeatMidiValue);        
            midiout.sendNoteOn(heartBeatMidiChannel, heartBeatMidiId, heartBeatMidiValue);
            //warmupSound.stop();
            //heartSound.play();
            
            // Grab a frame from the webcam
            if (cameraOn) {
                vidGrabber.grabFrame();
            }
            
            // Update the viewer's rate/accel stats
            viewers[0].recordBeat(beatTime);
            // Update the installation's rate/accel stats
            emergenceInstallation.updateRate(viewers[0].getRate());
            
            // Write event to the log
            log.write(fileTimeString, viewers[0].getID(), "HB");
            if (cameraOn) {
                pic.setFromPixels(vidGrabber.getPixels(), vidGrabber.getWidth(), vidGrabber.getHeight(), OF_IMAGE_COLOR, true);
            }
        }   
        
        // FAKEBEAT
        if (serial.getMessage() == serial.FAKEBEAT) {
            midiout.sendNoteOff(fakeBeatMidiChannel, fakeBeatMidiId, fakeBeatMidiValue);
            midiout.sendNoteOn(fakeBeatMidiChannel, fakeBeatMidiId, fakeBeatMidiValue);        
            //fakeSound.play();
            
            //log.write(EmergenceLog::fileDateTimeString(ofGetElapsedTimef()), viewers[0].getID(), "FB");
            
            
            // Grab a line from the log to accompany fake 
            recollection.loadRecollection(firstRecallBeat);
            firstRecallBeat = false;
            if (recollection.isBeating()) {
                recollection.play();
                midiout.sendControlChange(installationRateMidiChannel, fakeBeatRateMidiControlMsg,recollection.getRateMidiControlValue());
                
                if (cameraOn) {
                    // Grab a recall pic
                    screenTimeString = recollection.getScreenDateTimeString();
                    picPath = log.getImagesDirPath() + venue + ", " + location + ", " + recollection.getFileDateTimeString() + ".jpg";
                    pic.loadImage(picPath);
                    //printf("\n%s", picPath.c_str());
                }      
            }
        }
    }
}

//--------------------------------------------------------------
void testApp::update(){
    // Only do the most time-sensitive things in update
    
    //printf("\n lt= %f", ofGetElapsedTimef() - loopTime);
    //loopTime = ofGetElapsedTimef();
    
    // Parse incoming input from the serial stream
    // This may be changed to a multithreaded listener in the future
    // to improve latency
    //serial.parseInput();
    
    if (recollection.isBeating()) {
        if (recollection.queryPlay(ofGetElapsedTimef())) {
            midiout.sendControlChange(installationRateMidiChannel, fakeBeatRateMidiControlMsg,recollection.getRateMidiControlValue());

            if (cameraOn) {
                // Grab a recall pic
                screenTimeString = recollection.getScreenDateTimeString();
                picPath = log.getImagesDirPath() + venue + ", " + location + ", " + recollection.getFileDateTimeString() + ".jpg";
                pic.loadImage(picPath);
                //printf("\n%s", picPath.c_str());
            }
            
        }
    }
    

}

//--------------------------------------------------------------
void testApp::draw(){   
    // Do somewhat less time-sensitive things here
    //printf("\n lt= %f", ofGetElapsedTimef() - loopTime);
    //loopTime = ofGetElapsedTimef();

    if (!midiMapMode) {
        
            
        // TOUCH_BEGIN
        if (serial.getMessage() == serial.TOUCH_BEGIN) {
            // Set up Memory beats
            for (int i=0; i<NUM_MEMORIES; i++) {
                // If viewer has a valid heart rate
                if (viewers[i+1].getRate() >= viewers[0].getMinRate()) {
                    float ibi = 60.f / viewers[i+1].getRate(); // convert rate to seconds ibi
                    printf("\n ibi=%f", ibi);
                    // Set the inter-beat interval for memory beats
                    memorySounds[i].setInterBeatInterval(ibi);
                }
            }
        }
        
        // TOUCH_END
        if (serial.getMessage() == serial.TOUCH_END) {
            // Note that heartbeating has stopped
            heartBeating = false;
            
            /*
            // Send MIDI Off Notes
            if (viewerRateMidiOn) {
                midiout.sendNoteOff(viewerRateMidiChannel, viewerRateMidiId, viewerRateMidiValue);
                viewerRateMidiOn = false;
            }
            if (viewerAccelMidiOn) {
                midiout.sendNoteOff(viewerAccelMidiChannel, viewerAccelMidiId, viewerAccelMidiValue);
                viewerAccelMidiOn = false;
            }
            if (installationRateMidiOn) {
                midiout.sendNoteOff(installationRateMidiChannel, installationRateMidiId, installationRateMidiValue);
                installationRateMidiOn = false;
            }
            if (installationAccelMidiOn) {
                midiout.sendNoteOff(installationAccelMidiChannel, installationAccelMidiId, installationAccelMidiValue);
                installationAccelMidiOn = false;
            }
            if (touchLenMidiOn) {
                midiout.sendNoteOff(touchLenMidiChannel, touchLenMidiId, touchLenMidiValue);
                touchLenMidiOn = false;
            }
             */
        }
        
        // HEARTBEAT
        if (serial.getMessage() == serial.HEARTBEAT) {
             
            // Note that heartbeating has begun
            heartBeating = true;
            
            
            // **** Update MIDI Control Values **** //
            // If viewer heart rate has been calculated
            if (viewers[0].getRate() >= viewers[0].getMinRate()) {         
                viewerRateMidiControlValue = viewers[0].getRateMidiControlValue();
                //printf("\nvr=%i\n", viewerRateMidiControlValue);
            }
            // If viewer heart accel has been calculated
            if (viewers[0].getAccel() >= viewers[0].getMinAccel()) { 
                viewerAccelMidiControlValue = viewers[0].getAccelMidiControlValue(viewerAccelMidiMultiplier);
                //printf("\nva=%i\n", viewerAccelMidiControlValue);
            }
            // If installation heart rate has been calculated
            if (emergenceInstallation.getRate() >= emergenceInstallation.getMinRate()) { 
                installationRateMidiControlValue = emergenceInstallation.getRateMidiControlValue();
            }
            // If installation heart accel has been calculated
            if (emergenceInstallation.getAccel() >= emergenceInstallation.getMinAccel()) { 
                installationAccelMidiControlValue = emergenceInstallation.getAccelMidiControlValue(installationAccelMidiMultiplier);
            }
            // Get touchLen midi control value
            if (emergenceInstallation.getTouchLen(ofGetElapsedTimef()) > 0){ 
                touchLenMidiControlValue = (int) (touchLenMidiMultiplier * emergenceInstallation.getTouchLen(ofGetElapsedTimef()));
                // **** NEED A BETTER ALGORITHM TO CALCULATE touchLenMidiControlValue **** //
                touchLenMidiControlValue = touchLenMidiControlValue % 127; 
            }
            

             
            printf("\nvr=%i, va=%i, ir=%i, ia=%i, tl=%i\n", viewerRateMidiControlValue, viewerAccelMidiControlValue, installationRateMidiControlValue, installationAccelMidiControlValue, touchLenMidiControlValue);
            

            
            // **** Send MIDI Control Messages **** //
            // If we have valid viewer rate
            if (viewers[0].getRate() >= viewers[0].getMinRate()) {
                midiout.sendControlChange(viewerRateMidiChannel, viewerRateMidiControlMsg, viewerRateMidiControlValue);
            }
            // If we have valid viewer Accel
            if (viewers[0].getAccel() >= viewers[0].getMinAccel()) {
                midiout.sendControlChange(viewerAccelMidiChannel, viewerAccelMidiControlMsg, viewerAccelMidiControlValue);
            }
            // If we have valid installation rate
            if (emergenceInstallation.getRate() >= emergenceInstallation.getMinRate()) {
                midiout.sendControlChange(installationRateMidiChannel, installationRateMidiControlMsg, installationRateMidiControlValue);
            }
            // If we have valid installation Accel
            if (emergenceInstallation.getAccel() >= emergenceInstallation.getMinAccel()) {
                midiout.sendControlChange(installationAccelMidiChannel, installationAccelMidiControlMsg, installationAccelMidiControlValue);
            }
            // If we have valid touchLen
            if (emergenceInstallation.getTouchLen(ofGetElapsedTimef()) > 0) {
                midiout.sendControlChange(touchLenMidiChannel, touchLenMidiControlMsg, touchLenMidiControlValue);
            }
            
            /*
            // **** TURN ON MIDI CHANNELS **** //
            // Turn on viewerRateMidiChannel if not on and rate is valid
            if (!viewerRateMidiOn && (viewers[0].getRate() >= viewers[0].getMinRate())) {
                midiout.sendNoteOn(viewerRateMidiChannel, viewerRateMidiId, viewerRateMidiValue);
                viewerRateMidiOn = true;
            }
            // Turn on viewerAccelMidiChannel if not on and Accel is valid
            if (!viewerAccelMidiOn && (viewers[0].getAccel() >= viewers[0].getMinAccel())) {
                midiout.sendNoteOn(viewerAccelMidiChannel, viewerAccelMidiId, viewerAccelMidiValue);
                viewerAccelMidiOn = true;
            }
            // Turn on installationRateMidiChannel if not on and rate is valid
            if (!installationRateMidiOn && (emergenceInstallation.getRate() >= emergenceInstallation.getMinRate())) {
                midiout.sendNoteOn(installationRateMidiChannel, installationRateMidiId, installationRateMidiValue);
                installationRateMidiOn = true;
            }
            // Turn on installationAccelMidiChannel if not on and Accel is valid
            if (!installationAccelMidiOn && (emergenceInstallation.getAccel() >= emergenceInstallation.getMinAccel())) {
                midiout.sendNoteOn(installationAccelMidiChannel, installationAccelMidiId, installationAccelMidiValue);
                installationAccelMidiOn = true;
            }
            if (!touchLenMidiOn && (emergenceInstallation.getTouchLen(ofGetElapsedTimef()) > 0)) {
                midiout.sendNoteOn(touchLenMidiChannel, touchLenMidiId, touchLenMidiValue);
                touchLenMidiOn = true;        
            }
             */
            
            
            // Print debug info
            printf(" u=%i, n=%i, r=%f, a=%f", (int) viewers[0].getID(), viewers[0].getNumBeats(), viewers[0].getRate(), viewers[0].getAccel());
            
            // Save the picture from the webcam
            if (cameraOn) {
                pic.saveImage(log.getImagesDirPath() + venue + ", " + location + ", " +  fileTimeString + ".jpg");
            }
        }
        
        //ofSetColor(0,0,0);
        //ofRect(0.0f, 0.0f, 1280, 1024);
        ofBackground(0, 0, 0);
        
        // If we're picking up heartbeats or playing a recollection
        if (heartBeating || recollection.isBeating()) {
            
            // Draw the fake flickr background
            //ofSetColor(0,0,0);
            flickr.draw(112,32);
            
            // Overlay the webcam pic
            if (cameraOn) {
                pic.draw(112,100);
            }
            
            // Draw the picture header
            //string headerText = "Emergence Bio Art";
            //ofSetColor(255,255,255);
            //header.drawString( headerText.c_str(), 485.f, 80.f);  
            
            //Draw the picture header
            string headerText = "Emergence";
            ofSetColor(255,255,255);
            header.drawString( headerText.c_str(), 670.f, 80.f);  
            
            // Draw the picture title
            string titleText = venue + ", " + location  + ", " + screenTimeString;
            //printf("\n%s", titleText.c_str());
            ofSetColor(255,255,255);
            title.drawString( titleText.c_str(), 112.f, 730.f);  
            
            // If we're picking up heartbeats
            if (heartBeating) {
                // Play memory beats
                for (int i=0; i<NUM_MEMORIES; i++) {
                    //printf("\n ibimem=%f", memorySounds[i].getInterBeatInterval());
                    memorySounds[i].queryPlay(ofGetElapsedTimef());
                    //memorySounds[i].queryPlay(ofGetElapsedTimef(), midiout);
                }
            }
        } 
    }
    
    serial.clearMessage(); // Clear the serial message
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    // printf("\n+keyPressed = (%i)", key);
    if (((char) key) == '+') {
        midiMapMode = !midiMapMode;
    }
 
    if (((char) key) == '1') {
        midiout.sendControlChange(viewerRateMidiChannel, viewerRateMidiControlMsg, viewerRateMidiControlValue);
    } 
    if (((char) key) == '2') {
        midiout.sendControlChange(viewerAccelMidiChannel, viewerAccelMidiControlMsg, viewerAccelMidiControlValue);
    }
    if (((char) key) == '3') {
        midiout.sendControlChange(installationRateMidiChannel, installationRateMidiControlMsg, installationRateMidiControlValue);
    }
    if (((char) key) == '4') {
        midiout.sendControlChange(installationAccelMidiChannel, installationAccelMidiControlMsg, installationAccelMidiControlValue);
    }
    if (((char) key) == '5') {
        midiout.sendControlChange(touchLenMidiChannel, touchLenMidiControlMsg, touchLenMidiControlValue);
    }
    if (((char) key) == '6') {
        midiout.sendControlChange(fakeBeatMidiChannel, fakeBeatMidiControlMsg, 127);
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

