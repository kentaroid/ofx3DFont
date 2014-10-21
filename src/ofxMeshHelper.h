#pragma once
#include "ofMesh.h"
#include "ofPath.h"

class ofxMeshHelper
{
public:
	static ofTessellator tessellator;
	static ofMesh pushMesh(ofPath &path,float pushDepth,float smoothRad,float scale,bool reverseFace,bool generateTexCoord,float coordScale);
	static void triangulate(ofPath &path,ofMesh& mesh,vector<ofPolyline>&outline);


};

