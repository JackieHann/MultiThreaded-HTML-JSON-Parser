#pragma once
#include "stdafx.h"
#include "Trace.h"

Trace::Trace(std::string& str)
{
	//Going for speed, so can ignore a completely abstract approach

	int t;
	m_session_id = ExtractTagValue(str, "sessionid", t);
	m_ip_address = ExtractTagValue(str, "ipaddress", t);
	m_browser = ExtractTagValue(str, "browser", t);

	ExtractMultipleTagValues(m_visited_sites, str, "path");
	ExtractMultipleTagValues(m_visited_time, str, "time");

}

void Trace::ExportToJSON(std::ofstream& oss, bool is_final_entry)
{
	//Using printf buffers was slower than using string concatenation.
	//const int size = 300;
	//char buffer[size];
	//int cx;
	//cx = snprintf(buffer, sizeof(buffer), "\t\t\"sessionid\": \"%s\",\n\t\t\"ipaddress\": \"%s\",\n\t\t\"browser\": \"%s\",\n\t\t\"path\": [\n", m_session_id.c_str(), m_ip_address.c_str(), m_browser.c_str());
	//oss << buffer;

	oss << "\t\"entry\": {\n\t\t\"sessionid\": \"" << m_session_id.c_str() << "\",\n\t\t\"ipaddress\": \"" << m_ip_address.c_str() << "\",\n\t\t\"browser\": \"" << m_browser.c_str() << "\",\n\t\t\"path\": [\n";

	std::string timeStr = "\t\t\"time\": [\n";
	const int site_size = m_visited_sites.size();
	for (int i = 0; i < site_size; ++i)
	{
		bool do_end_comma = (i != (m_visited_sites.size() - 1));

		oss << "\t\t\t\"" << m_visited_sites[i].c_str() << "\"";
		std::string temp = "\t\t\t\"" + m_visited_time[i] + "\"";
		if (do_end_comma)
		{
			oss << ",\n";
			temp += ",\n";
		}
		else
		{
			oss << "\n\t\t],\n";
			temp += "\n\t\t]\n";
		}
		timeStr += temp;
	}

	oss << timeStr.c_str();
	if (!is_final_entry)
		oss << "\t},\n";
	else
		oss << "\t}\n}";

}

void Trace::ExtractMultipleTagValues(std::vector<std::string>& vec, std::string str, std::string tag)
{
	bool eof = false;
	while (!eof)
	{
		int end_pos = -1;
		std::string val = ExtractTagValue(str, tag, end_pos);
		if (val != "")
		{
			vec.push_back(val);
			str = str.substr(end_pos);
		}
		else
		{
			eof = true;
		}
	}
}

std::string Trace::ExtractTagValue(std::string& str, std::string tag, int& end_pos)
{
	//Simple function that returns
	size_t first_occurrence = str.find("<" + tag + ">");
	size_t last_occurrence = str.find("</" + tag + ">");

	const int first_tag_size = tag.length() + 2;

	//Ie did we find tags within this?
	if (first_occurrence != std::string::npos && last_occurrence != std::string::npos)
	{
		const int start_idx = first_occurrence + first_tag_size;
		end_pos = last_occurrence + first_tag_size + 1;
		return str.substr(first_occurrence + first_tag_size, last_occurrence - start_idx);
	}

	return "";
}