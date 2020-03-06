#pragma once

// #include "javascript_object.h"
#include "aardvark/irenderer.h"
#include "intersection_tester.h"
#include "collision_tester.h"

class CAardvarkRenderProcessHandler;
class CJavascriptRenderer;


/* class CJavascriptModelInstance : public CJavascriptObjectWithFunctions
{
	friend class CJavascriptRenderer;
public:
	CJavascriptModelInstance( std::unique_ptr<IModelInstance> modelInstance, std::shared_ptr<IRenderer> renderer );
	virtual ~CJavascriptModelInstance();
	virtual bool init( CefRefPtr<CefV8Value > container ) override;

	IModelInstance *getModelInstance() { return m_modelInstance.get(); }

protected:
	std::shared_ptr<IRenderer> m_renderer;
	std::unique_ptr<IModelInstance> m_modelInstance;
}; */

class CJavascriptRenderer
{
public:
	CJavascriptRenderer( );
	virtual ~CJavascriptRenderer() noexcept;

	virtual bool init();

	// bool hasPermission( const std::string & permission );
	void runFrame();


protected:
	std::shared_ptr<IRenderer> m_renderer;
	std::unique_ptr<IVrManager> m_vrManager;

	/* CefRefPtr< CefV8Value > m_jsTraverser;
	CefRefPtr< CefV8Value > m_jsHapticProcessor;
	CIntersectionTester m_intersections;
	CCollisionTester m_collisions; */

	CAardvarkRenderProcessHandler *m_handler = nullptr;
	bool m_quitting = false;
};
