#include "addon_client.hpp"
#include "foreach.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <fstream>
#include <sstream>
#include <iomanip>

static lg::log_domain log_ad("addons");
#define DBG_AD LOG_STREAM(debug, log_ad)
#define LOG_AD LOG_STREAM(info, log_ad)
#define ERR_AD LOG_STREAM(err, log_ad) 

namespace network {

addon_client_error::addon_client_error(const std::string& why) :
std::runtime_error(why)
{
}

addon_client::addon_client()
: error_buffer_()
{
	handle_ = curl_easy_init();

	//DEBUG: make verbose
	//TODO: get rid of this in production code
	//curl_easy_setopt(handle_, CURLOPT_VERBOSE, 1);

	//put errors in readable form into error_buffer
	curl_easy_setopt(handle_, CURLOPT_ERRORBUFFER, error_buffer_);

	//set default callback
	curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, addon_client::default_recv_callback);

	//do not include http  header in output
	curl_easy_setopt(handle_, CURLOPT_HEADER, 0);

	curl_easy_setopt(handle_, CURLOPT_FOLLOWLOCATION, true);
}

addon_client::~addon_client()
{
	curl_easy_cleanup(handle_);
}

std::string addon_client::url_encode(const std::string& raw_string) const
{
	//this may be slow but it shouldn't become a bottleneck
	//during any sane use case
	//optimize only if you really need to
	std::ostringstream encoded;
	foreach(unsigned char c, raw_string)
	{
		//note: you could just convert /all/ chars to %hex_val and it would be
		//proper but typically only special chars are escaped in such a way
		if ( (48 <= c && c <= 57) ||//0-9
			(65 <= c && c <= 90) ||//abc...xyz
			(97 <= c && c <= 122)) //ABC...XYZ
		{
			encoded << c;
		}
		else
		{
			encoded << '%' << std::hex << std::setfill ('0') << std::setw (2)<< int(c);
		}
	}
	return encoded.str();
}

std::string addon_client::get_response(const std::string &url)
{
	string_map_t arguments;
	return get_response(url, arguments);
}

std::string addon_client::get_response(std::string url,
		const string_map_t& arguments,
		bool post)
{
	std::ostringstream params;
	for(string_map_t::const_iterator i = arguments.begin(); i != arguments.end(); i++) {
		params << url_encode(i->first) << '=' << url_encode(i->second);
		string_map_t::const_iterator j = i;
		j++;
		if(j != arguments.end())
			params << '&';
		LOG_AD << params.str();
	}
	std::string params_str = params.str();

	if(post && arguments.find("wml") != arguments.end()) {
		//assume wml sending means a hueg amount of data and use a more
		//efficient encoding
		struct curl_httppost* first = NULL;
		struct curl_httppost* last = NULL;
		//Add name/ptrcontent/contenttype section
		for(string_map_t::const_iterator i = arguments.begin(); i != arguments.end(); i++) {
			curl_formadd(&first, &last, CURLFORM_COPYNAME, i->first.c_str(),
					   CURLFORM_PTRCONTENTS, i->second.c_str(),
					   CURLFORM_CONTENTSLENGTH, i->second.length(),
					   CURLFORM_CONTENTTYPE, "text/plain", CURLFORM_END);
		}
		//TODO: there's a small memory leak right there!
		//One should call curl_formfree on post and last after flushing.
		curl_easy_setopt(handle_, CURLOPT_HTTPPOST, first);
	} else {
		//convert arguments to an encoded string like key=value?other+key=1		
		if (post) {
			//set post body
			curl_easy_setopt(handle_, CURLOPT_POSTFIELDS, (void*)(params_str.c_str()));
			curl_easy_setopt(handle_, CURLOPT_POST, 1);
		} else if(!params.str().empty()) {
			//just slap the params to the url
			url += std::string("?") + params.str();
		}
	}

	//setup actual url
	curl_easy_setopt(handle_, CURLOPT_URL, url.c_str());

	//setup data to pass to callback
	std::string buffer;
	curl_easy_setopt(handle_, CURLOPT_WRITEDATA, &buffer);

	//execute
	flush(buffer);
	return buffer;
}

void addon_client::flush(const std::string& buffer)
{
	CURLcode error = curl_easy_perform(handle_);
	if(error != CURLE_OK)
		throw addon_client_error(error_buffer_);
	DBG_AD << buffer;
	std::ofstream ofs("last_get.html");
	ofs << buffer;
	long http_code = 0;
	curl_easy_getinfo(handle_, CURLINFO_RESPONSE_CODE, &http_code);
	LOG_AD << http_code << "\n";
	if (http_code == 404) {
		throw addon_client_error("Not found");
	} else if (http_code == 500) {
		throw addon_client_error("Server error");
	} else if (http_code != 200) {
		throw addon_client_error("Server response not recognized");
	}
}

void addon_client::set_base_url(const std::string& base_url)
{
	//TODO trailing slash bulletproofing
	base_url_ = base_url;
	if(base_url_[base_url_.length()-1] != '/')
		base_url_.push_back('/');
}

std::string addon_client::get_addon_description(unsigned int addon_id)
{
	std::ostringstream address;
	address <<base_url_<< "details/" << addon_id <<"/?wml";
	
	return get_response(address.str());
}

std::string addon_client::get_addon_list()
{
	std::ostringstream address;
	address << base_url_ <<"?wml";

	return get_response(address.str());;
}

size_t addon_client::default_recv_callback
		(void* buffer, size_t size, size_t nmemb, void* userp)
{
	//userp will be a std::string passed by an addon_client method
	//buffer size is size*nmemb
	//I have no idea what nmemb stands for.
	(static_cast<std::string*>(userp))->append((char*)buffer, size*nmemb);
	//if return size differs from total size, 
	//an error is returned by curl_easy_perform
	return size*nmemb;	
}

config addon_client::get_addon_list_cfg()
{
	std::string list = get_addon_list();
	config cfg;
	read(cfg, list);
	return cfg;
}

config addon_client::get_addon_cfg(unsigned int addon_id)
{
	std::ostringstream id;
	id << addon_id;
	return get_addon_cfg(id.str());
}

config addon_client::get_addon_cfg(const std::string& addon_name)
{
	config cfg;
	std::string wml = get_addon(addon_name);
	read(cfg, wml);
	return cfg;
}

std::string addon_client::get_addon(const std::string& addon_name)
{
	std::ostringstream address;
	address <<base_url_<< "download/" << url_encode(addon_name) <<"/?wml";
	return get_response(address.str());
}

//bool addon_client::is_addon_valid(const config& pbl, std::string login, std::string pass, std::string& error_message);
std::string addon_client::publish_addon(const config& addon, const std::string& login, const std::string& pass)
{
	std::ostringstream parsed_config;
	write(parsed_config, addon);
	string_map_t args;
	args["login"] = login;
	args["password"] = pass;
	args["wml"] = parsed_config.str();

	std::ostringstream address;
	address <<base_url_<< "publish/?wml";
	return get_response(address.str(), args, true);
}

std::string addon_client::delete_remote_addon(const std::string& addon_name, const std::string& login, const std::string& pass)
{
	string_map_t args;
	args["login"] = login;
	args["password"] = pass;

	std::ostringstream address;
	address <<base_url_<< "remove/" << url_encode(addon_name) <<"/?wml";
	return get_response(address.str(), args, true);

	
}
/*
-----------------------
Async methods and stuff
-----------------------
*/

int addon_client::upload_progress_callback(
		void *clientp,
		double /*dltotal*/,
		double /*dlnow*/,
		double ultotal,
		double ulnow)
{
	progress_data* pd = (progress_data*)(clientp);
	if(pd->abort())
		return 666; //client aborted
	pd->set_done(ulnow);
	pd->set_total(ultotal);	
	return 0;
}

int addon_client::download_progress_callback(
		void *clientp,
		double dltotal,
		double dlnow,
		double /*ultotal*/,
		double /*ulnow*/)
{
	progress_data* pd = (progress_data*)(clientp);
	if(pd->abort())
		return 666; //client aborted
	pd->set_done(dlnow);
	pd->set_total(dltotal);	
	return 0;
}

void addon_client::async_entry(
		progress_data* pd,
		bool download,
		boost::function<std::string ()> blocking_fun)
{
	pd->set_running(true);
	async_response_buffer_.clear();
	curl_easy_setopt(handle_, CURLOPT_NOPROGRESS, 0);
	if(download)
		curl_easy_setopt(handle_, CURLOPT_PROGRESSFUNCTION, addon_client::download_progress_callback);
	else
		curl_easy_setopt(handle_, CURLOPT_PROGRESSFUNCTION, addon_client::upload_progress_callback);
	curl_easy_setopt(handle_, CURLOPT_PROGRESSDATA, (void*)(pd));
	try 
	{
		async_response_buffer_ = blocking_fun();
	}
	catch(addon_client_error e)
	{
		std::ostringstream error;
		error << "[error]" << std::endl << "message=" << e.what() << std::endl <<"[/error]";
		async_response_buffer_ = error.str();
	}
	pd->set_running(false);
}

void addon_client::async_wrap(progress_data& pd, bool down, boost::function<std::string()> f)
{
        async_wait(); //Previous calls must finish
	thread_ = boost::thread(&addon_client::async_entry, this, &pd, down, f);
}

void addon_client::async_get_addon_list(progress_data& pd)
{
	boost::function<std::string()> f = boost::bind(&addon_client::get_addon_list, this);	
	async_wrap(pd, true, f);		
}

void addon_client::async_get_addon(progress_data& pd, const std::string& name)
{
	boost::function<std::string()> f = boost::bind(&addon_client::get_addon, this, name);
	async_wrap(pd, true, f);
}

void addon_client::async_publish_addon(
	progress_data& pd, 
	const config& addon, 
	const std::string& login,
	const std::string& pass)
{
	async_wrap(pd, true, boost::bind(
		&addon_client::publish_addon,
		this, addon, login, pass)		
		);	
}

void addon_client::async_delete_remote_addon(
	progress_data& pd, 
	const std::string& addon_name,
	const std::string& login,
	const std::string& pass)
{
	async_wrap(pd, true, boost::bind(
		&addon_client::delete_remote_addon, 
		this, addon_name, login, pass)
		);
}

void addon_client::async_wait()
{
	thread_.join();
}

std::string addon_client::get_async_response()
{
	async_wait();
	return async_response_buffer_;
}

} //end namespace network
