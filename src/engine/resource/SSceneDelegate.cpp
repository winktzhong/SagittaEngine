//////////////////////////////////////////////////////
// file: SSceneDelegate.cpp 
// created by zhangxiang on 09-01-09
// define of the class SSceneDelegate
// SSceneDelegate is a class ...
//////////////////////////////////////////////////////

// INCLUDES //////////////////////////////////////////
#include "SSceneDelegate.h"
#include "../scenegraph/SScene.h"
#include "../scenegraph/SSceneNode.h"
#include "../scenegraph/SEsse.h"
#include "../scenegraph/SEntity.h"
#include "../scenegraph/SSimpleEntities.h"
#include "../animation/SMotionClip.h"
#include "../animation/SAnimaJoint.h"
#include "../buffer/SVertexData.h"
#include "../buffer/SVertexBufferElement.h"
#include "../buffer/SVertexIndexBuffer.h"
#include "../io/SFile.h"
#include "../../common/utils/SString.h"
#include "../../common/utils/SException.h"


// DECLARES //////////////////////////////////////////

// DEFINES ///////////////////////////////////////////
namespace Sagitta{

	// SSceneDelegate //////////////////////////////////////////////////////////////////////////

	//  [1/9/2009 zhangxiang]
	void SSceneDelegate::loadImpl(const StdString &aFileName, SResource *outResource){
		// test file type
		size_t lastslash = aFileName.find_last_of("\\/");
		size_t lastdot = aFileName.find_last_of(".");

		if(lastdot != StdString::npos){
			StdString postfix = aFileName.substr(lastdot);
			if(postfix == ".obj"){
				// .obj file
				loadScene_obj(aFileName, outResource);
			}else{
				THROW_SAGI_EXCEPT(SException::ERR_NOT_IMPLEMENTED,
					postfix + " is not a supported file type.",
					"SSceneDelegate::loadImpl");
			}
		}else{
			THROW_SAGI_EXCEPT(SException::ERR_NOT_IMPLEMENTED,
				"File type without postfix is supported.",
				"SSceneDelegate::loadImpl");
		}
	}

	//  [1/9/2009 zhangxiang]
	void SSceneDelegate::loadScene_obj(const StdString &aFileName, SResource *outResource){
		SFile file(aFileName);
		if(!file){
			THROW_SAGI_EXCEPT(SException::ERR_INTERNAL_ERROR,
				"Cannot open file \"" + aFileName + "\".",
				"SSceneDelegate::loadScene_obj");
		}
		// create a node to be the root node for the adding scene
		size_t lastslash = aFileName.find_last_of("\\/");
		size_t lastdot = aFileName.find_last_of(".");

		SScene *scene = static_cast<SScene*>(outResource);
		SSceneNode *sceneRoot;
		StdString sceneName;
		if(lastslash == StdString::npos){
			sceneName = aFileName.substr(0, lastdot);
			sceneRoot = scene->createSceneNode(sceneName + "Node");
		}else{
			sceneName = aFileName.substr(lastslash + 1, lastdot);
			sceneRoot = scene->createSceneNode(sceneName + "Node");
		}

		size_t gCount = 0;			// group count
		size_t gVCount = 0;			// current group vertex count
		size_t gTCount = 0;			// current group texcoord count
		size_t gNCount = 0;			// current group normal count
		size_t gFCount = 0;			// current group face count
		StdString gName = "";		// current group name

		// Now assume triangle base scene,
		// and assume using Vector3 to express a vertex, and a normal
		// and using Vector2 to express a texcoord.
		// if this assumes are not support, throw exception.
		// for future to support more types.
		// for future ...
		uInt polyType = 3;

		std::vector<size_t> vcounts;
		std::vector<size_t> tcounts;
		std::vector<size_t> ncounts;
		std::vector<size_t> fcounts;
		std::vector<SEntity*> groups;
		vcounts.reserve(7);
		tcounts.reserve(7);
		ncounts.reserve(7);
		groups.reserve(7);

		// store the state
		// 1 - in v, 2 - in face
		int state = 2;

		// for get line
		std::vector<StdString> words;

		// first pass begin //////////////////////////////////////////////////////////////////////////
		// go through the file to 
		// gets all the statistics of the model
		// (such as #vertices, #normals, etc)
		char c;
		while(file >> c){
			switch(c){
				case '#':		// comment
					{
						// eat up rest of line
						file.getLine(0);
						break;
					}

				case 'v':		// v, vn, vt
					{
						if(state != 1){
							// start a new group
							// first take charge latest group
							if(gFCount != 0){
								// have a latest group
								if(gName.empty())
									gName = sceneName + SString::to_string(static_cast<uInt>(gCount));

								bool bsmooth = true;
								if(gNCount != 0 && gVCount != gNCount){
									bsmooth = false;
								}
								// assume counterclockwise
								SEntity *entity = new SEntity(gName, polyType, gVCount, gFCount, bsmooth);
								entity->renderOption().setRenderElementType(SRenderOption::RET_TRIANGLES);
								SSceneNode *node = scene->createSceneNode(gName + "_node");
								node->stepToParent(sceneRoot);
								node->attachEsse(entity);
								groups.push_back(entity);
								// init vertex and index data
								SVertexData *vdata = entity->getVertexData();
								SIndexData *idata = entity->getIndexData();
								if(gVCount > 0){
									vdata->createElement(SVertexBufferElement::ET_VERTEX, 3, gVCount);
									idata->createElement(SVertexBufferElement::ET_VERTEX);
								}
								if(gNCount > 0){
									vdata->createElement(SVertexBufferElement::ET_NORMAL, 3, gNCount);
									idata->createElement(SVertexBufferElement::ET_NORMAL);
								}
								if(gTCount > 0){
									vdata->createElement(SVertexBufferElement::ET_TEXTURE_COORD, 2, gTCount);
									idata->createElement(SVertexBufferElement::ET_TEXTURE_COORD);
								}

								vcounts.push_back(gVCount);
								tcounts.push_back(gTCount);
								ncounts.push_back(gNCount);
								fcounts.push_back(gFCount);

								gCount++;

								gVCount = 0;
								gNCount = 0;
								gTCount = 0;
								gFCount = 0;

								gName.clear();
							}
							// now we are in 'v' area
							state = 1;
						}

						c = file.get();
						switch(c){
				case ' ':	// vertex
					{
						++gVCount;
						// eat up rest of line
						file.getLine(0);
						break;
					}

				case 'n':	// normal
					{
						++gNCount;
						// eat up rest of line
						file.getLine(0);
						break;
					}

				case 't':	// texcoord
					{
						++gTCount;
						// eat up rest of line
						file.getLine(0);
						break;
					}

				default:	// unknown
					{
						file.close();
						THROW_SAGI_EXCEPT(SException::ERR_INVALIDPARAMS,
							"Unknown token after \"v\".",
							"SSceneDelegate::loadScene_obj");
					}
						}

						break;
					} //#### end v switch

				case 'm':		// mtl file
					{
						// read mtl file
						// for future ...
						break;
					}

				case 'u':		// nothing
					{
						// eat up rest of line
						file.getLine(0);
						break;
					}

				case 'g':		// group
					{
						if(state == 1){
							// next into 'f' area
							file.getLine(&words);
							if(!words.empty()){
								gName = words[0];
							}
						}else{
							// eat up rest of line
							file.getLine(0);
						}

						break;
					}

				case 'f':		// face
					{
						if(state != 2){
							// start a group
							state = 2;
						}
						++gFCount;
						// eat up rest of line
						file.getLine(0);
						break;
					}

				default:
					{
						// eat up rest of line
						file.getLine(0);
						break;
					}

			}
		}

		// take charge the last group
		if(gFCount != 0){
			// have a last group
			if(gName.empty())
				gName = sceneName + SString::to_string(static_cast<uInt>(gCount));

			bool bsmooth = true;
			if(gNCount != 0 && gVCount != gNCount){
				bsmooth = false;
			}
			// assume counterclockwise
			SEntity *entity = new SEntity(gName, polyType, gVCount, gFCount, bsmooth);
			entity->renderOption().setRenderElementType(SRenderOption::RET_TRIANGLES);
			SSceneNode *node = scene->createSceneNode(gName + "_node");
			node->stepToParent(sceneRoot);
			node->attachEsse(entity);
			groups.push_back(entity);
			// init vertex and index data
			SVertexData *vdata = entity->getVertexData();
			SIndexData *idata = entity->getIndexData();
			if(gVCount > 0){
				vdata->createElement(SVertexBufferElement::ET_VERTEX, 3, gVCount);
				idata->createElement(SVertexBufferElement::ET_VERTEX);
			}
			if(gNCount > 0){
				vdata->createElement(SVertexBufferElement::ET_NORMAL, 3, gNCount);
				idata->createElement(SVertexBufferElement::ET_NORMAL);
			}
			if(gTCount > 0){
				vdata->createElement(SVertexBufferElement::ET_TEXTURE_COORD, 2, gTCount);
				idata->createElement(SVertexBufferElement::ET_TEXTURE_COORD);
			}

			vcounts.push_back(gVCount);
			tcounts.push_back(gTCount);
			ncounts.push_back(gNCount);
			fcounts.push_back(gFCount);

			gCount++;


			gVCount = 0;
			gNCount = 0;
			gTCount = 0;
			gFCount = 0;

			gName.clear();
		}
		// first pass end //////////////////////////////////////////////////////////////////////////

		// second pass begin //////////////////////////////////////////////////////////////////////////
		// read and store data
		size_t tVCount = 0;			// total vertex count till current
		size_t tTCount = 0;			// total texcoord count till current
		size_t tNCount = 0;			// total normal count till current
		gCount = 0;
		gVCount = 0;
		gNCount = 0;
		gTCount = 0;
		gFCount = 0;
		state = 0;

		// store datas
		Vector3 *vertices = 0;
		Vector3 *normals = 0;
		Vector2 *texcoords = 0;
		IndexedTriangle *vindices = 0;
		IndexedTriangle *nindices = 0;
		IndexedTriangle *tindices = 0;

		Real x, y, z;
		int v, n, t;

		if(file.eof()){
			file.clear();
		}
		file.seekg(0);
		while(file >> c){
			switch(c){
				case '#':		// comment
					{
						// eat up rest of line
						file.getLine(0);
						break;
					}

				case 'v':		// v, vn, vt
					{
						if(state != 1){
							// start a new group
							// take charge the lastest group
							if(gFCount != 0){
								// have a last group
								SEntity *entity = groups[gCount];
								entity->prepareGeometry();
							//	if(!entity->isSmooth()){
								entity->setupNormals();
							//	}
								entity->attachedNode()->translate(entity->center());
								entity->locateToCenter();

								vertices = 0;
								normals = 0;
								texcoords = 0;
								vindices = 0;
								nindices = 0;
								tindices = 0;

								gCount++;

								tVCount += gVCount;
								tNCount += gNCount;
								tTCount += gTCount;

								gVCount = 0;
								gNCount = 0;
								gTCount = 0;
								gFCount = 0;
							}

							// prepare to setup next entity's data
							SEntity *nextEntity = groups[gCount];
							SVertexData *vdata = nextEntity->getVertexData();
							SIndexData *idata = nextEntity->getIndexData();

							// vertex
							vertices = static_cast<Vector3*>(vdata->getElement(SVertexBufferElement::ET_VERTEX)->data());
							vindices = static_cast<IndexedTriangle*>(idata->getElement(SVertexBufferElement::ET_VERTEX)->data());
							if(ncounts[gCount]){
								normals = static_cast<Vector3*>(vdata->getElement(SVertexBufferElement::ET_NORMAL)->data());
								nindices = static_cast<IndexedTriangle*>(idata->getElement(SVertexBufferElement::ET_NORMAL)->data());
							}
							if(tcounts[gCount]){
								texcoords = static_cast<Vector2*>(vdata->getElement(SVertexBufferElement::ET_TEXTURE_COORD)->data());
								tindices = static_cast<IndexedTriangle*>(idata->getElement(SVertexBufferElement::ET_TEXTURE_COORD)->data());
							}

							// now we are in 'v' area
							state = 1;
						}

						c = file.get();
						switch(c){
				case ' ':	// vertex
					{
						file >> x >> y >> z;
						vertices[gVCount].setValues(x, y, z);

						++gVCount;
						break;
					}

				case 'n':	// normal
					{
						file >> x >> y >> z;
						normals[gNCount].setValues(x, y, z);

						++gNCount;
						break;
					}

				case 't':	// texcoord
					{
						file >> x >> y;
						texcoords[gTCount].setValues(x, y);

						++gTCount;
						break;
					}

				default:	// unknown
					{
						file.close();
						THROW_SAGI_EXCEPT(SException::ERR_INVALIDPARAMS,
							"Unknown token after \"v\".",
							"SSceneDelegate::loadScene_obj");
					}
						}

						break;
					} //#### end v switch

				case 'm':		// mtl file
					{
						// read mtl file
						// for future ...
						break;
					}

				case 'u':		// nothing
					{
						// eat up rest of line
						file.getLine(0);
						break;
					}

				case 'g':		// group
					{
						// eat up rest of line
						file.getLine(0);
						break;
					}

				case 'f':		// face
					{
						if(state != 2){
							// start a group
							state = 2;
						}

						v = t = n = 0;
						for(uInt i=0; i<polyType; i++){
							if(tcounts[gCount]){
								if(ncounts[gCount]){	// v/t/n
									file >> v >> c >> t >> n;
								}else{			// v/t
									file >> v >> c >> t;
								}
							}else{
								if(ncounts[gCount]){	// v//n
									file >> v >> c >> c >> n;
								}else{			// v
									file >> v;
								}
							}

							if(v > 0){
								vindices[gFCount][i] = v - tVCount - 1;
								if(tindices)
									tindices[gFCount][i] = t - tTCount - 1;
								if(nindices)
									nindices[gFCount][i] = n - tNCount - 1;
							}else if(v < 0){
								vindices[gFCount][i] = v + gVCount;
								if(tindices)
									tindices[gFCount][i] = t + gTCount;
								if(nindices)
									nindices[gFCount][i] = n + gNCount;
							}
						}

						++gFCount;
						break;
					}

				default:
					{
						// eat up rest of line
						file.getLine(0);
						break;
					}

			}
		}

		// take charge the last group
		if(gFCount != 0){
			// have a last group
			SEntity *entity = groups[gCount];
			entity->prepareGeometry();
		//	if(!entity->isSmooth()){
			entity->setupNormals();
		//	}
			entity->attachedNode()->translate(entity->center());
			entity->locateToCenter();

			vertices = 0;
			normals = 0;
			texcoords = 0;
			vindices = 0;
			nindices = 0;
			tindices = 0;

			gCount++;

			tVCount += gVCount;
			tNCount += gNCount;
			tTCount += gTCount;

			gVCount = 0;
			gNCount = 0;
			gTCount = 0;
			gFCount = 0;
		}
		// second pass end //////////////////////////////////////////////////////////////////////////
	}

	// SSceneDelegate //////////////////////////////////////////////////////////////////////////

	// SSkeletonDelegate //////////////////////////////////////////////////////////////////////////

	//  [1/11/2009 zhangxiang]
	void SSkeletonDelegate::loadImpl(const StdString &aFileName, SResource *outResource){
		// test file type
		size_t lastslash = aFileName.find_last_of("\\/");
		size_t lastdot = aFileName.find_last_of(".");

		if(lastdot != StdString::npos){
			StdString postfix = aFileName.substr(lastdot);
			if(postfix == ".bvh"){
				// .obj file
				loadSkeleton_bvh(aFileName, static_cast<SScene*>(outResource));
			}else{
				THROW_SAGI_EXCEPT(SException::ERR_NOT_IMPLEMENTED,
					postfix + " is not a supported file type.",
					"SSkeletonDelegate::loadImpl");
			}
		}else{
			THROW_SAGI_EXCEPT(SException::ERR_NOT_IMPLEMENTED,
				"File type without postfix is supported.",
				"SSkeletonDelegate::loadImpl");
		}
	}

	//  [1/11/2009 zhangxiang]
	void SSkeletonDelegate::loadSkeleton_bvh(const StdString &aFileName, SScene *outScene){
		SFile file(aFileName);
		if(!file){
			THROW_SAGI_EXCEPT(SException::ERR_INTERNAL_ERROR,
				"Cannot open file \"" + aFileName + "\".",
				"SSkeletonDelegate::loadSkeleton_bvh");
		}
		
		SSceneNode *skroot = loadBVHSkeletonRecursively(file, outScene);

		// setup joint spheres
		setupJointSpheres(skroot, skroot->position().y() / 50);

		// create a node to be the root node for the adding scene
		size_t lastslash = aFileName.find_last_of("\\/");
		size_t lastdot = aFileName.find_last_of(".");

		StdString mcname;
		if(lastslash == StdString::npos){
			mcname = aFileName.substr(0, lastdot);
		}else{
			mcname = aFileName.substr(lastslash + 1, lastdot);
		}

		// motion clip
		SMotionClip *mc;
		mc = static_cast<SMotionClip*>(outScene->getAnimation());
		if(!mc){
			mc = new SMotionClip(mcname);
			outScene->setAnimation(mc);
		}

		mc->load(aFileName, SResource::LOP_REFRESH);

		// binding info
		setBindingInfoRecursively(skroot, mc, outScene->bindingMap());
	}

	//  [1/11/2009 zhangxiang]
	SSceneNode *SSkeletonDelegate::loadBVHSkeletonRecursively(SFile &file, SScene *outScene){
		SSceneNode *ret;
		int endsitenum = 0;
		Real x, y, z;
		StdString str, name;
		while(file >> str){
			if(str == "JOINT" || str == "ROOT"){
				// joint
				// get joint name
				file >> name;
				// create node for joint
				ret = outScene->createSceneNode(name);

				// "{"
				file >> str;

				// "OFFSET"
				file >> str;
				// offset data
				file >> x >> y >> z;
				ret->translate(Vector3(x, y, z), SNode::TS_PARENT);

				// "CHANNELS"
				file >> str;
				// eat up rest of line
				file.getLine(0);

				// children
				SSceneNode *child;
				while(child = loadBVHSkeletonRecursively(file, outScene)){
					child->stepToParent(ret);
				}

				return ret;

			}else if(str == "End"){
				// end site joint
				// eat up rest of line
				file.getLine(0);
				// set name for this End_Site
				name = "EndSite_" + SString::to_string(endsitenum++);
				// create node for joint
				ret = outScene->createSceneNode(name);

				// "{"
				file >> str;

				// "OFFSET"
				file >> str;
				// offset data
				file >> x >> y >> z;
				ret->translate(Vector3(x, y, z), SNode::TS_PARENT);

				// no channels
				// no children

				// "}"
				file >> str;

				return ret;
			}else if(str == "HIERARCHY"){
				continue;
			}else if(str == "}"){
				return 0;
			}else{
				// unknown token
				file.close();
				THROW_SAGI_EXCEPT(SException::ERR_INVALIDPARAMS,
					"Unknown token", "SSkeletonDelegate::loadBVHSkeletonRecursively");
			}

		} //#### end while

		return 0;
	}

	//  [1/12/2009 zhangxiang]
	void SSkeletonDelegate::setupJointSpheres(SSceneNode *aSNode, Real aRadius){
		// bone joint sphere
		SSphere *bonejoint = new SSphere(aSNode->name(), aRadius, 20, 20);
		bonejoint->attachToNode(aSNode);

		// children
		SNode::ConstChildIterator it = aSNode->getConstChildIterator();
		for(; it.hasMoreElements(); ++it){
			SSceneNode *child = dynamic_cast<SSceneNode*>(it.value());
			if(child){
				SLine *line = new SLine(Vector3::ZERO, child->position() - aSNode->position());
				line->attachToNode(aSNode);
				setupJointSpheres(child, aRadius);
			}
		}
	}

	//  [1/12/2009 zhangxiang]
	void SSkeletonDelegate::setBindingInfoRecursively(SSceneNode *aSNode, SAnimation *aAnima, BindingMap &outBindingMap){
		SAnimaJoint *joint = aAnima->findAnimaJoint(aSNode->name());
		if(joint){
			outBindingMap.addBindingInfo(aSNode->getNodeID(), joint->getDjID());
		}
		// children
		SNode::ConstChildIterator it = aSNode->getConstChildIterator();
		for(; it.hasMoreElements(); ++it){
			SSceneNode *child = dynamic_cast<SSceneNode*>(it.value());
			if(child){
				setBindingInfoRecursively(child, aAnima, outBindingMap);
			}
		}
	}

	// SSkeletonDelegate //////////////////////////////////////////////////////////////////////////

} // namespace Sagitta