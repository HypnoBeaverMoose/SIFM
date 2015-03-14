#pragma once

#include "Framebuffer.hpp"
#include "SceneNode.hpp"
#include "Mesh.hpp"
#include "Camera.hpp" 
#include "Light.hpp "
#include "Entity.hpp"
#include "Octree.hpp"
#include "Scene.hpp"
#include "Photon.hpp"
#include "Raytracer.hpp"
#include <mutex>

class iexperiment;

class Window 
{

public:	
	static  bool	MakeWindow(int width, int heigth, HINSTANCE instance, char* name, int cmdShow);
	static  bool	LoadScene(const char* filename, bool loadTiles = false, size_t splitIter = 0);
	static	void	SetPhothonCount(uint count);
	static	void	SetDensityEstimationCount(uint count);
	static	void	SetExposure(double exp);
	static	void	SetFilterParams(bool useTree, bool useKnn, float searchParam, double param, int Descriptor, int Metric);
	static	void	AddExperiment(iexperiment* exp) { s_experiments.push_back(exp); }
	static	void	RunExperiments();
	static	int		Run();


	~Window();

private:
	
	void OnCreate();

	void OnRender();

	void OnResize(uint width, uint height);

	void OnMouseButtonDown(uint posx, uint posy, uint button);

	void OnMouseMove(uint posx, uint posy, uint button);

	void OnKeyboardDown(uint keyCode);

	void OnKeyboardUp(uint keyCode);

	void OnDestroy();

	void renderNodes(SceneNode* node);

	void raytraceScene(bool lighting, bool texturing);

	void renderDensity();

	void save_descriptors();

	void load_image(std::vector<float>& img, const std::string& filename);

protected:
			Window(int width,int heigth,HINSTANCE instance, char* name, int cmdShow);	
	//		Window(const Window& other);
	//Window	operator=(const Window& other);
	bool	CreateGLWindow(char* title, int width, int height);
	int		MainLoop();
	

protected:
	//static	Window* const getInstance();
	static	LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	void toggleDrawMode();
	void toggleDebugWindow(bool show);
	void save_framebuffer(std::string filename);

private:

	static	Window*	s_instance;
	static	PIXELFORMATDESCRIPTOR s_pfd;
	static	std::vector<iexperiment*> s_experiments;
private:
	MSG				m_msg;			// Windows Message Structure
	HDC				m_hDC;			// Private GDI Device Context
	HGLRC			m_hRC;			// Permanent Rendering Context
	HWND			m_hWnd;			// Holds Our Window Handle
	HINSTANCE		m_hInstance;	// Holds The Instance Of The Application
	int				m_cmdShow;
	bool			m_success;	

	Vec3f		m_up;
	Vec3f		m_eye;
	Vec3f		m_center;	
	Vec3f		m_angles;
	Vec2i		m_size;
	Vec2i		m_mousePos;
	

	std::vector<Vec4f>	m_renderQuad;
	std::vector<Vec2f>	m_renderUV;	
	
	Raytracer			m_raytracer;
	Scene				m_scene;


	uint		m_photonCount;
	uint		m_dePhotons;

	bool		m_debugView;
	bool		m_drawCamRays;
	bool		m_drawOctTree;
	bool		m_showFilterTex;
	bool		m_ctrlPressed;
	uint		m_drawChild;
	PMode		m_photonMode;
	DMode		m_drawMode;	
	std::mutex  m_mutex;
	IFace*		m_selectedTile;
	
};