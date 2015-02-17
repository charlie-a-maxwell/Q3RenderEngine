/*
The basic game engine code. This is based on the book Game Coding Complete. 
The general structure is:
a base class that directly interfaces with the OS.
a game class that holds the logic for the game.
a view class that interacts with the player (video, audio, input)

Then the even system is used to tie it all together.
*/



#include "StdHeader.h"
#include "Game.h"
#include "..\ResourceCache\ResCache2.h"
#include <direct.h>
#include "SceneNode.h"
#include "Event.h"
#include <time.h>
#include "Sound.h"
#include "Q3FileParser.h"


GameApp *g_App;


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////GameApp////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

// Basic Constructor
GameApp::GameApp():m_eventManager()
{
	g_App = this;
	m_pGame = NULL;
	m_ResCache = NULL;
}

// Called before the object is destroyed to clean up variables.
int GameApp::OnClose()
{
	SAFE_DELETE(m_pGame);

	DestroyWindow(GetHwnd());

	SAFE_DELETE(m_ResCache);
	return 0;
}


/// checks the free space on a hard drive
// from Game Code Complete
bool GameApp::CheckHardDisk(int diskSpace)
{
	int const drive = _getdrive();

	struct _diskfree_t diskfree;

	_getdiskfree(drive, &diskfree);

	unsigned int const neededClusters = 
		diskSpace / (diskfree.sectors_per_cluster * diskfree.bytes_per_sector);

	if (diskfree.avail_clusters < neededClusters)
		return false;

	return true;
}


// checks the memory on the computer
// from Game Code Complete
bool GameApp::CheckMemory(const DWORD physicalRAM, const DWORD virtualRAM)
{
	MEMORYSTATUS status;

	GlobalMemoryStatus(&status);

	if (status.dwTotalPhys < physicalRAM)
		return false;

	if (status.dwAvailVirtual < virtualRAM)
		return false;

	char *buff = SAFE_NEW char[virtualRAM];

	if (buff)
	{
		delete[] buff;
		return true;
	}
	else
		return false;

	return true;
}

// Used to set up everything needed for the game before running.
bool GameApp::InitInstance(HINSTANCE hInstance, LPTSTR lpCommandLine)
{
	// this is from Game Code Complete
	bool resourceCheck = true;

	// Checks for enough ram and harddrive space.
	while (!resourceCheck)
	{
		const DWORD physicalRAM = 32 * MEGABYTE;
		const DWORD virtualRAM = 64 * MEGABYTE;
		if (!CheckMemory(physicalRAM, virtualRAM))
			return false;

		const int diskspace = 10 * MEGABYTE;
		if (!CheckHardDisk(diskspace))
			return false;

		const int minCPUSpeed = 266;
		extern int GetCPUSpeed();
		int thisCPU = GetCPUSpeed();

		if (thisCPU < minCPUSpeed)
			return false;

		resourceCheck = true;

	}
	
	// Opens the resource needed for the game.
	m_ResCache = SAFE_NEW ResCache(5, SAFE_NEW ResourceZipFile(_T("Q3Game.zip")));
	if (!m_ResCache->Init())
	{
		return false;
	}

	// Basic DXUT initialization.
	DXUTInit(true, true, true);

	DXUTCreateWindow(GetGameTitle(), hInstance);

	if (!GetHwnd())
		return false;

	SetWindowText( GetHwnd(), GetGameTitle() );

	// Creates the game class and the human view (only view used in this game).
	m_pGame = CreateGameAndView();
	if (!m_pGame)
		return false;

	DXUTCreateDevice( D3DADAPTER_DEFAULT, true, SCREEN_WIDTH, SCREEN_HEIGHT, IsDeviceAcceptable, ModifyDeviceSettings);

	srand(time(NULL));
	return true;
}

// Callback for interacting with the OS. Standard Windows messages.
// Most are just passed on to the appropriate classes.
LRESULT CALLBACK GameApp::MsgProc (HWND hWnd, UINT uMsg, WPARAM wParam, 
			LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext)
{
	LRESULT result;

	switch (uMsg)
	{
		case WM_DEVICECHANGE:
		{
			int event = (int)wParam;
			result = g_App->OnDeviceChange(event);
			break;
		}

		case WM_DISPLAYCHANGE:
		{
			int colorDepth = (int)wParam;
			int width = (int)(short) LOWORD(lParam);
			int height = (int)(short) HIWORD(lParam);

			result = g_App->OnDisplayChange(colorDepth, width, height);
			break;
		}

		case WM_KEYDOWN:
        case WM_KEYUP:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MOUSEWHEEL:
		{
			// Input is passed to the game views for processesing. 
			if (g_App->m_pGame)
			{
				Q3Game *pGame = g_App->m_pGame;
				AppMsg msg;
				msg.m_hWnd = hWnd;
				msg.m_uMsg = uMsg;
				msg.m_wParam = wParam;
				msg.m_lParam = lParam;
				for(GameViewList::reverse_iterator i=pGame->m_viewList.rbegin(); i!=pGame->m_viewList.rend(); ++i)
				{
					if ( (*i)->VOnMsgProc( msg ) )
					{
						result = true;
						break;
					}
				}
			}
			break;
		}

		case WM_CLOSE:
		{
			result = g_App->OnClose();
			break;
		}

		case WM_SYSCOMMAND: 
		{
			result = g_App->OnSysCommand(wParam, lParam);
			if (result)
			{
				*pbNoFurtherProcessing = true;
			}
			break;
		}

		default:
			break;
	}

	return 0;
}

// Used for system command processing.
LRESULT GameApp::OnSysCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
		case SC_CLOSE :
		{

			if (m_Quitting)
				return true;
			// used to close the game
			m_Quitting = true;

			return true;
		}
		return 0;

		default:
			return DefWindowProc(GetHwnd(), WM_SYSCOMMAND, wParam, lParam);
	}

	return 0;
}

// Used to restore the device
HRESULT CALLBACK GameApp::OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void *pUserContext  )
{
	if (g_App->m_pGame)
	{
		GameViewList::iterator it;
		Q3Game* game = g_App->m_pGame;
		for (it = game->m_viewList.begin(); it != game->m_viewList.end(); it++)
		{
			(*it)->VOnRestore();
		}
	}
	return 0;
}

// Called when the device is lost
void CALLBACK GameApp::OnLostDevice(void *pUserContext)
{
	HRESULT hr = D3DERR_DEVICELOST ;
	if (g_App->m_pGame)
	{
		GameViewList::iterator it;
		Q3Game* game = g_App->m_pGame;
		for (it = game->m_viewList.begin(); it != game->m_viewList.end(); it++)
		{
			(*it)->VOnLostDevice();
		}
	}
}


// Checks if the display is useable or not.
bool CALLBACK GameApp::IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	IDirect3D9* pD3D = DXUTGetD3DObject(); 
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                    D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    return true;
}

// Called to update the game AKA the main loop.
void CALLBACK GameApp::OnUpdateGame( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void *pUserContext )
{
	static DWORD lastTime = 0;
	DWORD elapsedTime = 0;
	DWORD now = timeGetTime();
	if (lastTime == 0)
	{
		lastTime = now;
	}

	// Finds the elapsed time from the last call of the main loop.
	elapsedTime = now - lastTime;
	lastTime = now;

	// If it is quiting, post the close message to clean up.
	if (g_App->IsQuitting())
	{
		PostMessage( g_App->GetHwnd(), WM_CLOSE, 0, 0);
		return;
	}

	// Update the game.
	if (g_App->m_pGame)
	{
		safeTick( 20 );
		g_App->m_pGame->OnUpdate(elapsedTime);
	}
}


// Called every time the game needs to be rendered.
void CALLBACK GameApp::OnRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void *pUserContext )
{
	if (g_App->m_pGame)
	{
		GameViewList::iterator it;
		Q3Game* game = g_App->m_pGame;
		for (it = game->m_viewList.begin(); it != game->m_viewList.end(); it++)
		{
			(*it)->VRender(fTime, fElapsedTime);
		}
	}
}

// Changes device settings to what is needed.
bool CALLBACK GameApp::ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
	static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( pDeviceSettings->DeviceType == D3DDEVTYPE_REF )
            DXUTDisplaySwitchingToREFWarning();
    }

    return true;
}

// Used to store the size of the display.
LRESULT GameApp::OnDisplayChange(int colorDepth, int width, int height)
{
	m_rcDesktop.left = 0;
	m_rcDesktop.top = 0; 
	m_rcDesktop.right = width;
	m_rcDesktop.bottom = height;
	m_iColorDepth = colorDepth;

	return 0;
}

// Creates a base game and a human view.
Q3Game* GameApp::CreateGameAndView()
{
	Q3Game* game = SAFE_NEW Q3Game();
	if (game)
	{
		shared_ptr<Q3Map> q = VLoadMap();
		shared_ptr<HumanView> view (SAFE_NEW HumanView());
		view->AddMap(q);

		game->VAddView(view);
		game->AddMap(q);
	}
	return game;
}

shared_ptr<Q3Map> GameApp::VLoadMap()
{
	MapFileParser m;
	m.Init("mpteam9.bsp");

	shared_ptr<Q3Map> q;
	q = m.ReadMap();
	return q;
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////Q3Game//////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

// Listener for Game events.
void ListenForGameEvents(EventListenerPtr listener)
{
	safeAddListener( listener, EventType(Evt_New_Actor::gkName) );
	safeAddListener( listener, EventType(Evt_Remove_Actor::gkName) );
	safeAddListener( listener, EventType(Evt_Move_Actor::gkName) );
	safeAddListener( listener, EventType(Evt_Change_GameState::gkName) );
	safeAddListener( listener, EventType(Evt_Damage_Actor::gkName) );
	safeAddListener( listener, EventType(Evt_Apply_Buff::gkName) );
	safeAddListener( listener, EventType(Evt_Create_Missile::gkName) );
	safeAddListener( listener, EventType(Evt_Left_Click::gkName) );
	safeAddListener( listener, EventType(Evt_Try_Move_Actor::gkName) );
}

// Base constructor, adds listener.
Q3Game::Q3Game()
{
	m_data.m_timeLeftUntilWave = 0;
	m_data.m_waveTimeLimit = 10000;
	m_data.m_curWave = 1;
	m_data.m_curMoney = 6;
	m_data.m_curLife = 10;
	m_LastActorId = 0;
	m_status = Game_Initializing;
	m_curTowerType = -1;
	m_selectedTower = 0;

	EventListenerPtr gameLogicListener (SAFE_NEW GameLogicListener( this) );
	ListenForGameEvents(gameLogicListener);
	m_eventListener = gameLogicListener;
}

// Clears out all actors and flushes process list.
Q3Game::~Q3Game()
{
	while(!m_pActorMap.empty())
	{
		ActorMap::iterator it = m_pActorMap.begin();
		ActorId id = (*it).first;
		m_pActorMap.erase(it);
	}

	m_processManager.DeleteProcessList();
}

// Main game loop.
void Q3Game::OnUpdate(int deltaMS)
{
	// Updates all the views
	for(GameViewList::iterator i=m_viewList.begin(); i!=m_viewList.end(); ++i)
	{
		(*i)->VOnUpdate( deltaMS );
	}

	switch (m_status)
	{
		// Main game running status, updates processes/actors, checks for win/lose condition, spawns waves
		case Game_Running:
			m_processManager.UpdateProcesses(deltaMS);
			for(ActorMap::iterator it=m_pActorMap.begin(); it != m_pActorMap.end(); it++)
			{
				shared_ptr<IActor> actor = it->second;
				actor->VOnUpdate( deltaMS );
			}
			break;
		
		// Starting a new game.
		case Game_Initializing:
			BuildInitialScene();
			safeTriggerEvent(Evt_Change_GameState(Game_Running));
			break;

		case Game_Pause:
			break;
	}	
}

// Creates the basic scene for the game and sets up the tower types.
void Q3Game::BuildInitialScene()
{
	safeTriggerEvent(Evt_RebuildUI());
}

// Adds an actor to the actor list, sends event to add actors elsewhere.
void Q3Game::VAddActor(shared_ptr<IActor> actor)
{
	m_pActorMap[m_LastActorId] = actor;
	actor->VSetId(m_LastActorId);
	m_LastActorId++;
	safeQueueEvent(EventPtr (SAFE_NEW Evt_New_Actor(actor->VGet())));
}

// Changes the game state.
void Q3Game::VGameStatusChange(GameStatus status)
{
	m_status = status;
}

// Removes an actor from the actor list.
void Q3Game::VRemoveActor(ActorId id)
{
	ActorMap::iterator it = m_pActorMap.find(id);
	if (it == m_pActorMap.end())
		return;

	shared_ptr<IActor> actor = (*it).second;
	m_pActorMap.erase(id);
}

// Moves an actor to the new location.
void Q3Game::VMoveActor(ActorId id, const Mat4x4 &m)
{
	if (id < m_LastActorId)
	{
		shared_ptr<IActor> actor = m_pActorMap[id];
		if (actor)
			actor->VSetMat(m);
	}
}

// Adds a view to the view list.
void Q3Game::VAddView(shared_ptr<IGameView> view)
{
	m_viewList.push_back(view);

	shared_ptr<CharacterParams> cp(SAFE_NEW CharacterParams());
	cp->m_Mat = Mat4x4::g_Identity;
	cp->m_viewId = view->VGetId();

	CreateCharacter(cp);
}

// Creates a basic square grid for the base of the map.
void Q3Game::CreateGrid()
{
	shared_ptr<PlaneParams> p (SAFE_NEW PlaneParams());
	p->m_Color = g_White;
	p->m_Texture = "background2.bmp";
	p->m_Mat = Mat4x4::g_Identity;
	p->m_hasTextureAlpha = false;
	p->m_Type = AT_GROUND;
	shared_ptr<IActor> actor (SAFE_NEW Actor(p));
	VAddActor(actor);
}

// Creates a missle to fire at the tower's target.
void Q3Game::CreateMissile(ActorId id)
{

	//create missile here
}

// Gets the pointer to the actor with this id.
shared_ptr<IActor> Q3Game::GetActor(ActorId id)
{
	ActorMap::iterator it = m_pActorMap.find(id);
	shared_ptr<IActor> actor;

	//actor = m_pActorMap[id];

	if (it != m_pActorMap.end())
		actor = it->second;

	return actor;
}

// Deals damage to the actor of this id.
void Q3Game::DamageActor(ActorId id, int damage)
{
	/*if (id > 2)
		m_pActorMap[id]->VTakeDamage(damage);*/
}

// Applys a buff to the actor.
void Q3Game::ApplyBuffToActor(ActorId id, shared_ptr<IBuff> buff)
{
	m_pActorMap[id]->VApplyBuff(buff);
}

// Used when the mouse if right clicked.
void Q3Game::RightClick(Vec3 l)
{

}

float Dist2d(Vec3 a, Vec3 b)
{
	float d = (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
	return d;
}

Vec3 Q3Game::Move(Vec3 start, Vec3 end, float size)
{
	TraceOut out;

	Vec3 outV;
	while (start.SqDistance(end) > 0.1f)
	{
		out = m_map->Trace(start, end, 1, size);

		outV = out.outputEnd;
		if (out.outputFraction != 1.0)
		{
			Vec3 dir = end-start;
			dir.Normalize();

			Vec3 planeNormal(-out.outputPlane.normal[0], -out.outputPlane.normal[1], -out.outputPlane.normal[2]);
			Vec3 invNormal = planeNormal * -1;

			Vec3 proj = (dir.Dot(invNormal) / invNormal.Dot(invNormal)) * invNormal;

			Vec3 wallDir = dir - proj;

			end = outV + wallDir * (start.Distance(end) * (1-out.outputFraction));
			start = outV;
		}
		else
		{
			break;
		}
	}

	return outV;
}

void Q3Game::AttemptActorMove(ActorId id, Mat4x4 m, float deltaMS)
{
	float size = 40.0f;
	shared_ptr<IActor> actor = GetActor(id);
	if (actor)
	{
		Mat4x4 mat = m;
		if (actor->VGet()->m_Mat.GetPosition() != m.GetPosition())
		{
			Vec3 start = actor->VGet()->m_Mat.GetPosition();
			Vec3 end = m.GetPosition()+g_Up*10;

			Vec3 stepMove = Move(start, end, size);

			mat.SetPosition(stepMove);
		}

		safeTriggerEvent(Evt_Move_Actor(id, mat));
	}
}


ActorId Q3Game::CreateCharacter(shared_ptr<CharacterParams> p)
{
	shared_ptr<IActor> character (new Actor(p));
	VAddActor(character);
	ActorId id = character->VGet()->m_Id;
	assert(id >= 0 && "Wrong Character ID");
	return id;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////HumanView//////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

// Listener for human view events.
void ListenForViewEvents(EventListenerPtr listener)
{
	safeAddListener( listener, EventType(Evt_New_Actor::gkName) );
	safeAddListener( listener, EventType(Evt_Move_Actor::gkName) );
	safeAddListener( listener, EventType(Evt_Move_Camera::gkName) );
	safeAddListener( listener, EventType(Evt_Remove_Actor::gkName) );
	safeAddListener( listener, EventType(Evt_Change_GameState::gkName) );
	safeAddListener( listener, EventType(Evt_Shot::gkName ) );
	safeAddListener( listener, EventType(Evt_Remove_Effect::gkName ) );
	safeAddListener( listener, EventType(Evt_Device_Created::gkName) );
	safeAddListener( listener, EventType(Evt_RebuildUI::gkName) );
	safeAddListener( listener, EventType(Evt_Remove_Effect_By_Id::gkName) );
	safeAddListener( listener, EventType(Evt_Mouse_Move::gkName) );
}

// Constructor
HumanView::HumanView():m_lastShot(0)
{
	m_id=0;

	// Starts the audio engine.
	InitAudio();
	
	m_processManager = SAFE_NEW ProcessManager;
	m_pScene.reset( SAFE_NEW HumanPlayerScene());
	m_humanUI.reset( SAFE_NEW HumanUI());

	// Frustum used for culling
	Frustum frustum;
	frustum.Init(D3DX_PI/4.0f, 1.0f, 0.5f, 10000.0f);
	m_pCamera.reset(SAFE_NEW CameraNode(&Mat4x4::g_Identity, frustum));
	assert(m_pScene && m_pCamera && _T("Out of memory"));

	m_pScene->VAddChild(-1, m_pCamera);
	m_pScene->SetCamera(m_pCamera);
	m_status = Game_Initializing;

	EventListenerPtr viewListener (SAFE_NEW GameViewListener( this) );
	ListenForViewEvents(viewListener);
	m_eventListener = viewListener;
	m_pFont = NULL;
	m_pTextSprite = NULL;
}

// Destructor
HumanView::~HumanView()
{
	// Remove each of the screen elements to call it's destructor
	while (!m_screenElementList.empty())	
	{
		ScreenElementList::iterator it = m_screenElementList.begin();
		m_screenElementList.pop_front();
	}

	SAFE_RELEASE( m_pFont );
    SAFE_RELEASE( m_pTextSprite );

	m_music.reset();
	m_processManager->DeleteProcessList();
	SAFE_DELETE(m_processManager);

	// Need to shutdown the audio specially incase of errors
	if (g_Audio)
		g_Audio->VShutdown();

	SAFE_DELETE(g_Audio);
}

// Creates the audio device and gets it read for sounds.
bool HumanView::InitAudio()
{
	SAFE_DELETE(g_Audio);
	g_Audio = SAFE_NEW CDirectSoundAudio();

	if (!g_Audio)
		return false;

	if (!g_Audio->VInitialize(g_App->GetHwnd()))
		return false;

	return true;
}

// Updates processes and the screen elements.
void HumanView::VOnUpdate(int deltaMS)
{
	m_processManager->UpdateProcesses(deltaMS);

	switch (m_status)
	{
		case Game_Initializing:
			BuildInitialScene();
			break;
	}

	for (ScreenElementList::iterator it = m_screenElementList.begin(); it != m_screenElementList.end(); it++)
	{
		if (*it)
			(*it)->VOnUpdate(deltaMS);
	}

	m_controller->OnUpdate(deltaMS);
}

// Sets view id.
void HumanView::VOnAttach(GameViewId vid)
{
	m_id = vid;
}

// Renders the scene from the scene node graph.
void HumanView::VRender(double fTime, float fElapsedTime)
{
	m_currTick = timeGetTime();

	if (m_currTick == m_lastDraw)
		return;

	HRESULT hr;

	// Runs as fast as possible
	if (1)//( (m_currTick - m_lastDraw) > SCREEN_REFRESH_RATE ) )
	{
		// clear the render target
		V( DXUTGetD3DDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
			D3DCOLOR_ARGB(0, 45, 50, 170), 1.0f, 0));

		// render the scenes
		if ( SUCCEEDED(DXUTGetD3DDevice()->BeginScene() ) )
		{
			CDXUTTextHelper txtHelper( m_pFont, m_pTextSprite, 15 );			
			VRenderText(txtHelper);

			m_screenElementList.sort();

			
			for (ScreenElementList::iterator it = m_screenElementList.begin();
				it != m_screenElementList.end(); it++)
			{
				if ((*it)->VIsVisible())
					(*it)->VRender(fTime, fElapsedTime);
			}
			
			m_lastDraw = m_currTick;
		}

		V( DXUTGetD3DDevice()->EndScene());
	}
}

// Renders text to the screen, mostly for debugging purposes
void HumanView::VRenderText(CDXUTTextHelper &txtHelper)
{
	txtHelper.Begin();
	txtHelper.SetInsertionPos( 5, 5 );
	txtHelper.SetForegroundColor( D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1.0f ) );
	std::wstring scoreStr;
	scoreStr.append(DXUTGetFrameStats());
	scoreStr.append(_T("     /     "));
	scoreStr.append(DXUTGetDeviceStats());
	txtHelper.DrawTextLine( scoreStr.c_str() );
	txtHelper.End();
}

// Message handler for Windows messages
LRESULT CALLBACK HumanView::VOnMsgProc( AppMsg msg )
{
	// First checks if any of the screen elements will consume the message
	for(ScreenElementList::iterator it = m_screenElementList.begin();
		it != m_screenElementList.end(); it++)
	{
		if ( (*it)->VIsVisible() )
		{
			if ((*it)->VOnMsgProc( msg) )
			{
				return 1;
			}
		}
	}

	// Passes it on to the input controllers.
	switch (msg.m_uMsg)
	{
		case WM_KEYDOWN:
			if (m_controller)
				m_controller->OnKeyDown(msg.m_wParam);
			break;
        case WM_KEYUP:
			if (m_controller)
				m_controller->OnKeyUp(msg.m_wParam);
			break;
		case WM_LBUTTONDOWN:
			SetCapture(msg.m_hWnd);
			if (m_controller)
				m_controller->OnLButtonDown(CPoint(LOWORD(msg.m_lParam), HIWORD(msg.m_lParam)));
			break;
		case WM_MOUSEMOVE:
			if (m_controller)
				m_controller->OnMouseMove(CPoint(LOWORD(msg.m_lParam), HIWORD(msg.m_lParam)));
			break;
		
		case WM_RBUTTONDOWN:
			SetCapture(msg.m_hWnd);
			if (m_controller)
				m_controller->OnRButtonDown(CPoint(LOWORD(msg.m_lParam), HIWORD(msg.m_lParam)));
			break;

		
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			{
				SetCapture(NULL);
			}
			break;

		case WM_MOUSEWHEEL:
			if (m_controller)
				m_controller->OnMouseScroll((short)HIWORD(msg.m_wParam));
			break;
	}
	return 0;	
}

// Called when the display device is created.
HRESULT HumanView::DeviceCreated(IDirect3DDevice9* device)
{ 
	HRESULT hr;
	m_humanUI->OnDeviceCreate(device); 
	V_RETURN( D3DXCreateFont( device, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                 OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                 L"Arial", &m_pFont ) );

	return S_OK;
}

// Called when the display device is restored.
HRESULT HumanView::VOnRestore()
{
	HRESULT hr;

	if( !m_pFont )
	{
	    // Initialize the font
	    V_RETURN( D3DXCreateFont( DXUTGetD3DDevice(), 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         L"Arial", &m_pFont ) );
	}
	else
	{
       V_RETURN( m_pFont->OnResetDevice());
	}

	if (!m_pTextSprite)
	{
		// Create a sprite to help batch calls when drawing many lines of text
		V_RETURN( D3DXCreateSprite( DXUTGetD3DDevice(), &m_pTextSprite ) );
	}
	else
	{
        V_RETURN( m_pTextSprite->OnResetDevice() );		
	}

	for (ScreenElementList::iterator it = m_screenElementList.begin(); it != m_screenElementList.end(); it++)
	{
		V_RETURN( (*it)->VOnRestore());
	}

	return hr;
}

// Handles losing the display device.
void HumanView::VOnLostDevice()
{
	m_humanUI->OnDeviceLost();
	if( m_pFont )
        m_pFont->OnLostDevice();
    SAFE_RELEASE( m_pTextSprite );
}

// Removes an actor from the scene node.
void HumanView::VRemoveActor(ActorId id)
{
	m_pScene->RemoveChild(id);
}

// Recreates the UI to the correct sizes
void HumanView::RebuildUI()
{
	m_humanUI->BuildInitialDialogs();
}

// Sets up the beginning scene and gets the scene node graph ready.
void HumanView::BuildInitialScene()
{
	//m_music.reset(SAFE_NEW CSoundProcess("music2.ogg",2, 20, true));
	//Attach(m_music);

	VPushScreen(m_pScene);
	VPushScreen(m_humanUI);
	VOnRestore();
}

// Adds a screen to the element list.
void HumanView::VPushScreen(shared_ptr<IScreenElement> e)
{
	m_screenElementList.push_back(e);
}

// Removes a screen from the element list.
void HumanView::VPopScreen()
{
	m_screenElementList.pop_back();
}

// Moves the scene node to the correct location
void HumanView::VMoveActor(ActorId id, Mat4x4 const &mat)
{
    shared_ptr<ISceneNode> node = m_pScene->FindActor(id);
	if (node)
	{
		node->VSetTransform(&mat);
	}
}

// Moves the camera to the changed location.
void HumanView::MoveCamera(Mat4x4 const &change)
{
    shared_ptr<CameraNode> node = m_pScene->GetCamera();
	if (node)
	{
		Mat4x4 mat, tmp;
		//tmp = node->VGet()->ToWorld() * change;
		node->VSetTransform(&change);
	}
}


// Adds a new "shot" effect to the scene node graph.
void HumanView::AddShot(ActorId id, int time, Vec3 start, Vec3 end, std::string texture)
{
	Mat4x4 s,e;
	s.BuildTranslation(start);
	e.BuildTranslation(end);
	shared_ptr<ISceneNode> object (SAFE_NEW ShotNode(id, time, m_lastShot, texture, s, e));
	++m_lastShot;
	m_pScene->AddChild(-1, object);
	object->VOnRestore(&*m_pScene);
}


// Used for the mouse over the towers.
void HumanView::MouseMove(Vec3 pos)
{

}

void HumanView::AddMap(shared_ptr<Q3Map> q)
{
	shared_ptr<ISceneNode> node(q);
	m_pScene->AddChild(-1,node);
}

shared_ptr<SceneNode> HumanView::CreateCharacter(shared_ptr<CharacterParams> params)
{
	shared_ptr<SceneNode> character(SAFE_NEW SceneNode(params->m_Id, "Character", NULL, RenderPass_Actor, &params->m_Mat));
	m_pScene->VAddChild(params->m_Id, character);

	return character;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////HumanInterfaceController///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define RADIANS_TO_DEGREES(x) ((x) * 180.0f / D3DX_PI)
#define DEGREES_TO_RADIANS(x) ((x) * D3DX_PI / 180.0f)

// Default constructor
HumanInterfaceController::HumanInterfaceController(shared_ptr<SceneNode> object, float initialYaw, float initialPitch):m_scrolled(13),m_object(object)
{
	m_object->VGet()->Transform(&m_matToWorld, &m_matFromWorld);

	m_fTargetYaw = m_fYaw = RADIANS_TO_DEGREES(-initialYaw);
	m_fTargetPitch = m_fPitch = RADIANS_TO_DEGREES(initialPitch);

	m_maxSpeed = 300.f;			// 1 meters per second
	m_currentSpeed = 0.0f;

	Vec3 pos = m_matToWorld.GetPosition();

	m_matPosition.BuildTranslation(pos);

	//m_bLeftMouseDown = false;
    POINT ptCursor;
    GetCursorPos( &ptCursor );
	m_lastMousePos = ptCursor;

	memset(m_bKey, 0x00, sizeof(m_bKey));
}

// Checks if any keys are down and does the appropriate action for that key.
void HumanInterfaceController::OnUpdate(int deltaMS)
{
	bool bTranslating = false;
	Vec4 atWorld(0,0,0,0);
	Vec4 rightWorld(0,0,0,0);
	Vec4 upWorld(0,0,0,0);

	m_matToWorld = m_object->VGet()->ToWorld();
	m_matPosition.SetPosition(m_object->VGet()->ToWorld().GetPosition());


	if (m_bKey['W'] || m_bKey['S'])
	{
		// In D3D, the "look at" default is always
		// the positive Z axis.
		Vec4 at = -g_Up4; 
		if (m_bKey['S'])
			at *= -1;

		// This will give us the "look at" vector 
		// in world space - we'll use that to move
		// the camera.
		atWorld = m_matToWorld.Xform(at);
		bTranslating = true;
	}

	if (m_bKey['A'] || m_bKey['D'])
	{
		// In D3D, the "right" default is always
		// the positive X axis.
		Vec4 right = g_Right4; 
		if (m_bKey['A'])
			right *= -1;

		// This will give us the "right" vector 
		// in world space - we'll use that to move
		// the camera
		rightWorld = m_matToWorld.Xform(right);
		bTranslating = true;
	}

	if (m_bKey[' '] || m_bKey['C'] || m_bKey['X'])
	{
		// In D3D, the "up" default is always
		// the positive Y axis.
		Vec4 up = -g_Up4; 
		if (!m_bKey[' '])
			up *= -1;

		//Unlike strafing, Up is always up no matter
		//which way you are looking
		upWorld = up;
		bTranslating = true;
	}

	//Handling rotation as a result of mouse position
	{
		// The secret formula!!! Don't give it away!
		//If you are seeing this now, then you must be some kind of elite hacker!
		m_fYaw += (m_fTargetYaw - m_fYaw) * ( .35f );
		m_fTargetPitch = MAX(-90, MIN(90, m_fTargetPitch));
		m_fPitch += (m_fTargetPitch - m_fPitch) * ( .35f );

		// Calculate the new rotation matrix from the camera
		// yaw and pitch.
		Mat4x4 matRot, matYaw, matPitch;
		matRot.BuildRotationX(D3DX_PI/2);
		matYaw.BuildYawPitchRoll(DEGREES_TO_RADIANS(-m_fYaw), 0,0);
		matPitch.BuildYawPitchRoll(0, DEGREES_TO_RADIANS(m_fPitch),0);
		matRot = matPitch * matYaw * matRot;

		m_matToWorld = matRot * m_matPosition;
		m_matFromWorld = m_matToWorld.Inverse(); 
	}

	if (bTranslating)
	{
		float elapsedTime = (float)deltaMS / 1000.0f;

		Vec3 direction = atWorld + rightWorld;
		direction.Normalize(); 

		// Ramp the acceleration by the elapsed time.
		float numberOfSeconds = 1.f;
		m_currentSpeed += m_maxSpeed *  (elapsedTime) ;
		if (m_currentSpeed > m_maxSpeed)
			m_currentSpeed = m_maxSpeed;

		direction *= m_maxSpeed * elapsedTime;
	/*	Vec3 up = upWorld*400*  (elapsedTime);
		direction = direction + up;*/

		Vec3 pos = m_matPosition.GetPosition() + direction;
		m_matPosition.SetPosition(pos);
		m_matToWorld.SetPosition(pos);

		m_matFromWorld = m_matToWorld.Inverse();
	}
	else
	{
		m_currentSpeed = 0.0f;
	}

	safeTriggerEvent(Evt_Try_Move_Actor(m_object->VGet()->ActorId(), m_matToWorld, deltaMS));
}

Mat4x4 HumanInterfaceController::CalcViewMatrix(float yaw, float pitch)
{
	Vec3 up(0.0f,0.0f,-1.0f);
	Vec3 look(0.0f,1.0f,0.0f);
	Vec3 right(1.0f,0.0f,0.0f);

	Mat4x4 yawMatrix;
	D3DXMatrixRotationAxis(&yawMatrix, &up, yaw);

	D3DXVec3TransformCoord(&look, &look, &yawMatrix); 
	D3DXVec3TransformCoord(&right, &right, &yawMatrix);

	Mat4x4 pitchMatrix;
	D3DXMatrixRotationAxis(&pitchMatrix, &right, pitch);

	//D3DXVec3TransformCoord(&look, &look, &pitchMatrix); 
	//D3DXVec3TransformCoord(&up, &up, &pitchMatrix);

	Mat4x4 viewMatrix;
	D3DXMatrixIdentity(&viewMatrix);

	viewMatrix._11 = right.x; viewMatrix._12 = up.x; viewMatrix._13 = look.x;
	viewMatrix._21 = right.y; viewMatrix._22 = up.y; viewMatrix._23 = look.y;
	viewMatrix._31 = right.z; viewMatrix._32 = up.z; viewMatrix._33 = look.z;

	Vec3 position = m_matToWorld.GetPosition();
	viewMatrix._41 = - D3DXVec3Dot( &position, &right ); 
	viewMatrix._42 = - D3DXVec3Dot( &position, &up );
	viewMatrix._43 = - D3DXVec3Dot( &position, &look );

	return viewMatrix;
}

// Zooms the camera in or out
void HumanInterfaceController::OnMouseScroll(int lines)
{
	int scrollMove = lines / WHEEL_DELTA;
	m_scrolled -= scrollMove;
//	if (m_scrolled <= 26 && m_scrolled >= 0)
//		safeTriggerEvent(Evt_Move_Camera(Vec3(0, -scrollMove, 0)));
//	else
//		m_scrolled += scrollMove;
}

// Finds the location on the map and sends the left click event.
void HumanInterfaceController::OnLButtonDown(const CPoint &mousePos)
{
	Vec3 v1(0,0,0);

	safeTriggerEvent(Evt_Left_Click(v1));
}

// Sends the event to sell a tower at the clicked location
void HumanInterfaceController::OnRButtonDown(const CPoint &mousePos)
{

}

// Sends the mouse move event when the mouse is moving
void HumanInterfaceController::OnMouseMove(const CPoint &mousePos)
{
	if(m_lastMousePos!=mousePos)
	{
		m_fTargetYaw = m_fTargetYaw + (m_lastMousePos.x - mousePos.x);
		m_fTargetPitch = m_fTargetPitch + (mousePos.y - m_lastMousePos.y);
		m_lastMousePos = mousePos;
	}
	return;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////GameLogicListener//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

// Event listener for the game logic, mostly just calls the game's functions.
bool GameLogicListener::HandleEvent(Event const & e)
{
	if (strcmp(e.getType().getName(), Evt_Remove_Actor::gkName)==0)
	{
		EvtData_Remove_Actor *data = e.getData<EvtData_Remove_Actor>();
		m_game->VRemoveActor(data->m_id);
	}
	else
	if (strcmp(e.getType().getName(), Evt_Move_Actor::gkName)==0)
	{
		EvtData_Move_Actor *data = e.getData<EvtData_Move_Actor>();
		m_game->VMoveActor(data->m_id, data->m_Mat);
	}
	else
	if (strcmp(e.getType().getName(), Evt_Try_Move_Actor::gkName)==0)
	{
		EvtData_Try_Move_Actor *data = e.getData<EvtData_Try_Move_Actor>();
		m_game->AttemptActorMove(data->m_id, data->m_Mat, data->m_deltaMS);
		return true;
	}
	else
	if (strcmp(e.getType().getName(), Evt_Change_GameState::gkName)==0)
	{
		EvtData_Change_GameState *data = e.getData<EvtData_Change_GameState>();
		m_game->VGameStatusChange(data->m_state);
	}
	else
	if (strcmp(e.getType().getName(), Evt_Damage_Actor::gkName) == 0 )
	{
		EvtData_Damage_Actor *data = e.getData<EvtData_Damage_Actor>();
		m_game->DamageActor(data->m_id, data->m_damage);
	}
	else
	if (strcmp(e.getType().getName(), Evt_Apply_Buff::gkName) == 0 )
	{
		EvtData_Apply_Buff *data = e.getData<EvtData_Apply_Buff>();
		m_game->ApplyBuffToActor(data->m_buff->VGetActorId(), data->m_buff);
	}
	else
	if (strcmp(e.getType().getName(), Evt_Create_Missile::gkName) == 0 )
	{
		EvtData_Create_Missile *data = e.getData<EvtData_Create_Missile>();
		m_game->CreateMissile(data->m_id);
	}
	else
	if (strcmp(e.getType().getName(), Evt_Left_Click::gkName) == 0 )
	{
		EvtData_Right_Click *data = e.getData<EvtData_Right_Click>();
		m_game->RightClick(data->m_loc);
	}	



	return false;
}

// Event listener for the game view, mostly just calls the view's functions.
bool GameViewListener::HandleEvent(Event const & e)
{
	if (strcmp(e.getType().getName(), Evt_New_Actor::gkName)==0)
	{
		EvtData_New_Actor *data = e.getData<EvtData_New_Actor>();
		// add new actor stuff here.
		switch (data->m_Params->m_Type)
		{
			case AT_CHARACTER:
			{
				shared_ptr<CharacterParams> p (boost::dynamic_pointer_cast<CharacterParams>( data->m_Params));
				shared_ptr<SceneNode> character = m_view->CreateCharacter(p);
				GameViewId temp = p->m_viewId;
				int cpSize = sizeof(CharacterParams);
				int apSize = sizeof(ActorParams);
				if (p->m_viewId == m_view->m_id)
				{
					m_view->m_controller.reset(SAFE_NEW HumanInterfaceController(character, 0, 0));
					m_view->m_pCamera->SetTarget(character);
				}
			}
		}
	}
	else
	if (strcmp(e.getType().getName(), Evt_Remove_Actor::gkName)==0)
	{
		EvtData_Remove_Actor *data = e.getData<EvtData_Remove_Actor>();
		m_view->VRemoveActor(data->m_id);
	}
	else
	if (strcmp(e.getType().getName(), Evt_Move_Actor::gkName)==0)
	{
		EvtData_Move_Actor *data = e.getData<EvtData_Move_Actor>();
		m_view->VMoveActor(data->m_id, data->m_Mat);
	}
	else
	if (strcmp(e.getType().getName(), Evt_Move_Camera::gkName)==0)
	{
		EvtData_Move_Camera *data = e.getData<EvtData_Move_Camera>();
		m_view->MoveCamera(data->m_mat);
	}
	else
	if (strcmp(e.getType().getName(), Evt_Change_GameState::gkName)==0)
	{
		EvtData_Change_GameState *data = e.getData<EvtData_Change_GameState>();
		m_view->VGameStatusChange(data->m_state);
	}
	else
	if (strcmp(e.getType().getName(), Evt_Shot::gkName)==0)
	{
		EvtData_Shot *data = e.getData<EvtData_Shot>();
		m_view->AddShot(data->m_id, data->m_time, data->m_start, data->m_end, data->m_texture);
		shared_ptr<CSoundProcess> sfx (SAFE_NEW CSoundProcess("tada.wav"));
		m_view->Attach(sfx);
	}
	else
	if (strcmp(e.getType().getName(), Evt_Remove_Effect::gkName)==0)
	{
		EvtData_Remove_Effect *data = e.getData<EvtData_Remove_Effect>();

	}
	else
	if (strcmp(e.getType().getName(), Evt_Remove_Effect_By_Id::gkName)==0)
	{
		EvtData_Remove_Effect_By_Id *data = e.getData<EvtData_Remove_Effect_By_Id>();

	}
	else
	if (strcmp(e.getType().getName(), Evt_Device_Created::gkName)==0)
	{
		EvtData_Device_Created *data = e.getData<EvtData_Device_Created>();
		m_view->DeviceCreated(data->m_device);
	}
	else
	if (strcmp(e.getType().getName(), Evt_RebuildUI::gkName) == 0 )
	{
		m_view->RebuildUI();
	}	
	else
	if (strcmp(e.getType().getName(), Evt_Mouse_Move::gkName) == 0 )
	{
		EvtData_Mouse_Move *data = e.getData<EvtData_Mouse_Move>();
		m_view->MouseMove(data->m_pos);
	}


	return false;
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////HumanUI////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

// Constructor for the UI
HumanUI::HumanUI():m_visible(true)
{
	// Changes the default text for the manager
	AddFontResourceEx(L"data/Amiga.ttf",0,0);
	// Default ui for tower switching.
	m_UI.Init(&m_DialogResourceManager);
	m_UI.SetTexture(0, L"data/tmpUI.bmp");
	m_UI.SetCallback( OnGUIEvent);
	m_UI.SetLocation(0,0);
	m_UI.SetFont(0, L"Amiga Forever", 15, 5);

	m_Width = 200;
	m_Height = 30;
	m_PosX = (DXUTGetBackBufferSurfaceDesc()->Width);// - m_Width;
	m_PosY = (DXUTGetBackBufferSurfaceDesc()->Height);// - m_Height;
}

// Destructor for the ui.
HumanUI::~HumanUI()
{
	m_DialogResourceManager.OnLostDevice();
	m_DialogResourceManager.OnDestroyDevice();
}

// Restores the UI after device lost
HRESULT HumanUI::VOnRestore()
{
	m_DialogResourceManager.OnResetDevice();
	// Finds the new location based on the size
	m_PosX = (DXUTGetBackBufferSurfaceDesc()->Width) - (m_Width + 5);
	if (100 > DXUTGetBackBufferSurfaceDesc()->Height)
		m_PosY = (DXUTGetBackBufferSurfaceDesc()->Height)- m_Height;
	else
		m_PosY = 100;
	// Sets the location and size for the ui.
	m_UI.SetLocation(m_PosX,m_PosY);
	m_UI.SetSize(m_Width, m_Height);
	return S_OK;
}

// Renders the UI every frame.
HRESULT HumanUI::VRender(double fTime, float fElapsedTime)
{
	m_UI.OnRender(fElapsedTime);
	return S_OK;
}

// Handles messages from the ui.
LRESULT HumanUI::VOnMsgProc(AppMsg msg)
{
	LRESULT r;
	r = m_UI.MsgProc(msg.m_hWnd, msg.m_uMsg, msg.m_wParam, msg.m_lParam);
	return r;
}

// Called when the UI triggers an event (such as button being pressed)
void CALLBACK HumanUI::OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl *pControl, void* pUserContext )
{
	switch (nControlID)
	{
	default:
		break;
	}
	//PostMessage(g_App->GetHwnd(), 1, 0, nControlID);
}

// Called when the device is created
void HumanUI::OnDeviceCreate(IDirect3DDevice9 * device)
{
	m_DialogResourceManager.OnCreateDevice(device);
}

// Called when the device is lost
void HumanUI::OnDeviceLost()
{
	m_DialogResourceManager.OnLostDevice();
}

// Sets up the location for the dialogs in their initial positions.
void HumanUI::BuildInitialDialogs()
{
}


