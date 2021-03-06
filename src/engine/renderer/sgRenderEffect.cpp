//  [4/11/2012  zhangxiang]

#include "sgRenderEffect.h"
#include "sgRenderPass.h"
#include "sgRenderer.h"
#include "sgRenderTechnique.h"
#include "sgRenderQueue.h"
#include "sgGpuProgram.h"
#include "engine/scenegraph/sgSceneObject.h"
#include "engine/buffer/sgBuffer.h"
#include "engine/buffer/sgVertexData.h"
#include "engine/buffer/sgVertexIndexBuffer.h"
#include "engine/buffer/sgVertexBufferElement.h"
#include "engine/common/sgException.h"
#include "engine/common/sgUtil.h"
#include "engine/scenegraph/sgSceneObject.h"
#include "engine/scenegraph/sgScene.h"
#include "engine/component/sgLightComponent.h"
#include "engine/component/sgRenderStateComponent.h"
#include "engine/component/sgCameraComponent.h"
#include "engine/component/sgMeshComponent.h"
#include "engine/resource/sgMaterial.h"
#include "engine/resource/sgMesh.h"
#include "engine/resource/sgTexture.h"

namespace Sagitta
{
    SG_META_DEFINE(sgRenderEffect, sgObject)

    const sgStrHandle sgRenderEffect::ModelMatrix("sg_ModelMatrix");
	const sgStrHandle sgRenderEffect::ViewMatrix("sg_ViewMatrix");
	const sgStrHandle sgRenderEffect::ProjectionMatrix("sg_ProjectionMatrix");
    const sgStrHandle sgRenderEffect::MVMatrix("sg_MVMatrix");
	const sgStrHandle sgRenderEffect::MVPMatrix("sg_MVPMatrix");
    
    const sgStrHandle sgRenderEffect::NormalMatrix("sg_NormalMatrix");
    const sgStrHandle sgRenderEffect::NormalMVMatrix("sg_NormalMVMatrix");
    
   	const sgStrHandle sgRenderEffect::Enviroment_Ambient("sg_Enviroment_Ambient");
    
	const sgStrHandle sgRenderEffect::Light0_Position("sg_Light0_Position");
	const sgStrHandle sgRenderEffect::Light1_Position("sg_Light1_Position");
	const sgStrHandle sgRenderEffect::Light2_Position("sg_Light2_Position");
	const sgStrHandle sgRenderEffect::Light3_Position("sg_Light3_Position");

	const sgStrHandle sgRenderEffect::Light0_Ambient("sg_Light0_Ambient");
	const sgStrHandle sgRenderEffect::Light1_Ambient("sg_Light1_Ambient");
	const sgStrHandle sgRenderEffect::Light2_Ambient("sg_Light2_Ambient");
	const sgStrHandle sgRenderEffect::Light3_Ambient("sg_Light3_Ambient");

	const sgStrHandle sgRenderEffect::Light0_Diffuse("sg_Light0_Diffuse");
	const sgStrHandle sgRenderEffect::Light1_Diffuse("sg_Light1_Diffuse");
	const sgStrHandle sgRenderEffect::Light2_Diffuse("sg_Light2_Diffuse");
	const sgStrHandle sgRenderEffect::Light3_Diffuse("sg_Light3_Diffuse");

	const sgStrHandle sgRenderEffect::Light0_Specular("sg_Light0_Specular");
	const sgStrHandle sgRenderEffect::Light1_Specular("sg_Light1_Specular");
	const sgStrHandle sgRenderEffect::Light2_Specular("sg_Light2_Specular");
	const sgStrHandle sgRenderEffect::Light3_Specular("sg_Light3_Specular");

	const sgStrHandle sgRenderEffect::Light0_Intensity("sg_Light0_Intensity");
	const sgStrHandle sgRenderEffect::Light1_Intensity("sg_Light1_Intensity");
	const sgStrHandle sgRenderEffect::Light2_Intensity("sg_Light2_Intensity");
	const sgStrHandle sgRenderEffect::Light3_Intensity("sg_Light3_Intensity");
	
    
	const sgStrHandle sgRenderEffect::Material_Ambient("sg_Material_Ambient");
	const sgStrHandle sgRenderEffect::Material_Diffuse("sg_Material_Diffuse");
	const sgStrHandle sgRenderEffect::Material_Specular("sg_Material_Specular");
	const sgStrHandle sgRenderEffect::Material_Emission("sg_Material_Emission");
	const sgStrHandle sgRenderEffect::Material_Shininess("sg_Material_Shininess");
	const sgStrHandle sgRenderEffect::Material_SpecularAmount("sg_Material_SpecularAmount");
	const sgStrHandle sgRenderEffect::Material_ReflectFraction("sg_Material_ReflectFraction");
    
    
    const sgStrHandle sgRenderEffect::Texture0("sg_Sampler0");
    const sgStrHandle sgRenderEffect::Texture1("sg_Sampler1");
    const sgStrHandle sgRenderEffect::Texture2("sg_Sampler2");
    const sgStrHandle sgRenderEffect::Texture3("sg_Sampler3");
    
    const sgStrHandle sgRenderEffect::Texture0_Enabled("sg_Sampler0_Enabled");
    const sgStrHandle sgRenderEffect::Texture1_Enabled("sg_Sampler1_Enabled");
    const sgStrHandle sgRenderEffect::Texture2_Enabled("sg_Sampler2_Enabled");
    const sgStrHandle sgRenderEffect::Texture3_Enabled("sg_Sampler3_Enabled");
    
    
    const sgStrHandle sgRenderEffect::Camera_Near("sg_Camera_Near");
    const sgStrHandle sgRenderEffect::Camera_Far("sg_Camera_Far");
    const sgStrHandle sgRenderEffect::Camera_Range("sg_Camera_Range");

    sgRenderEffect::sgRenderEffect(void)
	: sgObject(), mGpuProgram(NULL)
    {
        
    }
    
    sgRenderEffect::~sgRenderEffect(void)
    {   

        FrameUniformMap::iterator it = mFrameUniformMap.begin();
        for(; it!=mFrameUniformMap.end(); ++it)
        {
            delete it->second.data;
        }
        mFrameUniformMap.clear();
    }

	void sgRenderEffect::addFrameUniform( const sgStrHandle &dataName, FrameUniform uniform )
	{
		if(uniform.data == NULL)
			return ;

		sgBuffer *refdata = new sgBuffer(*(uniform.data), 0);

		FrameUniformMap::iterator it = mFrameUniformMap.find(dataName);
		if(it != mFrameUniformMap.end())
		{
			delete it->second.data;
			mFrameUniformMap.erase(it);
		}

		uniform.data = refdata;
		mFrameUniformMap.insert(std::make_pair(dataName, uniform));
	}

	void sgRenderEffect::renderObject( sg_render::CurrentRenderParam *param, sgSceneObject *object )
	{
		sgMeshComponent *meshComp = (sgMeshComponent*)(object->getComponent(sgMeshComponent::GetClassTypeName()));
		if(!meshComp)
			return ;
		sgMesh *mesh = meshComp->getMesh();
		if(!mesh)
			return ;

		sgVertexData *pvb = NULL;// = mesh->getVertexData(); //new sgVertexData();
		sgVertexIndexBuffer *pvib = NULL; // = mesh->getVertexIndexBuffer(); //new sgVertexIndexBuffer(sgVertexBufferElement::ET_VERTEX);
		if(!mesh->getVertexBuffer(&pvb, &pvib))
			return ;
		if(!pvb || !pvib)
			return ;

		const Matrix4 &modelMatrix = object->getFullTransform();

		if(param->last_gpu_program != param->current_gpu_program)
		{
			param->current_gpu_program->useProgram();
			setUniformFrame(param);
            sgGetRenderer()->getRenderTechnique()->_setUniformFrameExtra(param);
		}
		setUniformObject(param, object);
        sgGetRenderer()->getRenderTechnique()->_setUniformObjectExtra(param, object);

		//sgRenderStateComponent *renderState = (sgRenderStateComponent*)(object->getComponent(sgRenderStateComponent::GetClassTypeName()));
		//sgMaterial *material = 0;
		//if(renderState)
		//	material = (sgMaterial*)(renderState->getMaterial());

		

		sgGetRenderer()->renderProgramPipeline(pvb, pvib, modelMatrix, mesh->polyType());

		param->last_gpu_program = param->current_gpu_program;
	}

	void sgRenderEffect::setUniformFrame( sg_render::CurrentRenderParam *param )
	{
		if(!param)
			return ;
        
        /*
        // camera properties
        Real camera_near = param->pcamera->zNear();
        param->current_gpu_program->setParameter(Camera_Near, 1, &camera_near);
        
        Real camera_far = param->pcamera->zFar();
        param->current_gpu_program->setParameter(Camera_Far, 1, &camera_far);
        
        Real camera_range = camera_far - camera_near;
        param->current_gpu_program->setParameter(Camera_Range, 1, &camera_range);
         */
        
        // view and projection matrix
        param->current_gpu_program->setParameter(ViewMatrix, (1<<1)|0, param->view_matrix.arr());
        param->current_gpu_program->setParameter(ProjectionMatrix, (1<<1)|0, param->projection_matrix.arr());

		FrameUniformMap::iterator it = mFrameUniformMap.begin();
		for(; it!=mFrameUniformMap.end(); ++it)
		{
			param->current_gpu_program->setParameter(it->first, it->second.extra, it->second.data->data());
		}
        
        // Enviroment_Ambient
        param->current_gpu_program->setParameter(Enviroment_Ambient, 1, param->pscene->getAmbiantColor().toGLColor().data());
		
		// light data
		for(size_t i=0; i<param->lightlist.size() && i<4; ++i)
		{
			sgLightComponent *light = param->lightlist[i];
			Real intensity = light->getIntensity();
			if(i == 0)
			{
				param->current_gpu_program->setParameter(Light0_Position, 1, Vector4(light->getParent()->position()).data());
				param->current_gpu_program->setParameter(Light0_Ambient, 1, light->ambientColor().toGLColor().data());
				param->current_gpu_program->setParameter(Light0_Diffuse, 1, light->diffuseColor().toGLColor().data());
				param->current_gpu_program->setParameter(Light0_Specular, 1, light->specularColor().toGLColor().data());
				param->current_gpu_program->setParameter(Light0_Intensity, 1, &intensity);
			}
			else if(i == 1)
			{
				param->current_gpu_program->setParameter(Light1_Position, 1, Vector4(light->getParent()->position()).data());
				param->current_gpu_program->setParameter(Light1_Ambient, 1, light->ambientColor().toGLColor().data());
				param->current_gpu_program->setParameter(Light1_Diffuse, 1, light->diffuseColor().toGLColor().data());
				param->current_gpu_program->setParameter(Light1_Specular, 1, light->specularColor().toGLColor().data());
				param->current_gpu_program->setParameter(Light1_Intensity, 1, &intensity);
			}
			else if(i == 2)
			{
				param->current_gpu_program->setParameter(Light2_Position, 1, Vector4(light->getParent()->position()).data());
				param->current_gpu_program->setParameter(Light2_Ambient, 1, light->ambientColor().toGLColor().data());
				param->current_gpu_program->setParameter(Light2_Diffuse, 1, light->diffuseColor().toGLColor().data());
				param->current_gpu_program->setParameter(Light2_Specular, 1, light->specularColor().toGLColor().data());
				param->current_gpu_program->setParameter(Light2_Intensity, 1, &intensity);
			}
			else if(i == 3)
			{
				param->current_gpu_program->setParameter(Light3_Position, 1, Vector4(light->getParent()->position()).data());
				param->current_gpu_program->setParameter(Light3_Ambient, 1, light->ambientColor().toGLColor().data());
				param->current_gpu_program->setParameter(Light3_Diffuse, 1, light->diffuseColor().toGLColor().data());
				param->current_gpu_program->setParameter(Light3_Specular, 1, light->specularColor().toGLColor().data());
				param->current_gpu_program->setParameter(Light3_Intensity, 1, &intensity);
			}
		}
	}

	void sgRenderEffect::setUniformObject( sg_render::CurrentRenderParam *param, sgSceneObject *object )
	{
		if(!param || !object)
			return ;
        
        // set model matrix
        Matrix4 modelMatrix = object->getFullTransform().transpose();
        Matrix4 mvMatrix = modelMatrix * param->view_matrix;
        Matrix4 mvpMatrix = modelMatrix * param->vp_matrix;
        param->current_gpu_program->setParameter(ModelMatrix, (1<<1)|0, modelMatrix.arr());
        param->current_gpu_program->setParameter(MVMatrix, (1<<1)|0, mvMatrix.arr());
        param->current_gpu_program->setParameter(MVPMatrix, (1<<1)|0, mvpMatrix.arr());
        
        // normal matrix
        Matrix3 normalMat3;
        object->getFullTransform().extract3x3Matrix(normalMat3);
        normalMat3 = normalMat3.inverse().transpose();
        Matrix4 normalMat(normalMat3);
        normalMat = normalMat.transpose();
        Matrix4 normalMVMatrix = normalMat * param->view_matrix;
        param->current_gpu_program->setParameter(NormalMatrix, (1<<1)|0, normalMat.arr());
        param->current_gpu_program->setParameter(NormalMVMatrix, (1<<1)|0, normalMVMatrix.arr());

		sgRenderStateComponent *renderState = (sgRenderStateComponent*)(object->getComponent(sgRenderStateComponent::GetClassTypeName()));
		sgMaterial *material = NULL;
		if(renderState != NULL)
		{
			material = renderState->getMaterial();
		}
		if(material != NULL)
		{
			param->current_gpu_program->setParameter(Material_Ambient, 1, material->ambientColor().toGLColor().data());
			param->current_gpu_program->setParameter(Material_Diffuse, 1, material->diffuseColor().toGLColor().data());
			param->current_gpu_program->setParameter(Material_Specular, 1, material->specularColor().toGLColor().data());
			param->current_gpu_program->setParameter(Material_Emission, 1, material->emissionColor().toGLColor().data());
			
			Real shininess = material->shininess();
			param->current_gpu_program->setParameter(Material_Shininess, 1, &shininess);

			Real specularAmount = material->specularAmount();
			param->current_gpu_program->setParameter(Material_SpecularAmount, 1, &specularAmount);

			Real reflectFraction = material->reflectFraction();
			param->current_gpu_program->setParameter(Material_ReflectFraction, 1, &reflectFraction);
		}
        // texture
        int textureEnabled = 0;
        param->textures.clear();
        param->current_gpu_program->setParameter(Texture0_Enabled, 1, &textureEnabled);
        param->current_gpu_program->setParameter(Texture1_Enabled, 1, &textureEnabled);
        param->current_gpu_program->setParameter(Texture2_Enabled, 1, &textureEnabled);
        param->current_gpu_program->setParameter(Texture3_Enabled, 1, &textureEnabled);
        
        if(renderState != NULL)
        {
            textureEnabled = 1;
            size_t texture_num = sgMin(renderState->getTextureNum(), (size_t)Texture_Max);
            for(size_t i=0; i<texture_num; ++i)
            {
                sgTexture *texture = renderState->getTexture(i);
                if(texture == NULL || !(texture->isActive()) )
                    continue;
                
                param->textures.push_back(texture->getTextureId());
                
                if(i == 0)
                {
                    param->current_gpu_program->setParameter(Texture0, 1, &i);
                    param->current_gpu_program->setParameter(Texture0_Enabled, 1, &textureEnabled);
                }
                else if(i == 1)
                {
                    param->current_gpu_program->setParameter(Texture1, 1, &i);
                    param->current_gpu_program->setParameter(Texture1_Enabled, 1, &textureEnabled);
                }
                else if(i == 2)
                {
                    param->current_gpu_program->setParameter(Texture2, 1, &i);
                    param->current_gpu_program->setParameter(Texture2_Enabled, 1, &textureEnabled);
                }
                else if(i == 3)
                {
                    param->current_gpu_program->setParameter(Texture3, 1, &i);
                    param->current_gpu_program->setParameter(Texture3_Enabled, 1, &textureEnabled);
                }
            }
        }
        

		// uniform user defined
		setUniformObjectExtra(param, object);
	}

	void sgRenderEffect::setGpuProgram( sgGpuProgram *program )
	{
		mGpuProgram = program;
	}

} // namespace Sagitta
