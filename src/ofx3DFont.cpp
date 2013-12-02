#include "ofx3DFont.h"

//poly2tri:Used for make more beautiful triangulation.
#define TESSELLATOR_PORI2TRI
#ifdef  TESSELLATOR_PORI2TRI
#include "poly2tri.h"
#endif //  TESSELLATOR_PORI2TRI


ofx3DFont::ofx3DFont(void){
    bUseVbo=true;
}


ofx3DFont::~ofx3DFont(void){
}


void ofx3DFont::setup(string font,float fontSize,float depthRate, float smoothRad,bool reverseFace,float internalFontSize,float simplifyAmt){
    //simplifyAmtは長さではなく角度的な範囲。
    //一定のフォントサイズを保っていればsimplifyAmtの結果はおなじになる。
    //文字のDetailを決定できるのはあくまでfontSizeが有効となる。
    face.loadFont(font,internalFontSize,true,true,simplifyAmt,0,false);
    setup(face,fontSize,depthRate,smoothRad,mReverseFace);
}
void ofx3DFont::setup(ofxTrueTypeFontUL2 &face,float fontSize,float depthRate,float smoothRad,bool reverseFace){
    if(face.getFontSize()<10.0f)ofLogNotice("ofx3DFont") << "[setup] font size is too small. There may be destroyed font face.";
    mFontRender=&face;
    mPushDepth=mFontRender->getFontSize()*depthRate;
    mSmoothRad=smoothRad;
    mReverseFace=reverseFace;
    mFinalizeScale=fontSize/mFontRender->getFontSize();
    mInternalScale=1.0f/mFinalizeScale;
}

void ofx3DFont::_testFace(int index){
    if(bUseVbo){
        if(vboMap.count(index) == 0){
            if (meshMap.count(index) == 0){
                ofPath p=mFontRender->getCountours(index);
                meshMap[index]=pushMesh(p);
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
            ofPath p=mFontRender->getCountours(index);
            meshMap[index]=pushMesh(p);
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
void ofx3DFont::_drawString(T str,float x, float y,float z,float width,float height,int textAlign){
    const vector<ofxFaceVec2> &facePosis=_getFacePositions(str,x,y,width,height,textAlign);
    for(vector<ofxFaceVec2>::const_iterator itIn=facePosis.begin();itIn!=facePosis.end();++itIn){
        drawFaceAtPos((*itIn).faceIndex,(*itIn).x,(*itIn).y,z);
    }
}

template<class T>
vector<ofxFaceVec2> ofx3DFont::_getFacePositions(T str, float x, float y,float width ,float height,int textAlign){
    vector<ofxFaceVec2> facePosis;
    mFontRender->getLayoutData(facePosis,str,x*mInternalScale,y*mInternalScale,width*mInternalScale,height*mInternalScale,textAlign);
    for(vector<ofxFaceVec2>::iterator itIn=facePosis.begin();itIn!=facePosis.end();++itIn){
        _testFace((*itIn).faceIndex);
        (*itIn).x*=mFinalizeScale;
        (*itIn).y*=mFinalizeScale;
    }
    return facePosis;
}







ofMesh ofx3DFont::pushMesh(ofPath &path){
	ofMesh mesh;
	vector<ofPolyline> outline;

	triangulate(path,mesh,outline);

    const int numVrt=mesh.getNumVertices();
    const int numTri=mesh.getNumIndices()/3;
    
    ofVec2f pv,fv;int pi,fi;int sumV,numV;
    int frontPointer,backPointer;
    int i,t;

	ofVec3f v1=mesh.getVertex(mesh.getIndex(0));
	ofVec3f v2=mesh.getVertex(mesh.getIndex(1));
	ofVec3f v3=mesh.getVertex(mesh.getIndex(2));
	ofVec3f v4 =(v2-v1).crossed(v3-v1);
	bool velTopFace=v4.z<0;

	bool velSideFace=mReverseFace;
	v1=outline[0][0];
	v2=outline[0][1];
	ofVec3f v5;
	//側面の向きを計算
	for(i=0;i<numTri;i++){
		v3=mesh.getVertex(mesh.getIndex(i*3));
		v4=mesh.getVertex(mesh.getIndex(i*3+1));
		v5=mesh.getVertex(mesh.getIndex(i*3+2));
        if(v3==v1){
			if(v4==v2){
				velSideFace=velTopFace;
				break;		
			}else if(v5==v2){
                velSideFace=!velTopFace;
				break;
			}
		}else if(v4==v1){
			if(v5==v2){
				velSideFace=velTopFace;
				break;
			}else if(v3==v2){
                velSideFace=!velTopFace;
				break;
			}
		}else if(v5==v1){
			if(v3==v2){
				velSideFace=velTopFace;
				break;
			}else if(v4==v2){
                velSideFace=!velTopFace;
				break;
			}
		}
	}

	const float rotateNorm = velSideFace ? -90:90;

	if(mReverseFace){
		velSideFace=!velSideFace;
		velTopFace=!velTopFace;
	}
    
	//表面形成
    for(i=0;i<numVrt;i++){mesh.addNormal(ofVec3f(0,0,1));}

	//表Index
	if(!velTopFace){
		for(i=0;i<numTri;i++){
			t=mesh.getIndex(i*3+1);
			mesh.setIndex(i*3+1,mesh.getIndex(i*3+2));
			mesh.setIndex(i*3+2,t);
		}
	}

	//裏面頂点
    for(i=0;i<numVrt;i++){
        mesh.addVertex(ofVec3f(mesh.getVertex(i).x,mesh.getVertex(i).y,-mPushDepth));
        mesh.addNormal(ofVec3f(0,0,-1));
    }
	//裏面Index
    for(i=0;i<numTri;i++){
        mesh.addTriangle(numVrt+mesh.getIndex(i*3),numVrt+mesh.getIndex(i*3+2),numVrt+mesh.getIndex(i*3+1));
    }
    
    for(t=0;t<outline.size();t++){
        sumV=0;
        numV=outline[t].size();
        frontPointer=mesh.getNumVertices();
        for(i=0;i<numV;i++){
            pi=i==0?numV-1:i-1;
            fi=i==numV-1?0:i+1;
            pv.x=outline[t][pi].x-outline[t][i].x;
            pv.y=outline[t][pi].y-outline[t][i].y;
            fv.x=outline[t][i].x-outline[t][fi].x;
            fv.y=outline[t][i].y-outline[t][fi].y;
            pv.normalize();
            fv.normalize();
            if(abs(pv.angle(fv))>mSmoothRad){
                mesh.addVertex(ofVec3f(outline[t][i].x ,outline[t][i].y,0));
                mesh.addVertex(ofVec3f(outline[t][i].x ,outline[t][i].y,0));
                fv.rotate(rotateNorm);
                pv.rotate(rotateNorm);
                mesh.addNormal(ofVec3f(pv.x,pv.y,0));
                mesh.addNormal(ofVec3f(fv.x,fv.y,0));
                sumV+=2;
            }else{
                pv=(pv+fv).normalized().rotated(rotateNorm);
                mesh.addVertex(ofVec3f(outline[t][i].x ,outline[t][i].y,0));
                mesh.addNormal(ofVec3f(pv.x,pv.y,0));
                sumV++;
            }
        }
        
        backPointer=mesh.getNumVertices();
        for(i=0;i<sumV;i++){
            mesh.addVertex(ofVec3f(mesh.getVertex(frontPointer+i).x ,mesh.getVertex(frontPointer+i).y,-mPushDepth));
            mesh.addNormal(mesh.getNormal(frontPointer+i));
        }
        for(i=0;i<sumV;i++){
            fi=sumV-1==i?0:i+1;
            if(mesh.getVertex(frontPointer+i)==mesh.getVertex(frontPointer+fi))continue;
            if(velSideFace){
                mesh.addTriangle(frontPointer+i,backPointer+i,frontPointer+fi);
                mesh.addTriangle(backPointer+i,backPointer+fi,frontPointer+fi);
            }else{
                mesh.addTriangle(frontPointer+i,frontPointer+fi,backPointer+i);
                mesh.addTriangle(backPointer+i,frontPointer+fi,backPointer+fi);
            }
        }
    }
    
    if(mFinalizeScale!=1.0f){
		sumV=mesh.getNumVertices();
        for(int i=0;i<sumV;i++){
            mesh.setVertex(i,mesh.getVertex(i)*mFinalizeScale);
        }
	}
	return mesh;
}

inline bool isVertOnLine(const ofVec2f &v,const ofVec2f &v1,const ofVec2f &v2){
    if (v1.x > v2.x) {
		return v2.x <= v.x && v.x <= v1.x &&  ((v2.y <= v1.y && v2.y <= v.y && v.y <= v1.y) || (v2.y > v1.y && v1.y <= v.y && v.y <= v2.y))  && (v.y-v2.y)*(v1.x-v2.x) == (v1.y-v2.y)*(v.x-v2.x);
    }
    return v1.x <= v.x && v.x <= v2.x &&  ((v1.y <= v2.y && v1.y <= v.y && v.y <= v2.y) || (v1.y > v2.y && v2.y <= v.y && v.y <= v1.y))  && (v.y-v1.y)*(v2.x-v1.x) == (v2.y-v1.y)*(v.x-v1.x);
}

void ofx3DFont::triangulate(ofPath &path,ofMesh& mesh,vector<ofPolyline>&outline){


#ifndef  TESSELLATOR_PORI2TRI
	//Tesselete by ofTesseletor.
	outline=path.getOutline();
	mesh=path.getTessellation();

#else //  TESSELLATOR_PORI2TRI
	//Tesselete by poly2tri.
	//little otptimize for poly2tri.
	mTessellator.tessellateToPolylines( path.getOutline(), path.getWindingMode(), outline);

	int i,i2,t,t2,t3,size,size2;
	ofVec2f v1,v2,v3;
	for(i=0;i<outline.size();i++){
		size=outline[i].size();
		for(i2=0;i2<outline.size();i2++){
			size2=outline[i2].size();
			for(int t=0;t<size;t++){
				for(int t2=0;t2<size2;t2++){
					t3=(t2+1)==size2?0:t2+1;
					if(t==t2||t==t3)continue;
					if(isVertOnLine(outline[i][t],outline[i2][t2],outline[i2][t3])){
						ofLogNotice("ofx3DFont")<< "triangulate::having same psition.";
						mesh=path.getTessellation();
						return;
					}
				}
			}
		}
	}

    //入れ子(hole)の構造を取得 (OF_POLY_WINDING_ODDである前提)
    vector<int> pStructures;
	vector<bool>hools;
	int parentID;
	bool hool;

    for(i=0;i<outline.size();i++){
        parentID=-1;
		hool=false;
        for(t=0;t<outline.size();t++){
            if(i==t)continue;
			if(outline[t].inside(outline[i][0])){
				if(parentID>-1){
					if(outline[parentID].inside(outline[t][0])){
						parentID=t;
					}
				}else{
					parentID=t;
				}
				hool=!hool;
            }
        }
		hools.push_back(hool);
        pStructures.push_back(parentID);
    }

	///making face
	///////////////////////////////////////////////
    for(i=0;i<outline.size();i++){
        for(t=0;t<outline[i].size();t++){
            mesh.addVertex(outline[i][t]);
        }
    }
    
    for(i=0;i<outline.size();i++){
        if(hools[i])continue;
        //add outlines		
        vector<p2t::Point*>pOutLine;
        for(t=0;t<outline[i].size();t++){
            pOutLine.push_back(new p2t::Point(outline[i][t].x,outline[i][t].y));
        }
        p2t::CDT* cdt = new p2t::CDT(pOutLine);

        //add holes
        for(i2=0;i2<outline.size();i2++){
            if(i!=pStructures[i2])continue;
            vector<p2t::Point*>pHole;
            for(int t=0;t<outline[i2].size();t++){
                pHole.push_back(new p2t::Point(outline[i2][t].x,outline[i2][t].y));
            }
            cdt->AddHole(pHole);
        }

        ///// Triangulate!!
		cdt->Triangulate();
		/////

        vector<p2t::Triangle*> triangles = cdt->GetTriangles();
        for(i2=0;i2<triangles.size();i2++){
            for (t=2;t>-1;t--){
                p2t::Point& a = *triangles[i2]->GetPoint(t);
                for(t2=0;t2<mesh.getNumVertices();t2++){
                    if(a.x==mesh.getVertex(t2).x&&a.y==mesh.getVertex(t2).y){
                        mesh.addIndex(t2);
                        break;
                    }
                }
            }
        }
        delete cdt;
    } 
    pStructures.clear(); 
	hools.clear();

#endif //  TESSELLATOR_PORI2TRI
}



//////////////////////////// INTERFACE ///////////////////////////

vector<ofMesh> ofx3DFont::getMeshes(string str,float x, float y,float z,float width,float height,int textAlign){
    return _getMeshes(str, x,  y, z,width, height, textAlign);
}
vector<ofMesh> ofx3DFont::getMeshes(wstring str,float x, float y,float z,float width,float height,int textAlign){
    return _getMeshes(str, x,  y, z,width, height, textAlign);
}

void ofx3DFont::drawString(string str, float x, float y,float z,float width,float height,int textAlign){
    _drawString(str, x,  y,z, width, height, textAlign);
}
void ofx3DFont::drawString(wstring str, float x, float y,float z,float width,float height,int textAlign){
    _drawString(str, x,  y,z, width, height, textAlign);
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
