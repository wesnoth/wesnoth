#include <string>
#include <iostream>
#include "../md5.hpp"
#include <string.h>
#include <mysql++/mysql++.h>

namespace forum_auth {

// This class holds the result of authorisation against forum table for user_name
// and password given. That result can be allways accesed by is_authorised() method.
// Class throws an exception error (std::string) on :
// * Failure to obtain database connection data
// * Failure to connect to database
// * Failure to find entry coresponding to username

class forum_auth
{
	private :
// The class will only hold data provided by the user and the result of their validation
// It's not required or desirable to remember data pulled out from the forum tables.
		std::string user_name;
		std::string user_password;
		bool authorised;
		bool validate();
	public :
// Constructor - sets the data and evaluates them
		forum_auth(std::string name, std::string password);
// Destructor is empty
		~forum_auth();
// Function to return authorisation status
		bool is_authorised();
};
// Destructor
forum_auth::~forum_auth()
{
}
// Constructor
forum_auth::forum_auth(std::string name, std::string password)
{
	user_name = name;
	user_password = password;
	authorised = validate();
}
// Validation function
bool forum_auth::validate()
{
// Set an alphabet-like string for use in encrytpion algorithm
	std::string itoa64("./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	std::string hash, db_name, db_host, db_user, db_password;
// Get the database acces details
	std::fstream db_access_file("db_auth_data.cfg", std::fstream::out);
	if(!db_access_file.is_open())
	{
		std::string error("Forum auth : No file with access data\n");
		throw error;
	}
	db_access_file >> db_name;
	db_access_file >> db_host;
	db_access_file >> db_user;
	db_access_file >> db_password;
// Connect to the database
	mysqlpp::Connection db_interface(false);
	if (!db_interface.connect(db_name.c_str(), db_host.c_str(), db_user.c_str(), db_password.c_str()))
	{
		std::string error("Forum auth : Connection to the database failed\n");
		throw error;
	}
// Retrive users' password as hash
	std::string sql("SELECT hash FROM phpbb3_wesnothd WHERE username='");
	sql.append(user_name);
	sql.append("'");
	mysqlpp::Query query = db_interface.query(sql);
	mysqlpp::StoreQueryResult sql_res = query.store();
	if(sql_res.num_rows() == 0)
	{
		std::string error("Forum auth : User not found");
		throw error;
	}
	hash = std::string(sql_res[0][0]);
// Check hash prefix, if different than $H$ hash is invalid
	if(hash.substr(0,3) != "$H$")
		return false;
// Start of the encryption, get the position of first nonidentifier character in extended alphabet
	int hash_seed = itoa64.find_first_of(hash[3]);
// If position is lower than 8 or higher than 32 hash is also invalid
	if(hash_seed < 7 || hash_seed > 30)
		return false;
// Set the number of encryption passes as 2^position
	hash_seed = 1 << hash_seed;
// Create salt for mixing with the hash
	std::string salt=hash.substr(4,8);
// Start the MD5 hashing
	salt.append(user_password);
	MD5 md5_worker;
	md5_worker.update((unsigned char *)salt.c_str(),salt.length());
	md5_worker.finalize();
	unsigned char * output = (unsigned char *) malloc (sizeof(unsigned char) * 16);
	output = md5_worker.raw_digest();
	std::string temp_hash;
	do
	{
		temp_hash = std::string((char *) output, (char *) output + 16);
		temp_hash.append(user_password);
		md5_worker.~MD5();
		MD5 md5_worker;
		md5_worker.update((unsigned char *)temp_hash.c_str(),temp_hash.length());
		md5_worker.finalize();
		output =  md5_worker.raw_digest();
	} while (--hash_seed);
	temp_hash = std::string((char *) output, (char *) output + 16);
// Now encode the resulting mix
	std::string encoded_hash;
	unsigned int i = 0, value;
	do
	{
		value = output[i++];
		encoded_hash.append(itoa64.substr(value & 0x3f,1));
		if(i < 16)
			value |= (int)output[i] << 8;
		encoded_hash.append(itoa64.substr((value >> 6) & 0x3f,1));
		if(i++ >= 16)
			break;
		if(i < 16)
			value |= (int)output[i] << 16;
		encoded_hash.append(itoa64.substr((value >> 12) & 0x3f,1));
		if(i++ >= 16)
			break;
		encoded_hash.append(itoa64.substr((value >> 18) & 0x3f,1));
	} while (i < 16);
	free (output);
// Get the first 12 characters from correct hash
	std::string result = hash.substr(0,12);
// Append encoded results to the end of it
	result.append(encoded_hash);
// Check if reult of above matches the original hash
	if( result != hash)
		return false;
	return true;
}
// Function returning status
bool forum_auth::is_authorised()
{
	return authorised;
}

} // End of forum_auth namespace

