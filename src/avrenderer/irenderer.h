#pragma once

#include <memory>

#include "aardvark.capnp.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace aardvark
{
	class CAardvarkClient;
}

class IModelInstance
{
public:
	virtual ~IModelInstance() {}

	virtual void setUniverseFromModel( const glm::mat4 & universeFromModel ) = 0;
	virtual void setOverrideTexture( AvSharedTextureInfo::Reader ) = 0;
};

enum class EHand
{
	Invalid,
	Left,
	Right,
};


#ifndef _WINDEF_
class HINSTANCE__; // Forward or never
typedef HINSTANCE__* HINSTANCE;
#endif

class IRenderer
{
public:
	virtual ~IRenderer() {}

	virtual void init( HINSTANCE hInstance, aardvark::CAardvarkClient *client ) = 0;
	virtual std::unique_ptr<IModelInstance> createModelInstance( const std::string & uri ) = 0;
	virtual void resetRenderList() = 0;
	virtual void addToRenderList( IModelInstance *modelInstance ) = 0;
	virtual void processRenderList() = 0;

	// these probably don't belong on the renderer
	virtual bool getUniverseFromOrigin( const std::string & originPath, glm::mat4 *universeFromOrigin ) = 0;
	virtual bool isGrabPressed( EHand hand ) = 0;
	virtual void sentHapticEventForHand( EHand hand, float amplitude, float frequency, float duration ) = 0;
	virtual void updateOpenVrPoses() = 0;
	virtual void runFrame( bool *shouldQuit, double frameTime ) = 0;
};
