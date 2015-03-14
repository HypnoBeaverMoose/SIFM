#pragma once

#include<vector>

class timer {

public: 
	timer(bool start = false);

	void reset();

	bool start(size_t start_stage = 0);

	void stop();
	
	double elapsed();

	void register_stage(size_t& id);

	void set_stage(size_t& event_id);

	double elapsed(size_t& event_id);

private:

	LARGE_INTEGER m_globalBegin;
	LARGE_INTEGER m_globalEnd;
	bool m_started;
	size_t m_currentStage;
	LARGE_INTEGER m_stageBegin;
	LARGE_INTEGER m_frequency;
	std::vector<LARGE_INTEGER> m_stages;	
};