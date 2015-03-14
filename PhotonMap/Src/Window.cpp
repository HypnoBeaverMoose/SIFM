#include<cimg\CImg.h>

#include"Definitions.hpp"
#include "Math.hpp"
#include "Window.hpp"
#include "timer.h"
#include"ErrorEstimator.hpp"
#include"EnhancedFilter.hpp"
#include"ResolutionExperiment.hpp"
#include"FilterDescExperiment.hpp" 
#include"sigma_experiment.hpp"
#include"knn_experiment.hpp"
#include"quality_experiment.hpp"
#include<sstream>

////////////////////////////////////////////////////////////////////////////////////////
/////////////////WINDOW SPECIFIC DEFINITIONS////////////////////////////////////////////

Window* Window::s_instance = NULL;
std::vector<iexperiment*> Window::s_experiments;
PIXELFORMATDESCRIPTOR Window::s_pfd = {0};

 bool Window::MakeWindow(int width, int heigth, HINSTANCE instance, char* name, int cmdShow)
{

	if(s_instance == NULL)	{
		s_instance = new Window(width, heigth, instance, name, cmdShow);
		return s_instance->m_success;
	}

	return false;
}

 Window::Window(int width,int heigth,HINSTANCE instance, char* name, int cmdShow) 
	 :	m_hInstance(instance), m_cmdShow(cmdShow), m_size(width, heigth), m_drawChild(0),m_ctrlPressed(false), 
		m_debugView(true), 
		m_drawCamRays(false), 
		m_drawOctTree(false),
		m_photonMode(PM_POINTS),
		m_drawMode(DM_WIREFRAME),
		m_showFilterTex(false),
		m_selectedTile(0)
{
	m_success = CreateGLWindow(name, width,  heigth);
	
}

Window::~Window(void)
{
	if (m_hRC)
	{
		if (!wglMakeCurrent(NULL,NULL))	{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(m_hRC)) {
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		m_hRC=NULL;
	}

	if (m_hDC && !ReleaseDC(m_hWnd,m_hDC)) {
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		m_hDC=NULL;										// Set DC To NULL
	}

	if (m_hWnd && !DestroyWindow(m_hWnd)) {
		MessageBox(NULL,"Could Not Release m_hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		m_hWnd=NULL;										// Set m_hWnd To NULL
	}

	if (!UnregisterClass("Photon Mapping",m_hInstance))	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		m_hInstance=NULL;								
	}
}

bool Window::CreateGLWindow(char* title, int width, int height)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left =  (long)0;			// Set Left Value To 0
	WindowRect.right = (long)width;		// Set Right Value To Requested Width
	WindowRect.top  = (long)0;				// Set Top Value To 0
	WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height

	ZeroMemory(&wc,sizeof(WNDCLASS));
	wc.style			= CS_HREDRAW | CS_VREDRAW;				// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= WndProc;								// WndProc Handles Messages
	wc.hInstance		= m_hInstance;							// Set The Instance
	wc.lpszClassName	= "Photon Mapping";						// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK | MB_ICONEXCLAMATION);
		return FALSE;											
	}	

	dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
	dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(m_hWnd=CreateWindowEx(dwExStyle,							// Extended Style For The Window
								wc.lpszClassName,					// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								400, 150,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								m_hInstance,						// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	static	PIXELFORMATDESCRIPTOR _pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		32,											// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		32,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	s_pfd = _pfd;

	if (!(m_hDC=GetDC(m_hWnd)))							// Did We Get A Device Context?
	{
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								
	}

	if (!(PixelFormat=ChoosePixelFormat(m_hDC,&s_pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								
	}

	if(!SetPixelFormat(m_hDC,PixelFormat,&s_pfd))		// Are We Able To Set The Pixel Format?
	{
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								
	}

	if (!(m_hRC=wglCreateContext(m_hDC)))				// Are We Able To Get A Rendering Context?
	{
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								
	}

	if(!wglMakeCurrent(m_hDC,m_hRC))					
	{
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							
	}

	GLenum glewErr = glewInit();
	if(glewErr != GLEW_OK)
	{
		MessageBox(NULL,(char*)glewGetErrorString(glewErr),"ERROR",MB_OK|MB_ICONEXCLAMATION);
	}
	OnCreate();
	OnResize(width, height);

	ShowWindow(m_hWnd,SW_SHOW);
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);

	return TRUE;		
}
void Window::SetPhothonCount(uint count) {	
	
	s_instance->m_photonCount = count;
}
void Window::SetDensityEstimationCount(uint count) {

	s_instance->m_dePhotons = count;
}

void  Window::SetExposure(double exp) {

	PhotonMap::globalExposure = (float)exp;
}

void Window::SetFilterParams(bool useTree, bool useKnn, float searchParam, double param, int Descriptor, int Metric) {
	
	if(s_instance) {
		s_instance->m_scene.setFilterParams(useTree, useKnn, searchParam, param, Descriptor, Metric);
	}
}

bool Window::LoadScene(const char* filename, bool loadTiles, size_t splitIter) {
	
	if(s_instance) {
		s_instance->m_scene.LoadFromFile(filename, loadTiles, splitIter);
		s_instance->m_scene.getMainCamera()->setAspect(s_instance->m_size.x / (float)(s_instance->m_size.y));
		//debug("fov: %f", s_instance->m_scene.getMainCamera()->getFov());
		return true;
	}
	return false;

}

void Window::RunExperiments() {

	if(s_instance) {
		std::vector<float> gt, test;
		//s_instance->load_image(gt,"results/prgoressive2.bmp");
		//s_instance->load_image(test,"experiments/sigma/knn_50_0.000001.bmp");

		//double mse = 0;

		//for(size_t i = 0 ; i < gt.size(); i++) {
		//	mse += (gt[i] - test[i]) * (gt[i] - test[i]);
		//}
			
		//mse = mse / gt.size();

		size_t photon_counts[7] = {1000, 10000, 50000, 100000, 250000, 500000, 1000000};

		parameters params; 
		params.resolution = 5;
		//params.scene = "cornell_tiles.dae";
		//params.alpha = 0.4f;
		//params.knn_count = 20;
		//params.path = "experiments/quality_2_1";
		//s_instance->load_image(gt,"results/prgoressive2.bmp");
		//quality_experiment*  qe = new  quality_experiment(params, &(s_instance->m_raytracer), &(s_instance->m_scene),gt,8);
		//s_experiments.push_back(qe);
		
		//params.scene = "cornell_tiles.dae";
		//params.alpha = 0.4f;
		//params.knn_count = 20;
		//params.path = "experiments/quality_3_1";
		//s_instance->load_image(gt,"results/prgoressive2.bmp");
		//quality_experiment*  qe2 = new  quality_experiment(params, &(s_instance->m_raytracer), &(s_instance->m_scene),gt,8);
		//s_experiments.push_back(qe2);
		
		params.scene = "cornell_aligned.dae";
		params.alpha = 0.4f;
		params.knn_count = 20;
		params.path = "experiments/quality_4_1";
		s_instance->load_image(gt,"results/progressive_2.bmp");
		quality_experiment*  qe = new  quality_experiment(params, &(s_instance->m_raytracer), &(s_instance->m_scene),gt,8);
		s_experiments.push_back(qe);

		////resolution non-filtered
		//parameters params; params.scene = "cornell_tiles.dae";
		//params.photon_count = 1000000; 	
		//params.density_photon_count = 50;
		//params.knn_count = 100;
		//params.path = "experiments/sigma_1_1";
		//s_instance->load_image(gt,"results/prgoressive2.bmp");
		//sigma_experiment*  se = new  sigma_experiment(params, &(s_instance->m_raytracer), &(s_instance->m_scene),gt,8);
		//s_experiments.push_back(se);

	/*	params.photon_count = 400000; 	
		params.density_photon_count = 500;
		params.knn_count = 100;
		params.path = "experiments/sigma_1_2";
		s_instance->load_image(gt,"results/prgoressive2.bmp");
		sigma_experiment*  se2 = new  sigma_experiment(params, &(s_instance->m_raytracer), &(s_instance->m_scene),gt,8);
		s_experiments.push_back(se2);
*/

		//for(size_t i = 0; i < 4; i++) {		
		//	params.photon_cout = 1000 * (size_t)glm::pow(10.0f, (float)i); params.file_suffix = "";
		//	params.path = "experiments/resolution_" + std::to_string(params.photon_cout) ;
		//	iexperiment* exp = new resolution_experiment(params,&(s_instance->m_raytracer),&(s_instance->m_scene),30, 8);
		//	s_experiments.push_back(exp);
		//}	
		//std::vector<float> ground_truth;
		//s_instance->load_image(ground_truth,"results/prgoressive.bmp");

		////resolution filtered
		//params.density_photon_count = 200; 	
		//params.tile_scale = 4;
		//params.resolution = 2;
		//params.sigma = 0.005;
		//params.knn_count = 30;
		//params.photon_cout = 1000000;	
		//params.path = "experiments/filtered_res";
		//for(size_t res= 2 ; res<= 4; ++res) {
		//	params.file_suffix = "resolution_"+std::to_string(res);
		//	params.resolution = res;
		//	iexperiment* exp = new filter_desc_experiment(params,&(s_instance->m_raytracer),&(s_instance->m_scene), ground_truth, 8);
		//	s_experiments.push_back(exp);
		//}
		//tile-size filtered
		//params.density_photon_count = 150; 	
		//params.resolution = 0;
		//params.sigma = 0.01;
		//params.knn_count = 10;
		//params.photon_count = 100000;	
		//params.path = "experiments/filtered_tiles";

		//for(size_t tile_scale= 0 ; tile_scale <= 4; ++tile_scale) {
		//	params.tile_scale = tile_scale;
		//	for(size_t i= 0 ; i < 7; ++i) {
		//		params.file_suffix = "tile_scale_"+std::to_string(tile_scale) + "_photons_"+std::to_string(photon_counts[i]);
		//		params.photon_count = photon_counts[i];			
		//		iexperiment* exp = new filter_desc_experiment(params,&(s_instance->m_raytracer),&(s_instance->m_scene), ground_truth, 8);
		//		s_experiments.push_back(exp);
		//	}
		//}


		std::thread([] () {
				std::lock_guard<std::mutex> l(Window::s_instance->m_mutex);				
				std::for_each(Window::s_experiments.begin(), Window::s_experiments.end(), [] (iexperiment* exp) { exp->execute(); } );
		}).detach();

		s_instance->MainLoop();
	}

}

int Window::Run()
{	
	if(s_instance) {
			return s_instance->MainLoop();
	}
	return -1;
}

int Window::MainLoop()
{	
	///Window* l_instance = s_instance;
	ShowWindow(s_instance->m_hWnd, m_cmdShow);
	UpdateWindow(m_hWnd);
	
	while(true)
	{
		if (PeekMessage(&m_msg, NULL, 0, 0, PM_REMOVE))	
		{
			TranslateMessage(&m_msg);				
			DispatchMessage(&m_msg);

			if (m_msg.message == WM_QUIT)
				break;
		}
		else
		{
			OnRender();
			SwapBuffers(m_hDC);
		}
	}
	return (m_msg.wParam);
}
LRESULT CALLBACK Window::WndProc(	HWND	hWnd,			// Handle For This Window
									UINT	uMsg,			// Message For This Window
									WPARAM	wParam,			// Additional Message Information
									LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
		case WM_KEYDOWN:
			s_instance->OnKeyboardDown(wParam); break;
		case WM_KEYUP:
			s_instance->OnKeyboardUp(wParam); break;
		case WM_MOUSEMOVE:
			s_instance->OnMouseMove(LOWORD(lParam), HIWORD(lParam), wParam);
			break;
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
			s_instance->OnMouseButtonDown(LOWORD(lParam), HIWORD(lParam), wParam); break;
		case WM_SIZE:
			s_instance->OnResize(LOWORD(lParam),HIWORD(lParam));
			break;
		case WM_CLOSE:								
			PostQuitMessage(0);						
			return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam); 
}

////////////////////////////////////////////////////////////////////////////////////////
//////APP SPECIFIC DEFINITIONS///////////////////////////////////////////////////////

	
void Window::OnCreate() {

	m_renderQuad.push_back(Vec4f(-1,-1,0,1));
	m_renderQuad.push_back(Vec4f(-1, 1,0,1));
	m_renderQuad.push_back(Vec4f( 1, 1,0,1));
	m_renderQuad.push_back(Vec4f( 1, -1,0,1));

	m_renderUV.push_back(Vec2f(0,0));
	m_renderUV.push_back(Vec2f( 1,0));	
	m_renderUV.push_back(Vec2f( 1, -1));
	m_renderUV.push_back(Vec2f(0, -1));

	m_eye = Vec3f(0,0,-1) * 10.0f;
	m_center = Vec3f(0,0,0);
	m_up = Vec3f(0,1,0);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	m_raytracer.init(m_size);
}

void Window::OnRender() {

	glClearColor(0.8f, 0.8f, 0.8f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	static int test = 0;
	if(m_debugView) {	
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		//glMultMatrixf(glm::value_ptr(glm::inverse(m_scene.getMainCamera()->tranformation())));;
		gluLookAt(m_eye.x, m_eye.y, m_eye.z, m_center.x, m_center.y, m_center.z, m_up.x, m_up.y, m_up.z);
		m_scene.filter()->draw_descriptors();

		//if(m_photonMode == PM_POINTS)
		//	m_scene.photonMap()->drawPhotons();
		//if(m_photonMode == PM_DESCRIPTORS)
			

		//if(m_drawCamRays)
		//	m_raytracer.drawRays();
		//if(m_drawOctTree)
		//	m_scene.drawOcttree(m_drawChild);
		renderNodes(m_scene.getRoot());
		if(m_selectedTile != NULL)  {
			glEnable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			Tile* t = static_cast<Tile*>(m_selectedTile);
			Vec3f col = t->mesh()->color();
			t->mesh()->set_color(Vec3f(1,1,0));
			glPushMatrix();
			glMultMatrixf(glm::value_ptr(t->mesh()->tranformation()));
			uint drawMode = t->mesh()->drawMode();
			t->mesh()->setDrawMode(DMode::DM_FLAT);
			t->mesh()->draw();
			t->mesh()->setDrawMode(drawMode);
			t->mesh()->set_color(col);
			glPopMatrix();
			for(size_t i = 0 ; i < t->similarTilesCount(); i++) {
				Tile* tile = std::get<0>(t->similarTile(i));
				Vec3f col = t->mesh()->color();
				tile->mesh()->set_color(Vec3f(1,1,0) * (float)std::get<1>(t->similarTile(i)));
				glPushMatrix();
				glMultMatrixf(glm::value_ptr(tile->mesh()->tranformation()));
				uint drawMode = tile->mesh()->drawMode();
				tile->mesh()->setDrawMode(DMode::DM_FLAT);
				tile->mesh()->draw();
				tile->mesh()->setDrawMode(drawMode);
				tile->mesh()->set_color(col);
				glPopMatrix();
			}
		}
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
	}
	else {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		m_raytracer.framebuffer().bindFramebuffer();
		glColor3f(1,1,1);
		glBegin(GL_QUADS);
			for(int i = 0;i < (int)m_renderQuad.size();i++) {
				glVertex4fv(glm::value_ptr(m_renderQuad[i]));
				glTexCoord2fv(glm::value_ptr(m_renderUV[i]));
			}
		glEnd();
	}
//	if(m_raytracer.is_working())
		m_raytracer.framebuffer().updateTexure();
}
void Window::renderNodes(SceneNode* node) {
	
	glPushMatrix();
	glMultMatrixf(glm::value_ptr(node->transform()));
	if(node->getEntity())
		node->getEntity()->draw();
	glPopMatrix();	

	for(int  i = 0; i <(int)node->getChildrenCount(); i++) {
		SceneNode* n = node->getChild(i);
		renderNodes(n);
	}
}
void Window::OnResize(uint width, uint height) {
	
	m_size = Vec2i(800,800);
	glViewport(0,0,m_size.x, m_size.y);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(-1.0f,1.0f,-1.0,1.0,-1.0f,1000);
	gluPerspective(45.0f, m_size.x / (double)(m_size.y), 1, 10000);
}

void Window::OnKeyboardDown(uint keyCode) {


	switch (keyCode)
	{
		case  VK_SPACE:
			m_scene.subdivide();
			break;
		case VK_F1 : 
			toggleDebugWindow(!m_debugView);
			break;
		case VK_F2:		
			toggleDrawMode();
			break;
		case VK_F3:
			m_photonMode = (PMode)(((int)m_photonMode + 1) %  (int)PM_SIZE);
		case VK_F4:
			m_drawOctTree = !m_drawOctTree;
			break;
		case VK_F5:
			std::thread([] (Window* w) {
								std::lock_guard<std::mutex> l(w->m_mutex);
								time_t time = clock();
								w->m_raytracer.start_photon_shoot(w->m_photonCount,&(w->m_scene), 4);								
								w->m_scene.buildPhotonMap();						
								debug("photon shoot time: %f\n",(clock() - time) / (float)CLOCKS_PER_SEC);
			},this).detach();
			break;
		case VK_F6: 
			std::thread([] (Window* w) {
								std::lock_guard<std::mutex> l(w->m_mutex);
								w->m_raytracer.start_photon_shoot(w->m_photonCount,&(w->m_scene), 1);
								w->m_scene.buildPhotonMap();
								w->m_scene.filterScene(3);							
			},this).detach();		
			break;
		case VK_F7:
			m_scene.set_textures();
			m_showFilterTex = !m_showFilterTex;
			toggleDebugWindow(m_debugView);
			break;
		case VK_F8: 
			std::thread([] (Window* w) {
				std::lock_guard<std::mutex> l(w->m_mutex);				
				w->raytraceScene(false, false);
				w->save_framebuffer("results/raytraced.bmp");
			},this).detach();
			break;
		case VK_F9:		
			//m_raytracer.start_photon_shoot(m_photonCount,&(m_scene), 8);
			//renderDensity();
			save_framebuffer("results/unfiltered.bmp");
			std::thread([] (Window* w) {
				std::lock_guard<std::mutex> l(w->m_mutex);				
				//w->m_raytracer.start_photon_shoot(w->m_photonCount,&(w->m_scene), 8);
				w->renderDensity();
				w->save_framebuffer("results/unfiltered.bmp");
			},this).detach();
			break;
		case VK_F11:
			std::thread([] (Window* w) {
				std::lock_guard<std::mutex> l(w->m_mutex);				
				//w->m_raytracer.start_progressive_render(&w->m_scene,0.5f,w->m_photonCount,w->m_dePhotons,-1,8);
				w->m_raytracer.start_filtered_render(&w->m_scene,w->m_photonCount, w->m_dePhotons, 8);
				
			}, this).detach();
			break;
		case VK_F12 :	
			std::thread([] (Window* w) {
				std::lock_guard<std::mutex> l(w->m_mutex);		
				
				w->m_raytracer.start_photon_shoot(w->m_photonCount,&(w->m_scene), 8);
				w->m_scene.buildPhotonMap();
				w->m_raytracer.generate_hit_points(&(w->m_scene),density_photons,8);
				w->m_scene.filterScene(3);							
				w->m_raytracer.start_density(density_photons,&(w->m_scene), true, 8);
				w->save_framebuffer("results/filtered.bmp");
			},this).detach();
	
			break;
		case VK_OEM_MINUS: 
			PhotonMap::globalExposure-=1;
			m_raytracer.framebuffer().updateTexure();
			break;
		case VK_OEM_PLUS: 
			PhotonMap::globalExposure+=1;
			m_raytracer.framebuffer().updateTexure();
			break;
		case VK_CONTROL:
			m_ctrlPressed = true;
			break;
	}
}

void Window::OnKeyboardUp(uint keyCode) {

	switch (keyCode)
	{
		case VK_CONTROL:
			m_ctrlPressed = false;
			break;
	}
}
void Window::save_framebuffer(std::string filename) {

	m_raytracer.framebuffer().save_framebuffer(filename);
}
void Window::toggleDrawMode() {

	m_drawMode = (DMode)(((int)m_drawMode + 1) %  (int)DM_SIZE);
	for(uint i = 0; i < m_scene.geMeshCount(); i++) {
		m_scene.getMesh(i)->setDrawMode(m_drawMode);
	}
}

void Window::toggleDebugWindow(bool show) {
	
	m_debugView = show;
	if(show) {
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0f, m_size.x / (double)(m_size.y), 1, 10000);
		glBindTexture(GL_TEXTURE_2D, 0);	
		//if(m_showFilterTex)
		//	glEnable(GL_TEXTURE_2D);
		//else
		//	glDisable(GL_TEXTURE_2D);
	}
	else {
		glEnable(GL_TEXTURE_2D);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();			
	}

}

void Window::raytraceScene(bool lighting, bool texturing) {

	m_raytracer.start_raytracing(&m_scene, lighting, texturing, 8);
}
 void Window::renderDensity() {
 

	//m_scene.buildPhotonMap();
	m_raytracer.start_density(density_photons, &m_scene, false, 8);
	m_raytracer.framebuffer().updateTexure();
 }


void Window::OnMouseButtonDown(uint posx, uint posy, uint button) {


	if(m_ctrlPressed && ((button & VK_LBUTTON) == VK_LBUTTON)) {

		m_mousePos = Vec2i(posx,posy);
		IFace* res_face;
		float aspect = m_size.x / (float)(m_size.y);
		Mat4f inv_proj = glm::inverse(glm::perspective<float>(glm::radians(45.0f), aspect, 1.0f, 100000));
		Mat4f transform = glm::inverse(glm::lookAt(m_eye, m_center, m_up));
	
		Vec4f origin; Vec3f dir;
		float inv_resX  = 1.0f /(float)m_size.x;
		float inv_resY  = 1.0f /(float)m_size.y;

		origin.x = 2 * posx * inv_resX - 1.0f;
		origin.y = 2 * (m_size.y - posy) * inv_resY - 1.0f;
		origin.z = 0;
		origin.w = 1;
		origin = inv_proj * origin; origin/=origin.w;		
	
		//dir = Vec3f(origin);
		dir = glm::normalize(Vec3f(transform * glm::normalize(Vec4f(origin.x, origin.y, origin.z,0) ))); 
		origin = transform * origin;
		float t = m_scene.traceRay(Vec3f(origin), dir, &res_face); 

		if(t < FLT_MAX && res_face->type() == TILE)
			m_selectedTile  =res_face;
		else
			m_selectedTile = 0;
	}
}

void Window::OnMouseMove(uint posx, uint posy, uint button) {
	
	Vec3f vec = m_eye - m_center;
	float dist = glm::length(vec);
	glm::normalize(vec);

	Vec2i delta = m_mousePos - Vec2i(posx, posy);
	Vec3f right = glm::normalize(glm::cross(m_up, vec));
	Mat4f mat(1.0f);
	Vec4f tmp;
	switch (button)
	{
		
	case MK_LBUTTON: 
		m_angles.x -= delta.x; m_angles.y += delta.y;
		m_eye = Vec3f(Vec4f(0,0,-1,0)* glm::rotate(glm::radians(m_angles.y),Vec3f(1,0,0)) * 
									glm::rotate(glm::radians(m_angles.x),Vec3f(0,1,0))) * dist + m_center;
		m_up = Vec3f(Vec4f(0,1,0,0) * glm::rotate(glm::radians(m_angles.y),Vec3f(1,0,0)) * 
									glm::rotate(glm::radians(m_angles.x),Vec3f(0,1,0)));
		break;
	case MK_RBUTTON: 
		m_eye = m_eye - vec * glm::min(-(float)delta.y / (float)m_size.y, dist + 0.1f) * 30.0f;
		break;		
	case MK_MBUTTON: 
		m_eye = m_eye - (m_up * (delta.y / (float)m_size.y) - right * (delta.x / (float)m_size.x)) * 10.0f;
		m_center = m_center - (m_up * (delta.y / (float)m_size.y) - right * (delta.x / (float)m_size.x)) * 10.0f;
		break;
	default:
		break;
	}
	m_mousePos = Vec2i(posx,posy);
}

void Window::load_image(std::vector<float>& img, const std::string& filename){

	cimg_library::CImg<byte> image(filename.c_str());
	
	for(int y = image.height(); y  > 0 ; --y) {
		for(size_t x = 0; x  < image.width(); ++x) {
			img.push_back((float)*(image.data(x,y,0,0)) / (float)(PhotonMap::globalExposure * 255));
			img.push_back((float)*(image.data(x,y,0,1)) / (float)(PhotonMap::globalExposure * 255));
			img.push_back((float)*(image.data(x,y,0,2)) / (float)(PhotonMap::globalExposure * 255));
		}
	}
}


void Window::OnDestroy() {

}