// Note that the only valid version of the GPL as far as jwSMTP
// is concerned is v2 of the license (ie v2, not v2.2 or v3.x or whatever),
// unless explicitly otherwise stated.
//
// This file is part of the jwSMTP library.
//
//  jwSMTP library is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; version 2 of the License.
//
//  jwSMTP library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with jwSMTP library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// jwSMTP library
//   http://johnwiggins.net
//   smtplib@johnwiggins.net
//
#ifndef __MAILER_H__
#define __MAILER_H__

#include <string>
#include <string.h> //Edit: Included for  strcpy()
#include <vector>
#include "compat.h"

namespace jwsmtp {

class mailer
{
public:
   // if MXLookup is true:
   //    'server' is a nameserver to lookup an MX record by.
   // if MXLookup is false.
   //    'server' is an SMTP server which will be attempted directly for mailing
   // if an IP address is not found, either MX record or direct to SMTP server,
   // an attempt will be made to send mail directly to the server in the mail address.
   // e.g. mail to fred@somewhere.com will have a connection attempt made directly to:
   //      somewhere.com  (which is probably wrong and therefore will still fail)
   mailer(const char* TOaddress, const char* FROMaddress,
         const char* Subject, const std::vector<char>& Message,
         const char* server = "127.0.0.1"/*default to localhost*/,
         unsigned short Port = SMTP_PORT, // default SMTP port
         bool MXLookup = true);

   mailer(const char* TOaddress, const char* FROMaddress,
         const char* Subject, const char* Message,
         const char* server = "127.0.0.1"/*default to localhost*/,
         unsigned short Port = SMTP_PORT, // default SMTP port
         bool MXLookup = true);

   // defaults to SMTP_PORT & no MX lookup.
   //  now we can do:
   //         mailer m;                          // mail an smtp server direct.
   //         mailer m2(true);                   // use MX lookup.
   //         mailer m2(false, weirdportnumber); // SMTP to a non standard port.
   mailer(bool MXLookup = false, unsigned short Port = SMTP_PORT);

   ~mailer();
   
   // call this operator to have the mail mailed.
   // This is to facilitate using multiple threads
   // i.e. using boost::thread.     (see http://www.boost.org)
   //
   // e.g.
   //    mailer mail(args...);
   //    boost::thread thrd(mail); // operator()() implicitly called.
   //    thrd.join(); // if needed.
   //
   // or:
   //    mailer mail(args...);
   //    mail.operator()();
   void operator()();
   void send();

   // attach a file to the mail. (MIME 1.0)
   // returns false if !filename.length() or
   // the file could not be opened for reading...etc.
   bool attach(const std::string& filename);

   // remove an attachment from the list of attachments.
   // returns false if !filename.length() or
   // the file is not attached or there are no attachments.
   bool removeattachment(const std::string& filename);

   // Set a new message (replacing the old)
   // will return false and not change the message if newmessage is empty.
   bool setmessage(const std::string& newmessage);
   bool setmessage(const std::vector<char>& newmessage);
   
   // Set a new HTML message (replacing the old)
   // will return false and not change the message if newmessage is empty.
   bool setmessageHTML(const std::string& newmessage);
   bool setmessageHTML(const std::vector<char>& newmessage);
   // use a file for the data
   bool setmessageHTMLfile(const std::string& filename);

   // Set a new Subject for the mail (replacing the old)
   // will return false if newSubject is empty.
   bool setsubject(const std::string& newSubject);

   // sets the nameserver or smtp server to connect to
   // dependant on the constructor call, i.e. whether
   // 'lookupMXRecord' was set to false or true.
   // (see constructor comment for details)
   bool setserver(const std::string& nameserver_or_smtpserver);

   // sets the senders address (fromAddress variable)
   bool setsender(const std::string& newsender);

   // add a recipient to the recipient list. (maximum allowed recipients 100).
   // returns true if the address could be added to the
   // recipient list, otherwise false.
   // recipient_type must be in the range mailer::TO -> mailer::BCC if
   // not recipient_type defaults to BCC (blind copy), see const enum below.
   bool addrecipient(const std::string& newrecipient, short recipient_type = TO /*CC, BCC*/);

   // remove a recipient from the recipient list.
   // returns true if the address could be removed from the
   // recipient list, otherwise false.
   bool removerecipient(const std::string& recipient);

   // clear all recipients from the recipient list.
   void clearrecipients();

   // clear all attachments from the mail.
   void clearattachments();

   // clear all recipients, message, attachments, errors.
   // does not reset the name/smtp server (use setserver for this)
   // does not set the senders address (use setsender for this)
   void reset();

   // returns the return code sent by the smtp server or a local error.
   // this is the only way to find if there is an error in processing.
   // if the mail is sent correctly this string will begin with 250
   // see smtp RFC 821 section 4.2.2 for response codes.
   const std::string& response() const;

   // Constants
   // in unix we have to have a named object.
   const static enum {TO, Cc, Bcc, SMTP_PORT = 25, DNS_PORT = 53} consts;   

   // what type of authentication are we using.
   // (if using authentication that is).
   enum authtype {LOGIN = 1, PLAIN} type;

   // set the authentication type
   // currently LOGIN or PLAIN only.
   // The default login type is LOGIN, set in the constructor
   void authtype(const enum authtype Type);
   
   // set the username for authentication.
   // If this function is called with a non empty string
   // jwSMTP will try to use authentication.
   // To not use authentication after this, call again
   // with the empty string e.g.
   //    mailerobject.username("");
   void username(const std::string& User);
   // set the password for authentication
   void password(const std::string& Pass);
   
private:
   // create a header with current message and attachments.
   std::vector<char> makesmtpmessage() const;

   // this breaks a message line up to be less than 1000 chars per line.
   // keeps words intact also --- rfc821
   // Check line returns are in the form "\r\n"
   // (qmail balks otherwise, i.e. LAME server)
   // also if a period is on a line by itself add a period
   //   stops prematurely ending the mail before whole message is sent.
   void checkRFCcompat();

   // helper function.
   // returns the part of the string toaddress after the @ symbol.
   // i.e. the 'toaddress' is an email address eg. someone@somewhere.com
   // this function returns 'somewhere.com'
   std::string getserveraddress(const std::string& toaddress) const;

   // Does the work of getting MX records for the server returned by 'getserveraddress'
   // will use the dns server passed to this's constructor in 'nameserver'
   // or if MXlookup is false in the constuctor, will return an address
   // for the server that 'getserveraddress' returns.
   // returns false on failure, true on success
   bool gethostaddresses(std::vector<SOCKADDR_IN>& adds);

   // Parses a dns Resource Record (see TCP/IP illustrated, STEVENS, page 194)
   bool parseRR(int& pos, const unsigned char dns[], std::string& name, in_addr& address);

   // Parses a dns name returned in a dns query (see TCP/IP illustrated, STEVENS, page 192)
   void parsename(int& pos, const unsigned char dns[], std::string& name);

   // email address wrapper struct
   struct Address {
	
      Address() 
	  	: name()
		,address()
	  {
	  }

      std::string name;    // e.g.   freddy foobar
      std::string address; // e.g.   someone@mail.com
   };

   // authenticate against a server.
   bool authenticate(const std::string& servergreeting, const SOCKET& s);

   // less typing later, these are definately abominations!
   typedef std::vector<std::pair<std::vector<char>, std::string> >::const_iterator vec_pair_char_str_const_iter;
   typedef std::vector<std::pair<Address, short> >::const_iterator recipient_const_iter;
   typedef std::vector<std::pair<Address, short> >::iterator recipient_iter;
   typedef std::vector<std::string>::const_iterator vec_str_const_iter;

   // split an address into its relevant parts i.e.
   // name and actual address and return it in Address.
   // this may be usefull out of the class maybe
   // it should be a static function or a global? thinking about it.
   Address parseaddress(const std::string& addresstoparse);

   // The addresses to send the mail to
   std::vector<std::pair<Address, short> > recipients;
   // The address the mail is from.
   Address fromAddress;
   // Subject of the mail
   std::string subject;
   // The contents of the mail message
   std::vector<char> message;
   // The contents of the mail message in html format.
   std::vector<char> messageHTML;
   // attachments: the file as a stream of char's and the name of the file.
   std::vector<std::pair<std::vector<char>, std::string> > attachments;
   // This will be filled in from the toAddress by getserveraddress
   std::string server;
   // Name of a nameserver to query
   std::string nameserver;
   // The port to mail to on the smtp server.
   const unsigned short port;
   // use dns to query for MX records
   const bool lookupMXRecord;
   // using authentication
   bool auth;
   // username for authenticated smtp
   std::string user;
   // password for authenticated smtp
   std::string pass;
   // filled in with server return strings
   std::string returnstring;
};

} // end namespace jwsmtp

#endif // !ifndef __MAILER_H__
