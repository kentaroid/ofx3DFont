#include "ofx3DFont.h"

ofx3DFont::ofx3DFont(void){
    bUseVbo=true;
    mSmoothRad=30;
    mReverseFace=false;
	mFontSize_=64.0f;
	mDepthRate_=0.3f;
	mGenerateTexCoord=false;
	mCoordDetailScale=1.0f;
}

ofx3DFont::~ofx3DFont(void){
}

void ofx3DFont::setup(string font,float fontSize,float depthRate, float smoothRad,bool reverseFace){
	//In this way, internal font size is set to 128. Be cafeful to treat positions that is generated ofxTrueTypeFontUL2.  
    loadFont(font,fontSize,true,true,0.3f,0,false);
    setup(fontSize,depthRate,smoothRad,mReverseFace);
}

void ofx3DFont::setup(float fontSize,float depthRate,float smoothRad,bool reverseFace){
    mSmoothRad=smoothRad;
    mReverseFace=reverseFace;
	mFontSize_=fontSize;
	mDepthRate_=depthRate;
}


void ofx3DFont::_testFace(int index){
    if(bUseVbo){
        if(vboMap.count(index) == 0){
            if (meshMap.count(index) == 0){
                ofPath p=getCountours(index);
				meshMap[index]=ofxMeshHelper::pushMesh(p,
					getFontSize()*mDepthRate_,
					mSmoothRad,
					mFontSize_/getFontSize(),
					mReverseFace,
					mGenerateTexCoord,
					getFontSize()*mCoordDetailScale
					);
            }
            const ofMesh &mesh = meshMap[index];
            ofVbo vbo ;
            vbo.disableColors();
            vbo.disableTexCoords();
            vbo.setVertexData(mesh.getVerticesPointer(),mesh.getNumVertices(),GL_STATIC_DRAW);
            vbo.setNormalData(mesh.getNormalsPointer(),mesh.getNumNormals(),GL_STATIC_DRAW);
            vbo.setIndexData(mesh.getIndexPointer(), mesh.getNumIndices(), GL_STATIC_DRAW);
            vboIndexNumMap[index]=mesh.getNumIndices();
            vboMap[index]=vbo;
        }
    }else{
        if (meshMap.count(index) == 0){
            ofPath p=getCountours(index);
            meshMap[index]=ofxMeshHelper::pushMesh(p,
					getFontSize()*mDepthRate_,
					mSmoothRad,
					mFontSize_/getFontSize(),
					mReverseFace,
					mGenerateTexCoord,
					getFontSize()*mCoordDetailScale
					);
        }
    }
}

void ofx3DFont::drawFace(int index){
    if(bUseVbo){
        vboMap[index].drawElements(GL_TRIANGLES,vboIndexNumMap[index]);
    }else{
        meshMap[index].draw();
    }
}

ofMesh& ofx3DFont::getMesh(int index){
	return meshMap[index];

}

void ofx3DFont::drawFaceAtPos(int index,float x,float y,float z){
    glPushMatrix();
    glTranslatef(x,y,z);
    drawFace(index);
    glPopMatrix();
}

template<class T>
vector<ofMesh> ofx3DFont::_getMeshes(T str,float x, float y, float z,float width,float height,int textAlign){
    const vector<ofxFaceVec2> &facePosis=_getFacePositions(str,x,y,width,height,textAlign);
    vector<ofMesh> meshes;
    for(vector<ofxFaceVec2>::const_iterator itIn=facePosis.begin();itIn!=facePosis.end();++itIn){
        ofMesh mesh=meshMap[(*itIn).faceIndex];//copy
        const int num=mesh.getNumVertices();
        for(int ii=0;ii<num;ii++){
            const ofVec3f &v = mesh.getVertex(ii);
            mesh.setVertex(ii,ofVec3f(v.x+(*itIn).x,v.y+(*itIn).y,v.z+z));
        }
        meshes.push_back(mesh);
    }
    return meshes;
}

template<class T>
void ofx3DFont::_draw3dString(T str,float x, float y,float z,float width,float height,int textAlign){
    const vector<ofxFaceVec2> &facePosis=_getFacePositions(str,x,y,width,height,textAlign);
    for(vector<ofxFaceVec2>::const_iterator itIn=facePosis.begin();itIn!=facePosis.end();++itIn){
        drawFaceAtPos((*itIn).faceIndex,(*itIn).x,(*itIn).y,z);
    }
}

template<class T>
vector<ofxFaceVec2> ofx3DFont::_getFacePositions(T str, float x, float y,float width ,float height,int textAlign){
    vector<ofxFaceVec2> facePosis;
    float mFinalizeScale=mFontSize_/getFontSize();
    float mInternalScale=1.0f/mFinalizeScale;
    getLayoutData(facePosis,str,x*mInternalScale,y*mInternalScale,width*mInternalScale,height*mInternalScale,textAlign);
    for(vector<ofxFaceVec2>::iterator itIn=facePosis.begin();itIn!=facePosis.end();++itIn){
        _testFace((*itIn).faceIndex);
        (*itIn).x*=mFinalizeScale;
        (*itIn).y*=mFinalizeScale;
    }
    return facePosis;
}



//////////////////////////// INTERFACE ///////////////////////////

vector<ofMesh> ofx3DFont::getMeshes(string str,float x, float y,float z,float width,float height,int textAlign){
    return _getMeshes(str, x,  y, z,width, height, textAlign);
}
vector<ofMesh> ofx3DFont::getMeshes(wstring str,float x, float y,float z,float width,float height,int textAlign){
    return _getMeshes(str, x,  y, z,width, height, textAlign);
}

void ofx3DFont::draw3dString(string str, float x, float y,float z,float width,float height,int textAlign){
    _draw3dString(str, x,  y,z, width, height, textAlign);
}
void ofx3DFont::draw3dString(wstring str, float x, float y,float z,float width,float height,int textAlign){
    _draw3dString(str, x,  y,z, width, height, textAlign);
}

vector<ofxFaceVec2> ofx3DFont::getFacePositions(wstring str, float x, float y,float width,float height,int textAlign){
    return _getFacePositions(str,x,y,width,height,textAlign);
}
vector<ofxFaceVec2> ofx3DFont::getFacePositions(string str, float x, float y,float width,float height,int textAlign){
    return _getFacePositions(str,x,y,width,height,textAlign);
}

bool ofx3DFont::getUseVBO(){
	return bUseVbo;
}
void ofx3DFont::setUseVBO(bool useVBO){
	bUseVbo=useVBO;
}

bool ofx3DFont::getGenerateTexCoord(){
	return mGenerateTexCoord;
}
void ofx3DFont::setGenerateTexCoord(bool generateTexCoord,float detailScale){
	mGenerateTexCoord=generateTexCoord;
	mCoordDetailScale=detailScale;
}