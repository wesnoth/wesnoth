#ifndef PUBLISH_CAMPAIGN_HPP_INCLUDED
#define PUBLISH_CAMPAIGN_HPP_INCLUDED

class config;

#include <string>
#include <vector>

void get_campaign_info(const std::string& campaign_name, class config& cfg);
void set_campaign_info(const std::string& campaign_name, const class config& cfg);

std::vector<std::string> available_campaigns();
void archive_campaign(const std::string& campaign_name, class config& cfg);
void unarchive_campaign(const class config& cfg);

bool campaign_name_legal(const std::string& name);

#endif
