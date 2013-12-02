#pragma once

#include "ofxTrueTypeFontUL2.h"


class ofx3DFont{
public:
	ofx3DFont();
	virtual ~ofx3DFont();
	
	void setup(ofxTrueTypeFontUL2 &face,float fontSize,float depthRate,float smoothRad=30,bool reverseFace=false);
	void setup(string font,float fontSize,float depthRate,float smoothRad=30,bool reverseFace=false,float internalFontSize=256,float simplifyAmt=0.3);

	vector<ofMesh> getMeshes(string str, float x, float y,float z=0,float width=0,float height=0,int textAlign=UL2_TEXT_ALIGN_INVALID);
	vector<ofMesh> getMeshes(wstring str, float x, float y,float z=0,float width=0,float height=0,int textAlign=UL2_TEXT_ALIGN_INVALID);

    void drawString(string str, float x, float y,float z=0,float width=0,float height=0,int textAlign=UL2_TEXT_ALIGN_INVALID);
	void drawString(wstring str, float x, float y,float z=0,float width=0,float height=0,int textAlign=UL2_TEXT_ALIGN_INVALID);

	void drawFace(int index);
	void drawFaceAtPos(int index,float x,float y,float z=0);

	vector<ofxFaceVec2> getFacePositions(wstring str, float x, float y,float width=0,float height=0,int textAlign=UL2_TEXT_ALIGN_INVALID);
	vector<ofxFaceVec2> getFacePositions(string str, float x, float y,float width=0,float height=0,int textAlign=UL2_TEXT_ALIGN_INVALID);

	bool getUseVBO();
	void setUseVBO(bool useVBO);
private:

	ofxTrueTypeFontUL2 *mFontRender;
	ofxTrueTypeFontUL2 face;
	
	float mPushDepth;
	float mSmoothRad;
	float mFinalizeScale;
	float mInternalScale;
	bool  mReverseFace;

	void _testFace(int index);
	ofMesh pushMesh(ofPath &path);
	void triangulate(ofPath &path,ofMesh& mesh,vector<ofPolyline>&outline);

	template<class T>
	vector<ofMesh> _getMeshes(T str,float x, float y,float z,float width,float height,int textAlign);
	template<class T>
	void _drawString(T str,float x, float y,float z,float width,float height,int textAlign);
	template<class T>
	vector<ofxFaceVec2> _getFacePositions(T str, float x, float y,float width ,float height,int textAlign);

	
	map<int,ofMesh> meshMap;
	map<int,ofVbo> vboMap;
	map<int,GLuint> vboIndexNumMap;
	bool bUseVbo;
	
	ofTessellator mTessellator;

	// disallow copy and assign
	ofx3DFont(const ofx3DFont &);
	void operator=(const ofx3DFont &);


};

