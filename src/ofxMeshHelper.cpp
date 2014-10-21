#include "ofxMeshHelper.h"

//poly2tri:Used for make more beautiful triangulation.
#define TESSELLATOR_PORI2TRI
#ifdef  TESSELLATOR_PORI2TRI
#include "poly2tri.h"
#endif //  TESSELLATOR_PORI2TRI

ofTessellator ofxMeshHelper::tessellator;



inline bool isVertOnLine(const ofVec2f &v,const ofVec2f &v1,const ofVec2f &v2){
    if (v1.x > v2.x) {
		return v2.x <= v.x && v.x <= v1.x &&  ((v2.y <= v1.y && v2.y <= v.y && v.y <= v1.y) || (v2.y > v1.y && v1.y <= v.y && v.y <= v2.y))  && (v.y-v2.y)*(v1.x-v2.x) == (v1.y-v2.y)*(v.x-v2.x);
    }
    return v1.x <= v.x && v.x <= v2.x &&  ((v1.y <= v2.y && v1.y <= v.y && v.y <= v2.y) || (v1.y > v2.y && v2.y <= v.y && v.y <= v1.y))  && (v.y-v1.y)*(v2.x-v1.x) == (v2.y-v1.y)*(v.x-v1.x);
}

inline bool isVertNorm(const ofVec2f &start,const ofVec2f &end,const ofVec2f &position){
    float vx1 = end.x - start.x;
    float vy1 = end.y -start. y;
    float vx2 = position.x -start. x;
    float vy2 = position.y - start.y;
    return ( vx1 * vy2 - vy1 * vx2 ) < 0 ;
}

void ofxMeshHelper::triangulate(ofPath &path,ofMesh& mesh,vector<ofPolyline>&outline){
	path.close();

#ifndef  TESSELLATOR_PORI2TRI
	//Tesselete by ofTesseletor.
	outline=path.getOutline();
	mesh=path.getTessellation();

#else //  TESSELLATOR_PORI2TRI
	//Tesselete by poly2tri.
	//little otptimize for poly2tri.	
	tessellator.tessellateToPolylines( path.getOutline(), path.getWindingMode(), outline,true);
	
	int i,i2,t,t2,t3,t4,size,size2;
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
						float fv=FLT_MIN;
						v1=outline[i2][t2]-outline[i2][t3];
						t4=(t+1)==size?0:t+1;
						v1.normalize().rotate(isVertNorm(outline[i2][t2],outline[i2][t3],outline[i][t4])?90:-90);
						while(isVertOnLine(outline[i][t],outline[i2][t2],outline[i2][t3])){
							outline[i][t].set(outline[i][t]+v1*fv);
							fv*=2;
						}
					}
				}
			}
		}
	}
	
    //making stuructures.   (holes,OF_POLY_WINDING_ODD)
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

ofMesh ofxMeshHelper::pushMesh(ofPath &path,float pushDepth,float smoothRad,float scale,bool reverseFace,bool generateTexCoord,float coordScale){


	//float mPushDepth=getFontSize()*mDepthRate_;
    //float mFinalizeScale=mFontSize_/getFontSize();

	float coordRes=1.0/coordScale;
	ofMesh mesh;
	mesh.enableIndices();
	mesh.enableNormals();
	if(generateTexCoord)mesh.enableTextures();
	vector<ofPolyline> outline;

	triangulate(path,mesh,outline);

    const int numVrt=mesh.getNumVertices();
    const int numTri=mesh.getNumIndices()/3;
    
    ofVec2f pv,fv;int pi,fi;int sumV,numV;
    int frontPointer,backPointer;
    int i,t;

	//Vec for face.
	ofVec3f v1=mesh.getVertex(mesh.getIndex(0));
	ofVec3f v2=mesh.getVertex(mesh.getIndex(1));
	ofVec3f v3=mesh.getVertex(mesh.getIndex(2));
	ofVec3f v4 =(v2-v1).crossed(v3-v1);
	ofVec3f v5;

	bool velTopFace=v4.z<0;
	bool velSideFace=reverseFace;
	v1=outline[0][0];
	v2=outline[0][1];

	//Vec for side.
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

	if(reverseFace){
		velSideFace=!velSideFace;
		velTopFace=!velTopFace;
	}



    
	//front face normals
    for(i=0;i<numVrt;i++){mesh.addNormal(ofVec3f(0,0,1));}

	//front face Index
	if(!velTopFace){
		for(i=0;i<numTri;i++){
			t=mesh.getIndex(i*3+1);
			mesh.setIndex(i*3+1,mesh.getIndex(i*3+2));
			mesh.setIndex(i*3+2,t);
		}
	}


	
	//back face varts and normals
    for(i=0;i<numVrt;i++){
        mesh.addVertex(ofVec3f(mesh.getVertex(i).x,mesh.getVertex(i).y,-pushDepth));
        mesh.addNormal(ofVec3f(0,0,-1));
    }
	//back face Index
    for(i=0;i<numTri;i++){
        mesh.addTriangle(numVrt+mesh.getIndex(i*3),numVrt+mesh.getIndex(i*3+2),numVrt+mesh.getIndex(i*3+1));
    }

#if 1
	//generate texcoords(front and back)
	if(generateTexCoord){
		for(i=0;i<numVrt;i++){
			mesh.addTexCoord(mesh.getVertex(i)*coordRes);
		}
		for(i=0;i<numVrt;i++){
			mesh.addTexCoord(mesh.getVertex(i)*coordRes);
		}
	}
    
    for(t=0;t<outline.size();t++){
        sumV=0;
        numV=outline[t].size();
        frontPointer=mesh.getNumVertices();
		float dist=0.0f;
        for(i=0;i<numV;i++){
            pi=i==0?numV-1:i-1;
            fi=i==numV-1?0:i+1;
            pv=outline[t][pi]-outline[t][i];
            fv=outline[t][i]-outline[t][fi];
            pv.normalize();
            fv.normalize();
            if(abs(pv.angle(fv))>smoothRad){
                mesh.addVertex(ofVec3f(outline[t][i].x ,outline[t][i].y,0));
                mesh.addVertex(ofVec3f(outline[t][i].x ,outline[t][i].y,0));
                fv.rotate(rotateNorm);
                pv.rotate(rotateNorm);
                mesh.addNormal(ofVec3f(pv.x,pv.y,0));
                mesh.addNormal(ofVec3f(fv.x,fv.y,0));
				if(generateTexCoord){
					mesh.addTexCoord(ofVec2f(0,dist)*coordRes);
					mesh.addTexCoord(ofVec2f(0,dist)*coordRes);
				}
                sumV+=2;
            }else{
                pv=(pv+fv).normalized().rotated(rotateNorm);
                mesh.addVertex(ofVec3f(outline[t][i].x ,outline[t][i].y,0));
                mesh.addNormal(ofVec3f(pv.x,pv.y,0));
				if(generateTexCoord){
					mesh.addTexCoord(ofVec2f(0,dist)*coordRes);
				}
                sumV++;
            }
			fv=outline[t][fi]-outline[t][i];
			dist+=fv.length();
        }

		if(generateTexCoord){
			mesh.addVertex(mesh.getVertex(frontPointer));
            mesh.addNormal(mesh.getNormal(frontPointer));
			mesh.addTexCoord(ofVec2f(0,dist)*coordRes);
		}

        backPointer=mesh.getNumVertices();

        for(i=0;i<sumV;i++){
            mesh.addVertex(ofVec3f(mesh.getVertex(frontPointer+i).x ,mesh.getVertex(frontPointer+i).y,-pushDepth));
            mesh.addNormal(mesh.getNormal(frontPointer+i));
			if(generateTexCoord){
				mesh.addTexCoord(mesh.getTexCoord(frontPointer+i)+ofVec2f(pushDepth,0.0)*coordRes);
			}
        }
		if(generateTexCoord){
			mesh.addVertex(mesh.getVertex(backPointer));
            mesh.addNormal(mesh.getNormal(backPointer));
			mesh.addTexCoord(ofVec2f(0,dist)*coordRes+ofVec2f(pushDepth,0.0)*coordRes);
		}


        for(i=0;i<sumV;i++){
			if(generateTexCoord){
				fi=i+1;
			}else{
				fi=sumV-1==i?0:i+1;
			}
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
#else
	//generate texcoords(front and back)
	if(generateTexCoord){
		for(i=0;i<numVrt;i++){
			mesh.addTexCoord(mesh.getVertex(i)*coordRes);
		}
		for(i=0;i<numVrt;i++){
			mesh.addTexCoord(mesh.getVertex(i)*coordRes+ofVec2f(coordRes,coordRes));
		}
	}
    
    for(t=0;t<outline.size();t++){
        sumV=0;
        numV=outline[t].size();
        frontPointer=mesh.getNumVertices();

        for(i=0;i<numV;i++){
            pi=i==0?numV-1:i-1;
            fi=i==numV-1?0:i+1;
            pv=outline[t][pi]-outline[t][i];
            fv=outline[t][i]-outline[t][fi];
            pv.normalize();
            fv.normalize();
            if(abs(pv.angle(fv))>smoothRad){
                mesh.addVertex(ofVec3f(outline[t][i].x ,outline[t][i].y,0));
                mesh.addVertex(ofVec3f(outline[t][i].x ,outline[t][i].y,0));
                fv.rotate(rotateNorm);
                pv.rotate(rotateNorm);
                mesh.addNormal(ofVec3f(pv.x,pv.y,0));
                mesh.addNormal(ofVec3f(fv.x,fv.y,0));
				if(generateTexCoord){
					mesh.addTexCoord(ofVec2f(outline[t][i].x ,outline[t][i].y)*coordRes);
					mesh.addTexCoord(ofVec2f(outline[t][i].x ,outline[t][i].y)*coordRes);
				}
                sumV+=2;
            }else{
                pv=(pv+fv).normalized().rotated(rotateNorm);
                mesh.addVertex(ofVec3f(outline[t][i].x ,outline[t][i].y,0));
                mesh.addNormal(ofVec3f(pv.x,pv.y,0));
				if(generateTexCoord){
					mesh.addTexCoord(ofVec2f(outline[t][i].x ,outline[t][i].y)*coordRes);
				}
                sumV++;
            }
        }

        backPointer=mesh.getNumVertices();

        for(i=0;i<sumV;i++){
            mesh.addVertex(ofVec3f(mesh.getVertex(frontPointer+i).x ,mesh.getVertex(frontPointer+i).y,-pushDepth));
            mesh.addNormal(mesh.getNormal(frontPointer+i));
			if(generateTexCoord){
				mesh.addTexCoord(mesh.getTexCoord(frontPointer+i)+ofVec2f(coordRes,coordRes));
			}
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
#endif
    if(scale!=1.0f){
		sumV=mesh.getNumVertices();
        for(int i=0;i<sumV;i++){
            mesh.setVertex(i,mesh.getVertex(i)*scale);
        }
	}
	return mesh;
}

