#pragma once

#include "StdHeader.h"
#include "SceneNode.h"
#include "Event.h"
#include "Process.h"
#include "Q3FileParser.h"
#include "Actors.h"


const double SCREEN_REFRESH_RATE(1000.0f/60.0f);
const int	MAP_SIZE = 20;
const int	HALF_MAP_SIZE = 10;

class HumanView;

// Mouse and Keyboard controller
class HumanInterfaceController
{
	unsigned int	m_scrollLines;
	int				m_scrolled;

	Mat4x4  m_matFromWorld;
	Mat4x4	m_matToWorld;
    Mat4x4  m_matPosition;

	shared_ptr<SceneNode> m_object;

	CPoint					m_lastMousePos;
	BYTE					m_bKey[256];			// Which keys are up and down

	// Orientation Controls
	float		m_fTargetYaw;
	float		m_fTargetPitch;
	float		m_fYaw;
	float		m_fPitch;
	float		m_fPitchOnDown;
	float		m_fYawOnDown;
	float		m_maxSpeed;
	float		m_currentSpeed;
	
public:
	HumanInterfaceController(shared_ptr<SceneNode> object, float initialYaw, float initialPitch);
	void OnKeyDown(const BYTE c) {m_bKey[c] = true; }
	void OnKeyUp(const BYTE c) {if (c==16 || !m_bKey[16])m_bKey[c] = false; }
	void OnUpdate(int deltaMS);
	void OnMouseScroll(int lines);
	void OnLButtonDown(const CPoint &mousePoint);
	void OnRButtonDown(const CPoint &mousePoint);
	void OnMouseMove(const CPoint &mousePoint);
	Mat4x4 CalcViewMatrix(float yaw, float pitch);
};

// UI for the game
class HumanUI: public IScreenElement
{
	CDXUTDialogResourceManager m_DialogResourceManager;
	CDXUTDialog m_UI;
	int m_Zorder;
	bool m_visible;
	float	m_PosX, m_PosY, m_Width, m_Height;
	int m_index;
	
public:
	HumanUI();
	~HumanUI();
	virtual void VOnUpdate(int deltaMS){}
	virtual HRESULT VRender(double fTime, float fElapsedTime);
	virtual HRESULT VOnRestore();
	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg );

	virtual int VGetZOrder() const {return m_Zorder;}
	virtual void VSetZOrder(int const zOrder) {m_Zorder = zOrder;}
	virtual bool VIsVisible() const {return m_visible;}
	virtual void VSetVisible(bool visible) {m_visible = visible;}
	void OnDeviceCreate(IDirect3DDevice9* device);
	static void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl *pControl, void* pUserContext );
	void OnDeviceLost();
	void BuildInitialDialogs();
};

class CSoundProcess;

// Human view, includes 3D renderer
class HumanView: public IGameView
{
	friend class GameViewListener;
	GameViewId						m_id;
	ScreenElementList				m_screenElementList;
	DWORD							m_currTick;
	DWORD							m_lastDraw;
	GameStatus						m_status;
	shared_ptr<HumanInterfaceController>		m_controller;

	shared_ptr<HumanUI>				m_humanUI;
	shared_ptr<HumanPlayerScene>	m_pScene;
	shared_ptr<CameraNode>			m_pCamera;
	shared_ptr<CSoundProcess>		m_music;
	EventListenerPtr				m_eventListener;

	shared_ptr<SceneNode>			m_character;

	ID3DXFont*						m_pFont;
	ID3DXSprite*					m_pTextSprite;
	unsigned int					m_lastShot;
	ProcessManager					*m_processManager;


public:
	HumanView();
	~HumanView();

	virtual void VOnUpdate(int deltaMS);
	virtual GameViewId VGetId() const {return m_id;}
	virtual void VOnAttach(GameViewId vid);
	virtual void VRender(double fTime, float fElapsedTime);
	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg );
	virtual HRESULT VOnRestore();
	virtual void VOnLostDevice();
	virtual void VPushScreen(shared_ptr<IScreenElement> e);
	virtual void VPopScreen();
	virtual void VRenderText(CDXUTTextHelper &txtHelper);

	virtual void VRemoveActor(ActorId id);
	void AddShot(ActorId id, int time, Vec3 start, Vec3 end, std::string texture);
	shared_ptr<SceneNode> CreateCharacter(shared_ptr<CharacterParams> p);

	void VMoveActor(ActorId id, Mat4x4 const &mat);
	void MoveCamera(Mat4x4 const &change);
	HRESULT DeviceCreated(IDirect3DDevice9* device);
	void Attach(shared_ptr<Process> process) {m_processManager->Attach(process);}
	void BuildInitialScene();
	void RebuildUI();
	virtual void VGameStatusChange(GameStatus status) {m_status = status;}
	void MouseMove(Vec3 pos);

	bool InitAudio();

	void AddMap(shared_ptr<Q3Map> q);
};

// State data about the game
struct GameData
{
public: 
	int				m_timeLeftUntilWave;
	int				m_waveTimeLimit;
	int				m_curWave;
	int				m_curMoney;
	int				m_curLife;
};

// Logic class for the game.
class Q3Game: public IGame
{
	friend class GameApp;
	GameViewList		m_viewList;
	ActorMap			m_pActorMap;
	ActorId				m_LastActorId;
	GameStatus			m_status;
	
	EventListenerPtr	m_eventListener;
	GameData			m_data;
	int					m_curTowerType;
	ProcessManager		m_processManager;
	ActorId				m_selectedTower;

	shared_ptr<Q3Map>	m_map;
	
	void CreateGrid();
	void FindNewPaths();
	Vec3 Move(Vec3 start, Vec3 end, float size);
	
public:
	Q3Game();
	~Q3Game();
	void CreateMissile(ActorId id);
	virtual void OnUpdate(int deltaMS);
	virtual void VAddActor(shared_ptr<IActor> actor);
	virtual void VRemoveActor(ActorId id);
	virtual void VMoveActor(ActorId id, const Mat4x4 &m);
	virtual void VAddView(shared_ptr<IGameView> view);
	void BuildInitialScene();
	virtual void VGameStatusChange(GameStatus status);
	GameData GetData() {return m_data;}
	shared_ptr<IActor> GetActor(ActorId id);
	void DamageActor(ActorId id, int damage);
	void ApplyBuffToActor(ActorId id, shared_ptr<IBuff> buff);
	void RightClick(Vec3 l);
	void SelectTower(ActorId id) {m_selectedTower = id; m_curTowerType = -1;}
	void AttemptActorMove(ActorId id, Mat4x4 m, float deltaMS);

	void AddMap(shared_ptr<Q3Map> q) { m_map = q;}

	ActorId CreateCharacter(shared_ptr<CharacterParams> p);
};

// Base class that interacts with the underlying OS
class GameApp
{
	CRect m_rcWindow, m_rcDesktop;
	int m_iColorDepth;
	bool CheckMemory(const DWORD physicalRAM, const DWORD virtualRAM);
	bool CheckHardDisk(const int diskSpace);
	EventManager m_eventManager;
	bool	m_Quitting;
public:
	GameApp();
	HWND GetHwnd() {return DXUTGetHWND();}
	TCHAR *GetGameTitle() { return _T("Q3Game"); }
	bool InitInstance(HINSTANCE hInstance, LPTSTR lpCommandLine);

	static LRESULT CALLBACK MsgProc (HWND hWnd, UINT uMsg, WPARAM wParam, 
			LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext);

	static HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void *pUserContext  );
	static void    CALLBACK OnLostDevice(void *pUserContext);
	static bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
	static void CALLBACK OnUpdateGame( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void *pUserContext );
	static void CALLBACK OnRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void *pUserContext );
	static HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext) { safeTriggerEvent(Evt_Device_Created(pd3dDevice)); return S_OK; }
	static void CALLBACK OnDestroyDevice( void* pUserContext ) {};
	static bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext );
	int GetExitCode() { return DXUTGetExitCode(); }

	LRESULT OnDisplayChange(int colorDepth, int width, int height);
	LRESULT OnDeviceChange(int eventType) {return TRUE;}
	int OnClose();
	LRESULT OnSysCommand(WPARAM wParam, LPARAM lParam);

	Q3Game* CreateGameAndView();
	Q3Game* m_pGame;
	class ResCache *m_ResCache;

	bool IsQuitting() {return m_Quitting;}
	void AbortGame() {m_Quitting = true;}

	shared_ptr<Q3Map> VLoadMap();
};

// Event listener for the game logic
class GameLogicListener: public IEventListener
{
	Q3Game * m_game;
public:
	GameLogicListener(Q3Game * game):m_game(game){};
	virtual bool HandleEvent(Event const & e);
};

// Event listener for the game view
class GameViewListener: public IEventListener
{
	HumanView * m_view;
public:
	GameViewListener(HumanView * view):m_view(view){};
	virtual bool HandleEvent(Event const & e);
};

extern GameApp *g_App;
