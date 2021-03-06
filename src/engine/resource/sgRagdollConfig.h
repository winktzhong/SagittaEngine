//  [10/11/2012  zhangxiang]

#ifndef __SGRAGDOLLCONFIG_H__
#define __SGRAGDOLLCONFIG_H__


#include "sgResource.h"
#include "math/sgVector3.h"
#include "../serialization/sg_serialization_nvp.h"
#include "../math_serialization.h"
#include "engine/common/sgStlAllocator.h"
#include <map>
#include <vector>

namespace Sagitta{
    
	class _SG_KernelExport sgRagdollConfig : public sgResource
	{
		SG_META_DECLARE(sgRagdollConfig)
	public:
		struct BodyInfo
		{
			enum{
				SHAPE_CAPSULE = 0,
				SHAPE_BOX = 1
			};

			std::string jointName;
			std::string joint2Name;
			std::string bodyName;
			int shapeType;		// 0 - capsule, 1 - box
			float width;			// for capsule, it's radius
			float height;
			float thick;
			// if true, pos on the joint2
			bool posOnJoint;		

			std::string constraintTo;
			Vector3 angleLowerLimit;
			Vector3 angleUpperLimit;
			bool ifCollideWithParent;	// 0 - no collide, 1 - collide

			float mass;
			bool haveInertia;	
			Vector3 inertia;

			bool ifSyncPos;
		};

		struct BodyInfoSet
		{
			std::vector<BodyInfo> mBodyInfo;
		};
        
		typedef sg_map(std::string, BodyInfo) BodyInfoMap;
    private:
		BodyInfoMap mBodyInfoSet;
        
	public:
		sgRagdollConfig(void);
		virtual ~sgRagdollConfig(void);

		const BodyInfoMap &getBodyInfoSet(void) const{ return mBodyInfoSet; }

	protected:
		virtual void onSetFilename(void);

	}; 

	template<class Archive>
	Archive &operator & (Archive &archive, sgRagdollConfig::BodyInfo &obj)
	{
		archive & SG_MAKE_NVP("jointName", obj.jointName);
		archive & SG_MAKE_NVP("joint2Name", obj.joint2Name);
		archive & SG_MAKE_NVP("bodyName", obj.bodyName);
		
		archive & SG_MAKE_NVP("shapeType", obj.shapeType);
		archive & SG_MAKE_NVP("width", obj.width);
		archive & SG_MAKE_NVP("height", obj.height);
		archive & SG_MAKE_NVP("thick", obj.thick);
		archive & SG_MAKE_NVP("posOnJoint", obj.posOnJoint);

		archive & SG_MAKE_NVP("constraintTo", obj.constraintTo);
		archive & SG_MAKE_NVP("angleLowerLimit", obj.angleLowerLimit);
		archive & SG_MAKE_NVP("angleUpperLimit", obj.angleUpperLimit);
		archive & SG_MAKE_NVP("ifCollideWithParent", obj.ifCollideWithParent);

		archive & SG_MAKE_NVP("mass", obj.mass);
		archive & SG_MAKE_NVP("haveInertia", obj.haveInertia);
		archive & SG_MAKE_NVP("inertia", obj.inertia);

		archive & SG_MAKE_NVP("ifSyncPos", obj.ifSyncPos);

		return archive;
	}

	template<class Archive>
	Archive &operator & (Archive &archive, sgRagdollConfig::BodyInfoSet &obj)
	{
		archive & SG_MAKE_NVP("mBodyInfo", obj.mBodyInfo);
		return archive;
	}

} // namespace Sagitta


#endif // __SGRAGDOLLCONFIG_H__

