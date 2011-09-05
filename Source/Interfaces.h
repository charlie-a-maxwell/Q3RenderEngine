#pragma once
#include "StdHeader.h"
#include <strstream>

class Resource;
class IResourceFile
{
public:
	virtual bool VOpen()=0;
	virtual int VGetResourceSize(const Resource &r)=0;
	virtual int VGetResource(const Resource &r, char *buffer)=0;
	virtual ~IResourceFile() { }
};

typedef unsigned int ActorId;

enum ActorType
{
	AT_UNKNOWN,
	AT_GROUND,
	AT_Q3,
	AT_EFFECT,
	AT_MISSILE,
	AT_CHARACTER
};

enum RenderPass
{
	RenderPass_0,
	RenderPass_Static = RenderPass_0,
	RenderPass_Actor,
	RenderPass_Effect,
	RenderPass_Sky,
	RenderPass_Last
};

enum GameStatus
{
	Game_Initializing,
	Game_Running,
	Game_Pause,
	Game_Over,
	Game_Closing,
	Game_Last
};

struct ActorParams
{
	int					m_Size;
	ActorId				m_Id;
	ActorType			m_Type;
	Mat4x4				m_Mat;


	ActorParams(){ m_Mat=Mat4x4::g_Identity; m_Type=AT_UNKNOWN; m_Size=sizeof(ActorParams);}

	int GetSize() { return m_Size; }
	virtual bool VInit(std::istrstream &in){ return true; }
	virtual bool VSerialize(std::ostrstream &out){ return true; }
};

enum BuffType
{
	BT_ICE,
	BT_DAMAGE
};

class IBuff
{
public:
	virtual ~IBuff() {};
	virtual void VApply()=0;
	virtual void VRemove()=0;
	virtual bool VOnUpdate(int deltaMS)=0;
	virtual BuffType VGetType()=0;
	virtual ActorId VGetActorId()=0;
};

typedef std::list<shared_ptr<IBuff> > BuffList;

class IActor
{
public:
	virtual ~IActor() { };
	virtual void VSetParams(shared_ptr<ActorParams> p)=0;
	virtual Mat4x4 const &VGetMat()=0;
	virtual void VSetMat(const Mat4x4 &m)=0;
	virtual void VOnUpdate(int deltaMS)=0;
	virtual shared_ptr<ActorParams> VGet()=0;
	virtual void VSetId(ActorId id)=0;
	virtual void VApplyBuff(shared_ptr<IBuff> buff)=0;
};

typedef std::map<ActorId, shared_ptr<IActor> > ActorMap;

class IScreenElement
{
public:
	virtual ~IScreenElement(){};
	virtual void VOnUpdate(int deltaMS)=0;
	virtual HRESULT VRender(double fTime, float fElapsedTime)=0;
	virtual HRESULT VOnRestore()=0;
	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg )=0;

	virtual int VGetZOrder() const = 0;
	virtual void VSetZOrder(int const zOrder) = 0;
	virtual bool VIsVisible() const = 0;
	virtual void VSetVisible(bool visible) = 0;

	virtual bool const operator <(IScreenElement const &other) { return VGetZOrder() < other.VGetZOrder(); }
};

typedef std::list<shared_ptr<IScreenElement> > ScreenElementList;

typedef unsigned int GameViewId;

class IGameView
{
public:
	virtual ~IGameView() {};
	virtual void VOnUpdate(int deltaMS)=0;
	virtual GameViewId VGetId() const=0;
	virtual void VOnAttach(GameViewId vid)=0;
	virtual void VRender(double fTime, float fElapsedTime)=0;
	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg )=0;
	virtual HRESULT VOnRestore()=0;
	virtual void VOnLostDevice()=0;
	virtual void VRemoveActor(ActorId id)=0;
	virtual void VGameStatusChange(GameStatus status)=0;
	virtual void VMoveActor(ActorId id, Mat4x4 const &m)=0;
	virtual void VRenderText(CDXUTTextHelper &txtHelper)=0;
};

typedef std::list<shared_ptr<IGameView> > GameViewList; 

class IGame
{
public:
	virtual ~IGame() {};
	virtual void OnUpdate(int deltaMS)=0;
	virtual void VAddActor(shared_ptr<IActor> actor)=0;
	virtual void VRemoveActor(ActorId id)=0;
	virtual void VMoveActor(ActorId id, const Mat4x4 &m)=0;
	virtual void VAddView(shared_ptr<IGameView> view)=0;
	virtual void VGameStatusChange(GameStatus status)=0;
};

class IAudioBuffer
{
public:
	virtual ~IAudioBuffer() {}

	virtual void *VGet()=0;
	virtual bool VOnRestore()=0;
	virtual bool VPlay(int volume, bool looping)=0;
	virtual bool VStop()=0;

	virtual bool VTogglePause()=0;
	virtual bool VIsPlaying()=0;
	virtual bool VIsLooping()=0;
	virtual void VSetVolume(int volume)=0;
	virtual int VGetVolume()=0;
	virtual float VGetProgress()=0;
};

class CSoundResource;

class IAudio
{
public:
	virtual bool VActive()=0;
	virtual void VStopAllSounds()=0;
	virtual void VPauseAllSounds()=0;
	virtual void VResumeAllSounds()=0;

	virtual bool VInitialize(HWND hWnd)=0;
	virtual void VShutdown()=0;

	virtual IAudioBuffer *VInitAudioBuffer(shared_ptr<CSoundResource> soundResource)=0;
	virtual void VReleaseAudioBuffer(IAudioBuffer* audioBuffer)=0;
};

class Scene;
class SceneNodeProperties;

class ISceneNode
{
protected:

public:
	virtual const SceneNodeProperties * const VGet() const =0;
	virtual void VSetTransform(const Mat4x4 *toWorld, const Mat4x4 *fromWorld=NULL)=0;
	virtual HRESULT VOnUpdate(Scene *, DWORD const elapsed)=0;
	virtual HRESULT VOnRestore(Scene *pScene)=0;

	virtual HRESULT VPreRender(Scene *pScene)=0;
	virtual HRESULT VRender(Scene *pScene)=0;
	virtual HRESULT VRenderChildren(Scene *pScene)=0;
	virtual HRESULT VPostRender(Scene *pScene)=0;

	virtual bool VIsVisible(Scene *pScene)=0;

	virtual bool VAddChild(shared_ptr<ISceneNode> kid)=0;
	virtual bool VRemoveChild(ActorId id)=0;

	virtual ~ISceneNode() {};

};

typedef std::vector<shared_ptr<ISceneNode> > SceneNodeList;

struct AlphaSceneNode
{
	shared_ptr<ISceneNode> m_pNode;
	Mat4x4 m_concat;
	float m_ScreenZ;

	bool const operator < (AlphaSceneNode const &other){ return m_ScreenZ < other.m_ScreenZ;}
};

typedef std::list<AlphaSceneNode> AlphaSceneNodes;
typedef std::map<ActorId, shared_ptr<ISceneNode> > SceneActorMap;


class IEventData
{
public:
	virtual ~IEventData() {};
};

typedef shared_ptr<IEventData> EventDataPtr;


class EventType;
class Event;

class IEventListener
{
public:
	virtual ~IEventListener(){}
	virtual bool HandleEvent(Event const & e)=0;
};

typedef shared_ptr<IEventListener> EventListenerPtr;
typedef std::list<EventListenerPtr> EventListenerList;
typedef shared_ptr<Event> EventPtr;

class IEventManager
{
public:
	IEventManager();
	~IEventManager();

	static IEventManager * Get();
	virtual bool addListener(EventListenerPtr const & listener, EventType const & type)=0;
	virtual bool triggerEvent(Event const & event)=0;
	virtual bool queueEvent(EventPtr const & event)=0;
	virtual bool tick(unsigned int maxMS)=0;
	virtual bool validateType(EventType const & type)=0;

	friend bool safeAddListener(EventListenerPtr const & listener, EventType const & type);
	friend bool safeTriggerEvent(Event const & event);
	friend bool safeQueueEvent(EventPtr const & event);
	friend bool safeTick(unsigned int maxMS);
	friend bool safeValidateType(EventType const & type);
};

	bool safeAddListener(EventListenerPtr const & listener, EventType const & type);
	bool safeTriggerEvent(Event const & event);
	bool safeQueueEvent(EventPtr const & event);
	bool safeTick(unsigned int maxMS);
	bool safeValidateType(EventType const & type);

	static IEventManager* g_EventManager = NULL;