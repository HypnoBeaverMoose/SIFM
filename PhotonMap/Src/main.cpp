#include"Definitions.hpp"
#include"Math.hpp"
#include"Window.hpp"
#include"Experiment.hpp"
int density_photons = 150;
int APIENTRY WinMain( HINSTANCE hInstance,HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	Window::MakeWindow(800, 800, hInstance, "Photon Map", nCmdShow);
	//Window::SetExposure(1);
	Window::SetExposure(55);
	Window::RunExperiments();
	//Window::LoadScene("cornell_tiles.dae", true, 2);		
	//Window::SetPhothonCount(50000);	
	//Window::SetDensityEstimationCount(density_photons);
	//Window::SetFilterParams(true, false, 500, 100.0f, DensityDesc, 0);
	//Window::Run();
	return 0;
}