#include"Definitions.hpp"

#include"timer.h"
#include<algorithm>


timer::timer(bool start) : m_started(start) {

	LARGE_INTEGER li = {0};
	m_stages.push_back(li);
	QueryPerformanceFrequency(&m_frequency);
	m_currentStage = 0;

	if(m_started) {
		QueryPerformanceCounter(&m_globalBegin);
		m_globalEnd = m_globalBegin;
	}
}

void timer::reset()  {

	m_started = false;
	for(size_t i = 0; i < m_stages.size(); i++) {
		m_stages[i].QuadPart = 0;
	}

	m_globalBegin.LowPart = m_globalBegin.HighPart  = 0;
}

bool timer::start(size_t start_stage) {
	
	if(start_stage < m_stages.size())
		m_currentStage = start_stage;
	if(	QueryPerformanceCounter(&m_globalBegin) == TRUE) {
		m_globalEnd = m_globalBegin;
		m_stageBegin = m_globalBegin;
		m_started = true;
	}

	return m_started;

}

void timer::stop() {

	if(m_started) {
		QueryPerformanceCounter(&m_globalEnd);
		m_stages[m_currentStage].QuadPart+= m_globalEnd.QuadPart -  m_stageBegin.QuadPart;
		m_started = false;
	}
}
double timer::elapsed() {

	LARGE_INTEGER li = {0};
	for(size_t i = 0 ; i < m_stages.size(); i++) {
		li.QuadPart +=m_stages[i].QuadPart;
	}
	return li.QuadPart / (double)m_frequency.QuadPart;
	//return (m_globalEnd.QuadPart - m_globalBegin.QuadPart) / (double)m_frequency.QuadPart;
}

double timer::elapsed(size_t& event_id) {

	return m_stages[event_id].QuadPart  / (double)(m_frequency.QuadPart);
}

void timer::register_stage(size_t& event_id) {
	LARGE_INTEGER li = {0};
	m_stages.push_back(li);
	event_id = m_stages.size() - 1;
}

void timer::set_stage(size_t& event_id) {
	
	if(event_id > m_stages.size()) event_id = 0;

	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	m_stages[m_currentStage].QuadPart += li.QuadPart - m_stageBegin.QuadPart;
	m_currentStage = event_id;
	m_stageBegin = li;
}

