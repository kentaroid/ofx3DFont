#pragma once

#include "ofMain.h"
#include "ofx3DFont.h"

class testApp : public ofBaseApp{
public:
	void setup();
	void update();
	void draw();
		
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);


	wstring show;
	string strAlign;
	string strDirection;
	bool renderingMode;
	bool bRotation;
	int align;
	
	ofx3DFont face;
	ofx3DFont faceNavi3d;
	ofLight light;

};
