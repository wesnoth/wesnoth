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
#ifdef WIN32
// std::vector<std::string> This gives this warning in VC..
// bloody annoying, there is a way round it according to MS.
// The debugger basically cannot browse anything with a name
// longer than 256 characters, "get with the template program MS".
#pragma warning( disable : 4786 )
#endif
#include <fstream>
#include <sstream>   // ostrstream
#include <ctime>     // for localtime
#include <cassert>
#include "mailer.h"
#include "base64.h"

namespace jwsmtp {

mailer::mailer(const char* TOaddress, const char* FROMaddress,
               const char* Subject, const std::vector<char>& Message,
               const char* Nameserver, unsigned short Port,
               bool MXLookup): type(LOGIN),
                               subject(Subject),
                               server(getserveraddress(TOaddress)),
                               nameserver(Nameserver),
                               port(htons(Port)), // make the 'port' network byte order.
                               lookupMXRecord(MXLookup),
                               auth(false) {
   // Parse the email addresses into an Address structure.
   setsender(FROMaddress);
   addrecipient(TOaddress);
   setmessage(Message);

   initNetworking(); // in win32 init networking, else just does nothin'
}

mailer::mailer(const char* TOaddress, const char* FROMaddress,
               const char* Subject, const char* Message,
               const char* Nameserver, unsigned short Port,
               bool MXLookup): type(LOGIN),
                               subject(Subject),
                               server(getserveraddress(TOaddress)),
                               nameserver(Nameserver),
                               port(htons(Port)), // make the 'port' network byte order.
                               lookupMXRecord(MXLookup),
                               auth(false) {
   // Parse the email addresses into an Address structure.
   setsender(FROMaddress);
   addrecipient(TOaddress);
   setmessage(Message);

   initNetworking(); // in win32 init networking, else just does nothin'
}

mailer::mailer(bool MXLookup, unsigned short Port):
      type(LOGIN),
      port(htons(Port)),
      lookupMXRecord(MXLookup),
      auth(false) {
   initNetworking(); // in win32 init networking, else just does nothin'
}

mailer::~mailer() { }
   
bool mailer::setmessage(const std::string& newmessage) {
   if(!newmessage.length())
      return false;

   message.clear(); // erase the old message
   for (std::string::size_type i = 0; i < newmessage.length(); ++i)
      message.push_back(newmessage[i]);

   checkRFCcompat();

   return true;
}

bool mailer::setmessage(const std::vector<char>& newmessage) {
   if(!newmessage.size())
      return false;

   message = newmessage;

   checkRFCcompat();

   return true;
}

bool mailer::setmessageHTML(const std::string& newmessage) {
   if(!newmessage.length())
      return false;

   messageHTML.clear(); // erase the old message
   for (std::string::size_type i = 0; i < newmessage.length(); ++i)
      messageHTML.push_back(newmessage[i]);
   messageHTML = base64encode(messageHTML);

   return true;
}

bool mailer::setmessageHTML(const std::vector<char>& newmessage) {
   if(!newmessage.size())
      return false;

   messageHTML = base64encode(newmessage);

   return true;
}

bool mailer::setmessageHTMLfile(const std::string& filename) {
   if(!filename.length())
      return false;
   
   std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
   if(!file)
      return false;
   std::vector<char> filedata;
   char c = file.get();
   for(; file.good(); c = file.get()) {
      if(file.bad())
         break;
      filedata.push_back(c);
   }

   messageHTML = base64encode(filedata);

   return true;
}

// this breaks a message line up to be less than 1000 chars per line.
// keeps words intact also.
// Check line returns are in the form "\r\n"
// (qmail balks otherwise, i.e. LAME server)
void mailer::checkRFCcompat() {
   // Check the line breaks.
   std::vector<char>::iterator it;
   for(it = message.begin(); it != message.end(); ++it) {
      // look for \n add \r before if not there. Pretty lame but still.
      // haven't thought of a better way yet.
      if(*it == '\n') {
         if(it == message.begin()) {
            it = message.insert(it, '\r');
            ++it; // step past newline
            continue;
         }
         if((*(it -1) != '\r') ) {
            // add a return before '\n'
            it = message.insert(it, '\r');
            ++it; // step past newline
         }
      }
   }

   // if we get a period on a line by itself
   // add another period to stop the server ending the mail prematurely.
   // ( suggested by david Irwin )
   if(message.size() == 1) {
      if(*(message.begin()) == '.')
         message.push_back('.');
   }
   else if(message.size() == 2) {
      if(*(message.begin()) == '.') {
         it = message.begin();
         it = message.insert(it, '.');
      }
   }
   else {
      if(*(message.begin()) == '.') {         
         it = message.begin();
         it = message.insert(it, '.');
      }
      for(it = message.begin()+2; it != message.end(); ++it) {
         // follow the rfc. Add '.' if the first character on a line is '.'
         if(*it == '\n') {
            if( ((it + 1) != message.end()) && (*(it +1) == '.') ) {
               it = message.insert(it + 1, '.');
               ++it; // step past
            }
         }
      }
   }

   // don't do anything if we are not longer than a 1000 characters
   if(message.size() < 1000)
      return;

   // now we have checked line breaks
   // check line lengths.
   int count(1);
   for(it = message.begin(); it < message.end(); ++it, ++count) {
      if(*it == '\r') {
         count = 0; // reset for a new line.
         ++it; // get past newline
         continue;
      }
      else if(count >= 998) {
         ++it;
         if(*it != ' ') { // we are not in a word!!
            // it should never get to message.begin() because we
            // start at least 998 chars into the message!
            // Also, assume a word isn't bigger than 997 chars! (seems reasonable)
            std::vector<char>::iterator pos = it;
            for(int j = 0; j < 997; ++j, --pos) {
               if(*pos == ' ') {
                  it = ++pos; // get past the space.
                  break;
               }
            }
         }
         if(it < message.end())
            it = message.insert(it, '\r');
         ++it;
         if(it < message.end())
            it = message.insert(it, '\n');
         count = 0; // reset for a new line.
      }
   }
   count=1; // reset the count
   if(messageHTML.size()) {
      for(it = messageHTML.begin(); it < messageHTML.end(); ++it, ++count) {
         if(*it == '\r') {
            count = 0; // reset for a new line.
            ++it; // get past newline
            continue;
         }
         else if(count >= 998) {
            ++it;
            if(*it != ' ') { // we are in a word!!
               // it should never get to message.begin() because we
               // start at least 998 chars into the message!
               // Also, assume a word isn't bigger than 997 chars! (seems reasonable)
               std::vector<char>::iterator pos = it;
               for(int j = 0; j < 997; ++j, --pos) {
                  if(*pos == ' ') {
                     it = ++pos; // get past the space.
                     break;
                  }
               }
            }
            if(it < messageHTML.end())
               it = messageHTML.insert(it, '\r');
            ++it;
            if(it < messageHTML.end())
               it = messageHTML.insert(it, '\n');
            count = 0; // reset for a new line.
         }
      }
   }
}

bool mailer::setsubject(const std::string& newSubject) {
   if(!newSubject.length())
      return false;

   subject = newSubject;
   return true;
}

bool mailer::setserver(const std::string& nameserver_or_smtpserver) {
   if(!nameserver_or_smtpserver.length())
      return false;

   nameserver = nameserver_or_smtpserver;
   return true;
}

bool mailer::setsender(const std::string& newsender) {
   if(!newsender.length())
      return false;

   Address newaddress(parseaddress(newsender));

   fromAddress = newaddress;
   return true;
}

bool mailer::addrecipient(const std::string& newrecipient, short recipient_type) {
   // SMTP only allows 100 recipients max at a time.
   // rfc821
   if(recipients.size() >= 100) // == would be fine, but let's be stupid safe
      return false;

   if(newrecipient.length()) {
      // If there are no recipients yet
      // set the server address for MX queries
      if(!recipients.size()) {
         server = getserveraddress(newrecipient);
      }

      Address newaddress = parseaddress(newrecipient);

      if(recipient_type > Bcc || recipient_type < TO)
         recipient_type = Bcc; // default to blind copy on error(hidden is better)

      recipients.push_back(std::make_pair(newaddress, recipient_type));
      return true;
   }
   return false;
}

bool mailer::removerecipient(const std::string& recipient) {
   if(recipient.length()) { // there is something to remove
      std::vector<std::pair<Address, short> >::iterator it(recipients.begin());
      for(; it < recipients.end(); ++it) {
         if((*it).first.address == recipient) {
            recipients.erase(it);
            return true;
         }
      }
      // fall through as we did not find this recipient
   }
   return false;
}

void mailer::clearrecipients() {
   recipients.clear();
}

void mailer::clearattachments() {
   attachments.clear();
}

void mailer::reset() {
   recipients.clear();
   attachments.clear();
   // fromAddress = ""; // assume the same sender.
   // if this is to be changed use the setserver function to change it.
   // nameserver = ""; // we don't do this as the same server is probably used!!
   // leave auth type alone.
   // leave username password pair alone.
   server = "";
   message.clear();
   messageHTML.clear();
   returnstring = ""; // clear out any errors from previous use
}

// convenience function
void mailer::send() {
   operator()();
}

// this is where we do all the work.
void mailer::operator()() {
   returnstring = ""; // clear out any errors from previous use

   if(!recipients.size()) {
      returnstring = "451 Requested action aborted: local error who am I mailing";
      return;
   }
   if(!fromAddress.address.length()) {
      returnstring = "451 Requested action aborted: local error who am I";
      return;
   }
   if(!nameserver.length()) {
      returnstring = "451 Requested action aborted: local error no SMTP/name server/smtp server";
      return;
   }

   std::vector<SOCKADDR_IN> adds;
   if(lookupMXRecord) {
      if(!gethostaddresses(adds)) {
         // error!! we are dead.
         returnstring = "451 Requested action aborted: No MX records ascertained";
         return;
      }
   }
   else { // connect directly to an SMTP server.
      SOCKADDR_IN addr(nameserver, port, AF_INET);
      hostent* host = 0;
      if(addr) {
         host = gethostbyaddr(addr.get_sin_addr(), sizeof(addr.ADDR.sin_addr), AF_INET);
      }
      else
         host = gethostbyname(nameserver.c_str());
      if(!host) {
         returnstring = "451 Requested action aborted: local error in processing";
         return; // error!!!
      }
      //memcpy(addr.get_sin_addr(), host->h_addr, host->h_length);
      std::copy(host->h_addr_list[0], host->h_addr_list[0] + host->h_length, addr.get_sin_addr());
      adds.push_back(addr);
   }

   SOCKET s;
   if(!Socket(s, AF_INET, SOCK_STREAM, 0)) {
      returnstring =  "451 Requested action aborted: socket function error";
      return;
   }

   if(!adds.size()) { // oops
      returnstring = "451 Requested action aborted: No MX records ascertained";
   }

   const std::string OK("250");
   const std::vector<char> smtpheader(makesmtpmessage());
   const int buffsize(1024);
   char buff[buffsize] = "";

   for(std::vector<SOCKADDR_IN>::const_iterator address = adds.begin();
            address < adds.end(); ++address) {
      if(!Connect(s, *address)) {
         returnstring = "554 Transaction failed: server connect error.";
         continue;
      }

      // 220 the server line returned here
      int len1;
      if(!Recv(len1, s, buff, buffsize -1, 0)) {
         returnstring = "554 Transaction failed: server connect response error.";
         continue;
      }

      // get our hostname to pass to the smtp server
      char hn[buffsize] = "";
      if(gethostname(hn, buffsize)) {
         // no local hostname!!! make one up
         strcpy(hn, "flibbletoot");
      }
      std::string commandline(std::string("EHLO ") + hn + std::string("\r\n"));
      // say hello to the server

      if(!Send(len1, s, commandline.c_str(), commandline.length(), 0)) {
         returnstring = "554 Transaction failed: EHLO send";
         continue;
      }
      if(!Recv(len1, s, buff, buffsize -1, 0)) {
         returnstring = "554 Transaction failed: EHLO receipt";
         continue;
      }
      
      buff[len1] = '\0';
      std::string greeting = returnstring = buff;
      if(returnstring.substr(0,3) != OK) {
         if(auth) {
            // oops no ESMTP but using authentication no go bail out!
            returnstring = "554 possibly trying to use AUTH without ESMTP server, ERROR!";
            continue;
         }
         // maybe we only do non extended smtp
         // send HELO instead.
         commandline[0] = 'H';
         commandline[1] = 'E';
         if(!Send(len1, s, commandline.c_str(), commandline.length(), 0)) {
            returnstring = "554 Transaction failed: HELO send";
            continue;
         }
         if(!Recv(len1, s, buff, buffsize -1, 0)) {
            returnstring = "554 Transaction failed: HELO receipt";
            continue;
         }
         buff[len1] = '\0';

         returnstring = buff;
         if(returnstring.substr(0,3) != OK) {
            // we must issue a quit even on an error.
            // in keeping with the rfc's
            if(Send(len1, s, "QUIT\r\n", 6, 0)) {
               char dummy[buffsize];
               Recv(len1, s, dummy, buffsize -1, 0);
            }
            Closesocket(s);
            // don't know what went wrong here if we are connected!!
            // we continue because maybe we have more than 1 server to connect to.
            continue;
         }
      }

      if(auth)
         if(!authenticate(greeting, s))
            continue; // try the next server, you never know!!

      // MAIL
      // S: MAIL FROM:<Smith@Alpha.ARPA>
      // R: 250 OK
      // e.g. "MAIL FROM:<someone@somewhere.com>\r\n"
      // or   "MAIL FROM: John Wiggins <someone@somewhere.com>"
      commandline = "MAIL FROM:<" + fromAddress.address + ">\r\n";
      if(!Send(len1, s, commandline.c_str(), commandline.length(), 0)) {
         returnstring = "554 MAIL FROM sending error";
         continue;
      }

      if(!Recv(len1, s, buff, buffsize -1, 0)) {
         returnstring = "554 MAIL FROM receipt error";
         continue;
      }

      buff[len1] = '\0';
      returnstring = buff;
      if(returnstring.substr(0,3) != OK) {
         // we must issue a quit even on an error.
         // in keeping with the rfc's
         if(Send(len1, s, "QUIT\r\n", 6, 0)) {
            char dummy[buffsize];
            Recv(len1, s, dummy, buffsize -1, 0);
         }
         Closesocket(s);
         // don't know what went wrong here if we are connected!!
         // we continue because maybe we have more than 1 server to connect to.
         continue;
      }

      for(recipient_const_iter recip = recipients.begin(); recip < recipients.end(); ++recip) {
         // RCPT

         // S: RCPT TO:<Jones@Beta.ARPA>
         // R: 250 OK
         commandline = "RCPT TO: <" + (*recip).first.address + ">\r\n";
         // S: RCPT TO:<Green@Beta.ARPA>
         // R: 550 No such user here
         //
         // S: RCPT TO:<Brown@Beta.ARPA>
         // R: 250 OK
         if(!Send(len1, s, commandline.c_str(), commandline.length(), 0)) {
            returnstring = "554 Transaction failed";
            continue;
         }
         if(!Recv(len1, s, buff, buffsize -1, 0)) {
            returnstring = buff;
            continue;
         }
         buff[len1] = '\0';
         returnstring = buff;
         if(returnstring.substr(0,3) != OK) {
            // This particular recipient does not exist!
            // not strictly an error as we may have more than one recipient
            // we should have an error vector e.g.
            // vector<pair<string address, string error> > errs;
            // errs.push_back(make_pair(recip->first, returnstring));
            //
            // we then need a function to return this vector.
            // e.g. const vector<pair<string address, string error> >& getrecipienterrors();
            continue;
         }
      }

      // DATA

      // S: DATA
      // R: 354 Start mail input; end with <CRLF>.<CRLF>
      // S: Blah blah blah...
      // S: ...etc. etc. etc.

      // S: <CRLF>.<CRLF>
      // R: 250 OK
      if(!Send(len1, s, "DATA\r\n", 6, 0)) {
         returnstring = "554 DATA send error";
         continue;
      }
      if(!Recv(len1, s, buff, buffsize -1, 0)) {
         returnstring = "554 DATA, server response error";
         continue;
      }
      buff[len1] = '\0';
      returnstring = buff;
      if(returnstring.substr(0,3) != "354") {
         // we must issue a quit even on an error.
         // in keeping with the rfc's

         if(Send(len1, s, "QUIT\r\n", 6, 0)) {
            char dummy[buffsize];
            Recv(len1, s, dummy, buffsize -1, 0);
         }
         Closesocket(s);
         continue;
      }
      // Sending the email
      /*if(!Send(len1, s, smtpheader.c_str(), smtpheader.length(), 0)) {*/
      if(!Send(len1, s, &smtpheader[0], smtpheader.size(), 0)) {
         returnstring = "554 DATA, server response error (actual send)";
         continue;   
      }
      if(!Recv(len1, s, buff, buffsize -1, 0)) {
         returnstring = "554 DATA, server response error (actual send)";
         continue;   
      }

      // The server should give us a 250 reply if the mail was delivered okay
      buff[len1] = '\0';
      returnstring = buff;
      if(returnstring.substr(0,3) != OK) {
         // we must issue a quit even on an error.
         // in keeping with the rfc's
         if(Send(len1, s, "QUIT\r\n", 6, 0)) {
            char dummy[buffsize];
            Recv(len1, s, dummy, buffsize -1, 0);
         }
         Closesocket(s);
         continue;
      }
      // hang up the connection
      if(Send(len1, s, "QUIT\r\n", 6, 0)) {
         char dummy[buffsize];
         Recv(len1, s, dummy, buffsize -1, 0);
      }

      // Let the server give us our 250 reply.
      //buff[len1] = '\0';
      //returnstring = buff;

      // for future reference the server is meant to give a 221 response to a quit.
      if(returnstring.substr(0,3) != "221") {
         // maybe will use this later   
      }
      Closesocket(s); // disconnect

      // Let the server give us our 250 reply.
      // don't continue as we have delivered the mail
      // at this point just leave. all done
      //returnstring = "250 Requested mail action okay, completed";
      break;
   }
}

std::vector<char> mailer::makesmtpmessage() const {
   std::string sender(fromAddress.address);
   if(sender.length()) {
      std::string::size_type pos(sender.find("@"));
      if(pos != std::string::npos) { //found the server beginning
         sender = sender.substr(0, pos);
      }
   }

   std::vector<char> ret;
   std::string headerline;
   if(fromAddress.name.length()) {
      headerline = "From: " + fromAddress.address + " (" + fromAddress.name + ") \r\n"
                   "Reply-To: " + fromAddress.address + "\r\n";
      ret.insert(ret.end(), headerline.begin(), headerline.end());
   }
   else {
      headerline = "From: " + fromAddress.address + "\r\n"
                   "Reply-To: " + fromAddress.address + "\r\n";
      ret.insert(ret.end(), headerline.begin(), headerline.end());
   }
   headerline.clear(); // clearout our temp variable

   // add the recipients to the header
   std::vector<std::string> to, cc, bcc;
   for(recipient_const_iter recip = recipients.begin(); recip < recipients.end(); ++recip) {
      if(recip->second == TO) {
         to.push_back(recip->first.address);
      }
      else if(recip->second == Cc) {
         cc.push_back(recip->first.address);
      }
      else if(recip->second == Bcc) {
         bcc.push_back(recip->first.address);
      }
   }
   vec_str_const_iter it; // instead of making three on the stack, just one (stops VC whining too)
   // next section adds To: Cc: Bcc: lines to the header
   int count = to.size();
   if(count)
      headerline += "To: ";
   for(it = to.begin(); it < to.end(); ++it) {
      headerline += *it;
      if(count > 1 && ((it + 1) < to.end()) )
         headerline += ",\r\n "; // must add a space after the comma
      else
         headerline += "\r\n";
   }
   count = cc.size();
   if(count)
      headerline += "Cc: ";
   for(it = cc.begin(); it < cc.end(); ++it) {
      headerline += *it;
      if(count > 1 && ((it + 1) < cc.end()) )
         headerline += ",\r\n "; // must add a space after the comma
      else
         headerline += "\r\n";
   }

   // Remove insertion of the Bcc line into the header, I should never have done this, oopsy
   //
   //count = bcc.size();
   //if(count)
   //   headerline += "Bcc: ";
   //for(it = bcc.begin(); it < bcc.end(); ++it) {
   //   headerline += *it;
   //   if(count > 1 && ((it + 1) < bcc.end()) )
   //      headerline += ",\r\n "; // must add a space after the comma
   //   else
   //      headerline += "\r\n";
   //}
   ret.insert(ret.end(), headerline.begin(), headerline.end());
   // end adding To: Cc: Bcc: lines to the header

   const std::string boundary("bounds=_NextP_0056wi_0_8_ty789432_tp");
   bool MIME(false);
   if(attachments.size() || messageHTML.size())
      MIME = true;

   if(MIME) { // we have attachments
      // use MIME 1.0
      headerline = "MIME-Version: 1.0\r\n"
                 "Content-Type: multipart/mixed;\r\n"
                 "\tboundary=\"" + boundary + "\"\r\n";
      ret.insert(ret.end(), headerline.begin(), headerline.end());
      headerline.clear();
   }

   ///////////////////////////////////////////////////////////////////////////
   // add the current time.
   // format is
   //     Date: 05 Jan 93 21:22:07
   //     Date: 05 Jan 93 21:22:07 -0500
   //     Date: 27 Oct 81 15:01:01 PST        (RFC 821 example)
   time_t t;
   time(&t);
   char timestring[128] = "";
   char * timeformat = "Date: %d %b %y %H:%M:%S %Z";
   if(strftime(timestring, 127, timeformat, localtime(&t))) { // got the date
      headerline = timestring;
      headerline += "\r\n";
      ret.insert(ret.end(), headerline.begin(), headerline.end());
   }
   ///////////////////////////////////////////////////////////////////////////
   // add the subject
   headerline = "Subject: " + subject + "\r\n\r\n";
   ret.insert(ret.end(), headerline.begin(), headerline.end());
   
   ///////////////////////////////////////////////////////////////////////////
   //
   // everything else added is the body of the email message.
   //
   ///////////////////////////////////////////////////////////////////////////
   
   if(MIME) {
      headerline = "This is a MIME encapsulated message\r\n\r\n";
      headerline += "--" + boundary + "\r\n";
      if(!messageHTML.size()) {
         // plain text message first.
         headerline += "Content-type: text/plain; charset=iso-8859-1\r\n"
                       "Content-transfer-encoding: 7BIT\r\n\r\n";

         ret.insert(ret.end(), headerline.begin(), headerline.end());
         ret.insert(ret.end(), message.begin(), message.end());
         headerline = "\r\n\r\n--" + boundary + "\r\n";
      }
      else { // make it multipart/alternative as we have html
         const std::string innerboundary("inner_jfd_0078hj_0_8_part_tp");
         headerline += "Content-Type: multipart/alternative;\r\n"
                       "\tboundary=\"" + innerboundary + "\"\r\n";
         
         // need the inner boundary starter.
         headerline += "\r\n\r\n--" + innerboundary + "\r\n";
         
         // plain text message first.
         headerline += "Content-type: text/plain; charset=iso-8859-1\r\n"
                       "Content-transfer-encoding: 7BIT\r\n\r\n";
         ret.insert(ret.end(), headerline.begin(), headerline.end());
         ret.insert(ret.end(), message.begin(), message.end());
         headerline = "\r\n\r\n--" + innerboundary + "\r\n";
         ///////////////////////////////////
         // Add html message here!
         headerline += "Content-type: text/html; charset=iso-8859-1\r\n"
                       "Content-Transfer-Encoding: base64\r\n\r\n";

         ret.insert(ret.end(), headerline.begin(), headerline.end());
         ret.insert(ret.end(), messageHTML.begin(), messageHTML.end());
         headerline = "\r\n\r\n--" + innerboundary + "--\r\n";
         
         // end the boundaries if there are no attachments
         if(!attachments.size())
            headerline += "\r\n--" + boundary + "--\r\n";
         else
            headerline += "\r\n--" + boundary + "\r\n";
         ///////////////////////////////////
      }
      ret.insert(ret.end(), headerline.begin(), headerline.end());
      headerline.clear();
      
      // now add each attachment.
      for(vec_pair_char_str_const_iter it1 = attachments.begin();
                                       it1 != attachments.end(); ++ it1) {
         if(it1->second.length() > 3) { // long enough for an extension
            std::string typ(it1->second.substr(it1->second.length()-4, 4));
            if(typ == ".gif") { // gif format presumably
               headerline += "Content-Type: image/gif;\r\n";
            }
            else if(typ == ".jpg" || typ == "jpeg") { // j-peg format presumably
               headerline += "Content-Type: image/jpg;\r\n";
            }
            else if(typ == ".txt") { // text format presumably
               headerline += "Content-Type: plain/txt;\r\n";
            }
            else if(typ == ".bmp") { // windows bitmap format presumably
               headerline += "Content-Type: image/bmp;\r\n";
            }
            else if(typ == ".htm" || typ == "html") { // hypertext format presumably
               headerline += "Content-Type: plain/htm;\r\n";
            }
            else if(typ == ".png") { // portable network graphic format presumably
               headerline += "Content-Type: image/png;\r\n";
            }
            else if(typ == ".exe") { // application
               headerline += "Content-Type: application/X-exectype-1;\r\n";
            }
            else { // add other types
               // everything else
               headerline += "Content-Type: application/X-other-1;\r\n";
            }
         }
         else {
            // default to don't know
            headerline += "Content-Type: application/X-other-1;\r\n";
         }

         headerline += "\tname=\"" + it1->second + "\"\r\n";
         headerline += "Content-Transfer-Encoding: base64\r\n";
         headerline += "Content-Disposition: attachment; filename=\"" + it1->second + "\"\r\n\r\n";
         ret.insert(ret.end(), headerline.begin(), headerline.end());
         headerline.clear();

         ret.insert(ret.end(), it1->first.begin(), it1->first.end());

         // terminate the message with the boundary + "--"
         if((it1 + 1) == attachments.end())
            headerline += "\r\n\r\n--" + boundary + "--\r\n";
         else
            headerline += "\r\n\r\n--" + boundary + "\r\n";
         ret.insert(ret.end(), headerline.begin(), headerline.end());
			headerline.clear();
      }
   }
   else // just a plain text message only
      ret.insert(ret.end(), message.begin(), message.end());

   // end the data in the message.
   headerline = "\r\n.\r\n";
   ret.insert(ret.end(), headerline.begin(), headerline.end());

   return ret;
}

bool mailer::attach(const std::string& filename) {
   if(!filename.length()) // do silly checks.
      return false;

   std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
   if(!file)
      return false;

   std::vector<char> filedata;
   char c = file.get();
   for(; file.good(); c = file.get()) {
      if(file.bad())
         break;
      filedata.push_back(c);
   }

   filedata = base64encode(filedata);

   std::string fn(filename);
   std::string::size_type p = fn.find_last_of('/');
   if(p == std::string::npos)
      p = fn.find_last_of('\\');
   if(p != std::string::npos) {
      p +=1; // get past folder delimeter
      fn = fn.substr(p, fn.length() - p);
   }

   attachments.push_back(std::make_pair(filedata, fn));

   return true;
}

bool mailer::removeattachment(const std::string& filename) {
   if(!filename.length()) // do silly checks.
      return false;

   if(!attachments.size())
      return false;

   std::string fn(filename);
   std::string::size_type p = fn.find_last_of('/');
   if(p == std::string::npos)
      p = fn.find_last_of('\\');
   if(p != std::string::npos) {
      p +=1; // get past folder delimeter
      fn = fn.substr(p, fn.length() - p);
   }
   // fn is now what is stored in the attachments pair as the second parameter
   // i.e.  it->second == fn
   std::vector<std::pair<std::vector<char>, std::string> >::iterator it;
   for(it = attachments.begin(); it < attachments.end(); ++it) {
      if((*it).second == fn) {
         attachments.erase(it);
         return true;
      }
   }
   return false;
}

// returns everything after the '@' synbol in an email address

// if there is no '@' symbol returns the empty string.
std::string mailer::getserveraddress(const std::string& toaddress) const{
   if(toaddress.length()) {
      std::string::size_type pos(toaddress.find("@"));
      if(pos != std::string::npos) { //found the server beginning
         if(++pos < toaddress.length())
            return toaddress.substr(pos, toaddress.length()- pos);
      }
   }
   return "";
}


// this function has to get an MX record for 'server'
// and return its address. Correct form for smtp.
// as the domain 'server' may not be the mail server for the server domain!!
bool mailer::gethostaddresses(std::vector<SOCKADDR_IN>& adds) {
   adds.clear(); // be safe in case of my utter stupidity

   SOCKADDR_IN addr(nameserver, htons(DNS_PORT), AF_INET);

   hostent* host = 0;
   if(addr)
      host = gethostbyaddr(addr.get_sin_addr(), sizeof(addr.ADDR.sin_addr), AF_INET);
   else
      host = gethostbyname(nameserver.c_str());

   if(!host) { // couldn't get to dns, try to connect directly to 'server' instead.
      ////////////////////////////////////////////////////////////////////////////////
      // just try to deliver mail directly to "server"
      // as we didn't get an MX record.
      // addr.sin_family = AF_INET;
      addr = SOCKADDR_IN(server, port);
      addr.ADDR.sin_port = port; // smtp port!! 25
      if(addr) {
         host = gethostbyaddr(addr.get_sin_addr(), sizeof(addr.ADDR.sin_addr), AF_INET);
      }
      else
         host = gethostbyname(server.c_str());

      if(!host) {
         returnstring = "550 Requested action not taken: mailbox unavailable";
         return false; // error!!!
      }

      //memcpy((char*)&addr.sin_addr, host->h_addr, host->h_length);
      std::copy(host->h_addr_list[0], host->h_addr_list[0] + host->h_length, addr.get_sin_addr());
      adds.push_back(addr);

      return true;
   }
   else
      //memcpy((char*)&addr.sin_addr, host->h_addr, host->h_length);
      std::copy(host->h_addr_list[0], host->h_addr_list[0] + host->h_length, addr.get_sin_addr());

   SOCKET s;
   if(!Socket(s, AF_INET, SOCK_DGRAM, 0)) {
      returnstring = "451 Requested action aborted: socket function error";
      return false;
   }

   if(!Connect(s, addr)) {
      returnstring = "451 Requested action aborted: dns server unavailable";
      return false; // dns connection unavailable
   }

   // dnsheader info         id    flags   num queries
   unsigned char dns[512] = {1,1,   1,0,      0,1,      0,0, 0,0, 0,0};
   int dnspos = 12; // end of dns header
   std::string::size_type stringpos(0);
   std::string::size_type next(server.find("."));
   if(next != std::string::npos) { // multipart name e.g. "aserver.somewhere.net"
      while(stringpos < server.length()) {
         std::string part(server.substr(stringpos, next-stringpos));
         dns[dnspos] = part.length();
         ++dnspos;
         for(std::string::size_type i = 0; i < part.length(); ++i, ++dnspos) {
            dns[dnspos] = part[i];
         }

         stringpos = ++next;
         next = server.find(".", stringpos);
         if(next == std::string::npos) {
            part = server.substr(stringpos, server.length() - stringpos);
            dns[dnspos] = part.length();
            ++dnspos;
            for(std::string::size_type i = 0; i < part.length(); ++i, ++dnspos) {
               dns[dnspos] = part[i];
            }
            break;
         }
      }
   }
   else { // just a single part name. e.g. "aserver"
      dns[dnspos] = server.length();
      ++dnspos;

      for(std::string::size_type i = 0; i < server.length(); ++i, ++dnspos) {
         dns[dnspos] = server[i];
      }
   }
   // in case the server string has a "." on the end
   if(server[server.length()-1] == '.')
      dns[dnspos] = 0;
   else
      dns[dnspos++] = 0;

   // add the class & type
   dns[dnspos++] = 0;
   dns[dnspos++] = 15;  // MX record.

   dns[dnspos++] = 0;
   dns[dnspos++] = 1;

   // used to have MSG_DONTROUTE this breaks obviously if you are not
   // running a local nameserver and using it (as I used to do so I didn't
   // notice until now, oops)
   int ret;
   if(!Send(ret, s, (char*)dns, dnspos, 0)) {
      returnstring = "451 Requested action aborted: server seems to have disconnected.";
      Closesocket(s); // clean up
      return false;
   }
   if(Recv(ret, s, (char*)dns, 512, 0)) {
      Closesocket(s);
      // now parse the data sent back from the dns for MX records
      if(dnspos > 12) { // we got more than a dns header back
         unsigned short numsitenames = ((unsigned short)dns[4]<<8) | dns[5];
         unsigned short numanswerRR = ((unsigned short)dns[6]<<8) | dns[7];
         unsigned short numauthorityRR = ((unsigned short)dns[8]<<8) | dns[9];
         unsigned short numadditionalRR = ((unsigned short)dns[10]<<8) | dns[11];

         if(!(dns[3] & 0x0F)) { // check for an error
            // int auth((dns[2] & 0x04)); // AA bit. the nameserver has given authoritive answer.
            int pos = 12; // start after the header.

            std::string questionname;
            if(numsitenames) {
               parsename(pos, dns, questionname);
               pos += 4; // move to the next RR
            }

            // This gives this warning in VC.
            // bloody annoying, there is a way round it according to microsoft.
            // The debugger basically cannot browse anything with a name
            // longer than 256 characters, "get with the template program MS".
            // #pragma warning( disable : 4786 )
            // #pragma warning( default : 4786 )
            std::vector<std::string> names;
            in_addr address;
            std::string name;
            // VC++ incompatability scoping
            // num should be able to be declared in every for loop here
            // not in VC
            int num = 0;
            for(; num < numanswerRR; ++num) {
               name = "";
               parseRR(pos, dns, name, address);
               if(name.length())
                  names.push_back(name);
            }
            for(num = 0; num < numauthorityRR; ++num) {
               name = "";
               parseRR(pos, dns, name, address);
               if(name.length())
                  names.push_back(name);
            }
            for(num = 0; num < numadditionalRR; ++num) {
               name = "";
               parseRR(pos, dns, name, address);
               if(name.length())
                  names.push_back(name);
            }

            // now get all the MX records IP addresess
            addr.ADDR.sin_family = AF_INET;
            addr.ADDR.sin_port = port; // smtp port!! 25
            hostent* host = 0;
            for(vec_str_const_iter it = names.begin(); it < names.end(); ++it) {
               host = gethostbyname(it->c_str());
               if(!host) {
                  addr.zeroaddress();
                  continue; // just skip it!!!
               }
               std::copy(host->h_addr_list[0], host->h_addr_list[0] + host->h_length, addr.get_sin_addr());
               adds.push_back(addr);
            }
            // got the addresses
            return true;
         }
      }
   }
   else
      Closesocket(s);
   // what are we doing here!!
   return false;
}

// we assume the array 'dns' must be 512 bytes in size!
bool mailer::parseRR(int& pos, const unsigned char dns[], std::string& name, in_addr& address) {
   if(pos < 12) // didn,t get more than a header.
      return false;
   if(pos > 512) // oops.
      return false;

   int len = dns[pos];
   if(len >= 192) { // pointer
      int pos1 = dns[++pos];
      len = dns[pos1];
   }
   else { // not a pointer.
      parsename(pos, dns, name);
   }
   // If I do not seperate getting the short values to different
   // lines of code, the optimizer in VC++ only increments pos once!!!
   unsigned short a = ((unsigned short)dns[++pos]<<8);
   unsigned short b = dns[++pos];
   unsigned short Type = a | b;
   a = ((unsigned short)dns[++pos]<<8);
   b = dns[++pos];
   // unsigned short Class = a | b;
   pos += 4; // ttl
   a = ((unsigned short)dns[++pos]<<8);
   b = dns[++pos];
   unsigned short Datalen = a | b;
   if(Type == 15) { // MX record
      // first two bytes the precedence of the MX server
      a = ((unsigned short)dns[++pos]<<8);
      b = dns[++pos];
      // unsigned short order = a | b; // we don't use this here
      len = dns[++pos];
      if(len >= 192) {
         int pos1 = dns[++pos];
         parsename(pos1, dns, name);
      }
      else
         parsename(pos, dns, name);
   }
   else if(Type == 12) { // pointer record
      pos += Datalen+1;
   }
   else if(Type == 2) { // nameserver
      pos += Datalen+1;
   }
   else if(Type == 1) { // IP address, Datalen should be 4.
      pos += Datalen+1;
   }
   else {
      pos += Datalen+1;
   }
   return true;
}

void mailer::parsename(int& pos, const unsigned char dns[], std::string& name) {
   int len = dns[pos];
   if(len >= 192) {
      int pos1 = ++pos;
      ++pos;
      parsename(pos1, dns, name);
   }
   else {
      for(int i = 0; i < len; ++i)
         name += dns[++pos];
      len = dns[++pos];
      if(len != 0)
         name += ".";
      if(len >= 192) {
         int pos1 = dns[++pos];
         ++pos;
         parsename(pos1, dns, name);
      }
      else if(len > 0) {
         parsename(pos, dns, name);
      }
      else if(len == 0)
         ++pos;
   }
}

const std::string& mailer::response() const {
   return returnstring;
}

mailer::Address mailer::parseaddress(const std::string& addresstoparse) {
   Address newaddress; // return value

   // do some silly checks
   if(!addresstoparse.length())
      return newaddress; // its empty, oops (this should fail at the server.)

   if(!addresstoparse.find("@") == std::string::npos) {
      // no '@' symbol (could be a local address, e.g. root)
      // so just assume this. The SMTP server should just deny delivery if its messed up!
      newaddress.address = addresstoparse;
      return newaddress;
   }
   // we have one angle bracket but not the other
   // (this isn't strictly needed, just thought i'd throw it in)
   if(((addresstoparse.find('<') != std::string::npos) &&
      (addresstoparse.find('>') == std::string::npos)) ||
      ((addresstoparse.find('>') != std::string::npos) &&
      (addresstoparse.find('<') == std::string::npos))) {
      return newaddress; // its empty, oops (this should fail at the server.) 
   }

   // we have angle bracketed delimitered address
   // like this maybe:
   //        "foo@bar.com"
   // or     "foo bar <foo@bar.com>"
   // or     "<foo@bar.com> foo bar"
   if((addresstoparse.find('<') != std::string::npos) &&
      (addresstoparse.find('>') != std::string::npos)) {
      std::string::size_type sta = addresstoparse.find('<');
      std::string::size_type end = addresstoparse.find('>');

      newaddress.address = addresstoparse.substr(sta + 1, end - sta - 1);

      if(sta > 0) { // name at the beginning
         // we are cutting off the last character if the bracket address 
         // continues without a space into the bracketed address
         // e.g.  "hoopla girl<hoopla@wibble.com>"
         //       name becomes 'hoopla gir'
         // Fix by David Irwin
         // old code:
         // end = sta -1;
         // newaddress.name = addresstoparse.substr(0, end);
         newaddress.name = addresstoparse.substr(0, sta);
         return newaddress;
      }
      else { // name at the end
         // no name to get
         if(end >= addresstoparse.length()-1)
            return newaddress;

         end += 2;
         if(end >= addresstoparse.length())
            return newaddress; 

         newaddress.name = addresstoparse.substr(end, addresstoparse.length()- end);
         // remove whitespace from end if need be
         if(newaddress.name[newaddress.name.length()-1] == ' ')
            newaddress.name = newaddress.name.substr(0, newaddress.name.length()-1);
         return newaddress;
      }
   }
   // if we get here assume an address of the form: foo@bar.com
   // and just save it.
   newaddress.address = addresstoparse;

   return newaddress;
}

// set the authentication type
void mailer::authtype(const enum authtype Type) {
   assert(Type == LOGIN || Type == PLAIN);
   type = Type;
}
   
// set the username for authentication.
// If this function is called with a non empty string
// jwSMTP will try to use authentication.
// To not use authentication after this, call again
// with the empty string e.g.
//    mailerobject.username("");
void mailer::username(const std::string& User) {
   auth = (User.length() != 0);
   user = User;
}

// set the password for authentication
void mailer::password(const std::string& Pass) {
   pass = Pass;
}

// authenticate against a server.
bool mailer::authenticate(const std::string& servergreeting, const SOCKET& s) {
   assert(auth && user.length()); // shouldn't be calling this function if this is not set!
   int len(0);
   if(!user.length()) { // obvioulsy a big whoops
      Send(len, s, "QUIT\r\n", 6, 0);
      return false;
   }

   // now parse the servergreeting looking for the auth type 'type'
   // if 'type' is not present exit with error (return false)
   std::string at;
   if(type == LOGIN)
      at = "LOGIN";
   else if(type == PLAIN)
      at = "PLAIN";
   else { // oopsy no other auth types yet!! MUST BE A BUG
      assert(false);
      returnstring = "554 jwSMTP only handles LOGIN or PLAIN authentication at present!";
      Send(len, s, "QUIT\r\n", 6, 0);
      return false;
   }

   // uppercase servergreeting first.
   std::string greeting(servergreeting);
   //char ch;
   for(std::string::size_type pos = 0; pos < greeting.length(); ++pos) {
      //ch = greeting[pos];
      greeting[pos] = toupper(greeting[pos] /*ch*/);
   }
   if(greeting.find(at) == std::string::npos) {
      returnstring = "554 jwSMTP only handles LOGIN or PLAIN authentication at present!";
      Send(len, s, "QUIT\r\n", 6, 0);
      return false; // didn't find that type of login!
   }

   // ok try and authenticate to the server.
   const int buffsize(1024);
   char buff[buffsize];
   if(type == LOGIN) {
      greeting = "auth " + at + "\r\n";
      if(!Send(len, s, greeting.c_str(), greeting.length(), 0)) {
         returnstring = "554 send failure: \"auth " + at + "\"";
         return false;
      }
      if(!Recv(len, s, buff, buffsize, 0)) {
         returnstring = "554 receive failure: waiting on username question!";
         return false;
      }
      buff[len] = '\0';
      returnstring = buff;

      // The server should give us a "334 VXNlcm5hbWU6" base64 username
      if(returnstring.substr(0,16) != "334 VXNlcm5hbWU6") {
         // returnstring = "554 Server did not return correct response to \'auth login\' command";
         Send(len, s, "QUIT\r\n", 6, 0);
         return false;
      }
      greeting = base64encode(user, false) + "\r\n";
      if(!Send(len, s, greeting.c_str(), greeting.length(), 0)) {
         returnstring = "554 send failure: sending username";
         return false;
      }
      // now get the password question
      if(!Recv(len, s, buff, buffsize, 0)) {
         returnstring = "554 receive failure: waiting on password question!";
         return false;
      }
      buff[len] = '\0';
      returnstring = buff;
      // The server should give us a "334 UGFzc3dvcmQ6" base64 password
      if(returnstring.substr(0,16) != "334 UGFzc3dvcmQ6") {
         // returnstring = "554 Server did not return correct password question";
         Send(len, s, "QUIT\r\n", 6, 0);
         return false;
      }
      greeting = base64encode(pass, false) + "\r\n";
      if(!Send(len, s, greeting.c_str(), greeting.length(), 0)) {
         returnstring = "554 send failure: sending password";
         return false;
      }
      // now see if we are authenticated.
      if(!Recv(len, s, buff, buffsize, 0)) {
         returnstring = "554 receive failure: waiting on auth login response!";
         return false;
      }
      buff[len] = '\0';
      returnstring = buff;
      if(returnstring.substr(0,3) == "235")
         return true;
   }
   // PLAIN authetication
   else if(type == PLAIN) { // else if not needed, being anal
      // now create the authentication response and send it.
      //       username\0username\0password\r\n
      // i.e.  \0fred\0secret\r\n                 (blank identity)
      std::vector<char> enc;
      std::string::size_type pos = 0;
      for(; pos < user.length(); ++pos)
         enc.push_back(user[pos]);
      enc.push_back('\0');
      for(pos = 0; pos < user.length(); ++pos)
         enc.push_back(user[pos]);
      enc.push_back('\0');
      for(pos = 0; pos < pass.length(); ++pos)
         enc.push_back(pass[pos]);
      
      enc = base64encode(enc, false);
      greeting = "auth plain ";
      for(std::vector<char>::const_iterator it1 = enc.begin(); it1 < enc.end(); ++it1)
         greeting += *it1;
      greeting += "\r\n";

      if(!Send(len, s, greeting.c_str(), greeting.length(), 0)) {
            returnstring = "554 send failure: sending login:plain authenication info";
            return false;
      }
      if(!Recv(len, s, buff, buffsize, 0)) {
         returnstring = "554 receive failure: waiting on auth plain autheticated response!";
         return false;
      }
      buff[len] = '\0';
      returnstring = buff;
      if(returnstring.substr(0,3) == "235")
         return true;
   }

   // fall through return an error.
   Send(len, s, "QUIT\r\n", 6, 0);
   return false;
}

} // end namespace jwsmtp
