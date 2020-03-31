#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>

class Trace
{
	//For now all strings
	std::string m_session_id;

	//could just be a number?! probably not...
	std::string m_ip_address;

	//Lookup table?
	std::string m_browser;

	//These are actually a pair
	std::vector<std::string> m_visited_sites;
	std::vector<std::string> m_visited_time;

public:
	//When constructing a trace, we 
	Trace(std::string& str);
	void ExportToJSON(std::ofstream& oss, bool is_final_entry);

	const char* GetIP() { return this->m_ip_address.c_str(); };
	const char* GetSessionID() { return this->m_session_id.c_str(); };
	std::vector<std::string>* GetVisitedSites() { return &this->m_visited_sites; };
	std::vector<std::string>* GetVisitedTimes() { return &this->m_visited_time; };
private:
	void ExtractMultipleTagValues(std::vector<std::string>& vec, std::string str, std::string tag);
	std::string ExtractTagValue(std::string& str, std::string tag, int& end_pos);
};