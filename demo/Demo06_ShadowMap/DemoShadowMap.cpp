#include "DemoShadowMap.h"
#include <engine/scenegraph/sgScene.h>
#include <engine/scenegraph/sgSceneObject.h>
#include <engine/component/sgCameraComponent.h>
#include <engine/component/sgMeshComponent.h>
#include <engine/resource/sgResourceCenter.h>
#include <engine/resource/sgSimpleMeshes.h>
#include <engine/resource/sgMaterial.h>
#include <engine/resource/sgTexture.h>
#include <engine/resource/sgLoader.h>
#include <engine/renderer/sgSceneRenderEffect.h>
#include <engine/renderer/sgObjectRenderEffect.h>
#include <engine/renderer/sgRenderPass.h>
#include <engine/renderer/sgGpuProgram.h>
#include <engine/renderer/sgGLGpuProgram.h>
#include <engine/renderer/sgRenderer.h>
#include <engine/renderer/sgRenderQueue.h>
#include <engine/component/sgLightComponent.h>
#include <engine/component/sgRenderStateComponent.h>
#include <engine/common/sgLogSystem.h>
using namespace Sagitta;

DemoShadowMap::DemoShadowMap() 
: sgDemo("Sagitta Demo ShadowMap", "../resource/")
{

}

DemoShadowMap::~DemoShadowMap()
{

}

void DemoShadowMap::prepare(void)
{
    if(mScene && mCamera)
	{
		if( ! (sgGetRenderer()->initShaderEnvironment()) )
		{
			sgLogSystem::instance()->warning("Init shader environment failed");
		}

		// prepare shaders
		sgShader *vsColor = (sgShader*)sgResourceCenter::instance()->createResource(sgGLVertexShader::GetClassName(), "shaders/VertexColor.vs");
		sgShader *fsColor = (sgShader*)sgResourceCenter::instance()->createResource(sgGLFragmentShader::GetClassName(), "shaders/VertexColor.fs");
		sgGpuProgram *programColor = (sgGpuProgram*)sgObject::createObject(sgGLGpuProgram::GetClassName());
		if(!programColor->setShader(vsColor->getFilename(), fsColor->getFilename()))
		{
			sgLogSystem::instance()->warning("Program 'Color' is invalid");
		}

		sgShader *vsStandard = (sgShader*)sgResourceCenter::instance()->createResource(sgGLVertexShader::GetClassName(), "shaders/StandardShading.vs");
		sgShader *fsStandard = (sgShader*)sgResourceCenter::instance()->createResource(sgGLFragmentShader::GetClassName(), "shaders/StandardShading.fs");
		sgGpuProgram *programStandard = (sgGpuProgram*)sgObject::createObject(sgGLGpuProgram::GetClassName());
		if(!programStandard->setShader(vsStandard->getFilename(), fsStandard->getFilename()))
		{
			sgLogSystem::instance()->warning("Program 'Standard Shading' is invalid");
		}

		// create scene effect
		sgRenderEffect *sceneRe = mScene->createRenderEffect(sgSceneRenderEffect::GetClassName());
		sgRenderPass *sceneRp = sceneRe->addPass(sgRenderQueue::GetClassName());
		sceneRp->setProgram(programStandard);


		// prepare materials
		sgMaterial *mat1 = (sgMaterial*)sgResourceCenter::instance()->createResource(sgMaterial::GetClassName(), "material_test_1");
		mat1->setDiffuseColor(Color(255, 125, 75));
		

		// prepare camera
		mCamera->translate(Vector3(0.0f, 32.5f, 202.0f));
		//mCamera->pitch(Radian(-Math::PI / 6.0f));

		// set lights
		sgSceneObject *light1 = (sgSceneObject*)sgObject::createObject(sgSceneObject::GetClassName());
		light1->setParent(mScene->getRoot());
		light1->translate(Vector3(3.0f, 3.0f, 5.0f));
		sgLightComponent *lightComp1 = (sgLightComponent*)light1->createComponent(sgLightComponent::GetClassName());
		//lightComp1->setDiffuseColor(Color(0, 125, 11));
		lightComp1->setIntensity(8.0f);
		

		// prepare resources
		sgMeshTriangle *meshTriangle = (sgMeshTriangle*)sgResourceCenter::instance()->createResource(sgMeshTriangle::GetClassName(), sgMeshTriangle::InternalFileName);
		meshTriangle->setVertecies(Vector3(-1.0f, 0.0, 0.0f), Color::RED, 
			Vector3(1.0f, 0.0f, 0.0f), Color::GREEN, 
			Vector3(0.0f, 1.0f, 0.0f), Color::BLUE);

		// place cube 
		sgSceneObject *objRoot = sgLoader::load_pod("scene/podscene1/scene.pod");//sgLoader::load_obj("models/cube.obj");
        objRoot->setParent(mScene->getRoot());
		objRoot->translate(Vector3(-1.0f, 0.0f, -2.5f));
		objRoot->yaw(Radian(Math::PI_DIV_4));
		//objRoot->pitch(Radian(-Math::PI_DIV_3));

        sgSceneObject *objCube = (sgSceneObject*)objRoot->getFirstChild();
		sgRenderStateComponent *cubeRsComp = (sgRenderStateComponent*)objCube->createComponent(sgRenderStateComponent::GetClassName());
		cubeRsComp->setMaterialFile(mat1->getFilename());
        // load textures
        sgTexture *texture = (sgTexture*)sgResourceCenter::instance()->createResource(sgTexture::GetClassName(), "images/cube.png");
        if(texture)
        {
            cubeRsComp->addTexture(texture->getFilename());
        }

		// triangle
	/*	sgSceneObject *triangle = (sgSceneObject*)sgObject::createObject(sgSceneObject::GetClassName());
		triangle->setParent(mScene->getRoot());
		triangle->yaw(Radian(Math::PI_DIV_4));
		//triangle->scale(Vector3(5, 5, 5));
    
		sgMeshComponent *triMeshComp = (sgMeshComponent*)triangle->createComponent(sgMeshComponent::GetClassName());
		triMeshComp->setMeshFile(meshTriangle->getFilename());
	    
		sgRenderStateComponent *triRsComp = (sgRenderStateComponent*)triangle->createComponent(sgRenderStateComponent::GetClassName());
		sgObjectRenderEffect *triRe = triRsComp->createRenderEffect(sgObjectRenderEffect::GetClassName());
		sgRenderPass *triRs0 = triRe->addPass();
		triRs0->setProgram(programColor);      
      */  
	}
}
