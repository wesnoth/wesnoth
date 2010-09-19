import gzip, zlib, StringIO
import socket, struct, glob, sys, shutil, threading, os, fnmatch
import wesnoth.wmldata as wmldata
import wesnoth.wmlparser as wmlparser
import urllib
import urllib2

# See the following files (among others):
# src/addon_management.cpp
# src/network.cpp

dumpi = 0
class CampaignClient:
    # First port listed will be used as default.
    portmap = (("15002", "1.9.x"), ("15004", "trunk"), ("15001", "1.8.x"), ("15003", "1.6.x"), ("15005", "1.4.x"))

    def __init__(self, address = None):
        """
        Return a new connection to the campaign server at the given address.
        """

        self.length = 0 # length of last processed packet
        self.counter = 0 # position inside above packet
        self.words = {} # dictionary for WML decoder
        self.wordcount = 4 # codewords in above dictionary
        self.codes = {} # dictionary for WML encoder
        self.codescount = 4 # codewords in above dictionary
        self.event = None
        self.name = None
        self.args = None
        self.cs = None
        self.verbose = False
        self.canceled = False
        self.error = False
        self.sock = None
        self.address = "127.0.0.1:8000"
        
        if address != None:
            self.address = address

        """
        if address != None:
            s = address.split(":")
            if len(s) == 2:
                self.host, self.port = s
            else:
                self.host = s[0]
                self.port = self.portmap[0][0]
            self.port = int(self.port)
            addr = socket.getaddrinfo(self.host, self.port, socket.AF_INET,
                socket.SOCK_STREAM, socket.IPPROTO_TCP)[0]
            sys.stderr.write("Opening socket to %s" % address)
            bfwv = dict(self.portmap).get(str(self.port))
            if bfwv:
                sys.stderr.write(" for " + bfwv + "\n")
            else:
                sys.stderr.write("\n")
            self.sock = socket.socket(addr[0], addr[1], addr[2])
            self.sock.connect(addr[4])
            self.sock.send(struct.pack("!l", 0))
            try:
                connection_num = self.sock.recv(4)
            except socket.error:
                connection_num = struct.pack("!l", -1)
                self.error = True
            sys.stderr.write("Connected as %d.\n" % struct.unpack(
                "!l", connection_num))
        """

    def async_cancel(self):
        """
        Call from another thread to cancel any current network operation.
        """
        self.canceled = True

    def __del__(self):
        if self.canceled:
            sys.stderr.write("Canceled socket.\n")
        elif self.error:
            sys.stderr.write("Unexpected disconnection.\n")
        elif self.sock is not None:
            sys.stderr.write("Closing socket.\n")
            try:
                self.sock.shutdown(2)
            except socket.error:
                pass # Well, what can we do?


    def decode( self, data ):
        if self.verbose:
            sys.stderr.write("Decoding text WML...\n")
        data = self.decode_WML( data )

        return data

    def unescape(self, data):
        # 01 is used as escape character
        data2 = ""
        escape = False
        for c in data:
            if escape:
                data2 += chr(ord(c) - 1)
                escape = False
            elif c == "\01":
                escape = True
            else:
                data2 += c
        return data2

    def decode_WML(self, data):
        p = wmlparser.Parser( None, no_macros_in_string=True )
        p.verbose = False
        p.do_preprocessor_logic = True
        p.no_macros = True
        p.parse_text(data, binary=True)
        doc = wmldata.DataSub( "WML" )
        p.parse_top(doc)

        return doc

        def done():
            return pos[0] >= len(data)

        def next():
            c = data[pos[0]]
            pos[0] += 1
            return c

        def literal():
            s = pos[0]
            e = data.find("\00", s)

            pack = data[s:e]

            pack = pack.replace("\01\01", "\00")
            pack = pack.replace("\01\02", "\01")

            pos[0] = e + 1

            return pack

        while not done():
            code = ord(next())
            if code == 0: # open element (name code follows)
                open_element = True
            elif code == 1: # close current element
                tag.pop()
            elif code == 2: # add code
                self.words[self.wordcount] = literal()
                self.wordcount += 1
            else:
                if code == 3:
                    word = literal() # literal word
                else:
                    word = self.words[code] # code
                if open_element: # we handle opening an element
                    element = wmldata.DataSub(word)
                    tag[-1].insert(element) # add it to the current one
                    tag.append(element) # put to our stack to keep track
                elif word == "contents": # detect any binary attributes
                    binary = wmldata.DataBinary(word, literal())
                    tag[-1].insert(binary)
                else: # others are text attributes
                    text = wmldata.DataText(word, literal())
                    tag[-1].insert(text)
                open_element = False

        return WML

    def encode_WML( self, data ):
        """
        Not needed - not sure this method should even be here.
        """
        pass

    def list_campaigns(self):
        """
        Returns a WML object containing all available info from the server.
        """
        if self.error:
            return None

        req = urllib2.Request('http://' + self.address + '/addons/?wml')
        response = urllib2.urlopen(req)
        data = response.read()
        return self.decode(data)

    def validate_campaign(self, name, passphrase):
        """
        Validates python scripts in the named campaign.
        """
        request = wmldata.DataSub("validate_scripts")
        request.set_text_val("name", name)
        request.set_text_val("master_password", passphrase)
        self.send_packet(self.make_packet(request))

        return self.decode(self.read_packet())

    def delete_campaign(self, addon_id, login, password):
        """
        Deletes the named campaign on the server.
        """

        data = urllib.urlencode({"login" : login, "password" : password})
        req = urllib2.Request('http://' + self.address + '/addons/remove/'
                                + str(addon_id) + '/?wml', data)
        
        response = urllib2.urlopen(req)
        data = response.read()

        return self.decode(data)

    def get_campaign_raw(self, id):
        """
        Downloads the named campaign and returns it as a raw binary WML packet.
        """
        req = urllib2.Request('http://' + self.address + '/addons/download/' + id + '/?wml')
        response = urllib2.urlopen(req)
        raw_packet = response.read()

        return raw_packet

    def get_campaign(self, id):
        """
        Downloads the named campaign and returns it as a WML object.
        """

        packet = self.get_campaign_raw(id)
        if packet:
            return self.decode(packet)

        return None

    def put_raw_campaign(self, wmldata, login, password):
        data = urllib.urlencode({"login" : login, "password" : password, "wml" : wmldata})
        req = urllib2.Request('http://' + self.address + '/addons/publish/?wml', data)
        
        response = urllib2.urlopen(req)
        data = response.read()
        return self.decode(data)

    def put_campaign(self, name, cfgfile, directory, ign, stuff, creds):
        """
        Uploads a campaign to the server. The title, name, author, passphrase,
        description, version and icon parameters are what would normally be
        found in a .pbl file.

        The cfgfile is the name of the main .cfg file of the campaign.

        The directory is the name of the campaign's directory.
        """
        wml_data = wmldata.DataSub("WML")
        for k, v in stuff.items():
            wml_data.set_text_val(k, v)
        wml_data.set_text_val("name", name)

        data = wmldata.DataSub("data")
        wml_data.insert(data)

        def put_file(name, f):
            for ig in ign:
                if ig[-1] != "/" and fnmatch.fnmatch(name, ig):
                    print("Ignored file", name)
                    return None
            fileNode = wmldata.DataSub("file")

            # Order in which we apply escape sequences matters.
            contents = f.read()
            contents = contents.replace("\x01", "\x01\x02" )
            contents = contents.replace("\x00", "\x01\x01")
            contents = contents.replace("\x0d", "\x01\x0e")
            contents = contents.replace("\xfe", "\x01\xff")

            fileContents = wmldata.DataText("contents", contents)
            fileNode.insert(fileContents)
            fileNode.set_text_val("name", name)

            return fileNode

        def put_dir(name, path):
            for ig in ign:
                if ig[-1] == "/" and fnmatch.fnmatch(name, ig[:-1]):
                    print("Ignored dir", name)
                    return None
            dataNode = wmldata.DataSub("dir")
            dataNode.set_text_val("name", name)
            for fn in glob.glob(path + "/*"):
                if os.path.isdir(fn):
                    sub = put_dir(os.path.basename(fn), fn)
                else:
                    sub = put_file(os.path.basename(fn), open(fn, 'rb'))
                if sub: dataNode.insert(sub)
            return dataNode

        # Only used if it's an old-style campaign directory
        # with an external config.
        if cfgfile:
            data.insert(put_file(name + ".cfg", file(cfgfile)))

        sys.stderr.write("Adding directory %s as %s.\n" % (directory, name))
        data.insert(put_dir(name, directory))

        return self.put_raw_campaign(wml_data.make_string(), creds['login'], creds['password'])

    def get_campaign_raw_async(self, id):
        """
        This is like get_campaign_raw, but returns immediately, 
        doing server communications in a background thread.
        """
        class MyThread(threading.Thread):
            def __init__(self, id, client):
                threading.Thread.__init__( self, name=str(id) )
                self.id = id
                self.cs = client

                self.event = threading.Event()
                self.data = None

            def run(self):
                data = self.cs.get_campaign_raw(self.name)
                self.data = data
                self.event.set()

            def cancel(self):
                self.data = None
                self.event.set()
                self.cs.async_cancel()

        mythread = MyThread( id, self )
        mythread.start()

        return mythread

    def unpackdir(self, data, path, i = 0, verbose = False):
        """
        Call this to unpack a campaign contained in a WML object 
        to the filesystem. The data parameter is the WML object, 
        path is the path under which it will be placed.
        """
        try:
            os.mkdir(path)
        except OSError:
            pass

        if data.get_all("data") != []:
            data = data.get_first("data")

        for f in data.get_all("file"):
            name = f.get_text_val("name", "?")
            contents = f.get_text("contents")
            if not contents:
                contents = ""
                sys.stderr.write("File %s is empty.\n" % name)
                sys.stderr.write(f.debug(write = False) + "\n")
            if contents:
                contents = contents.get_value()
            if verbose:
                sys.stderr.write(i * " " + name + " (" +
                      str(len(contents)) + ")\n")
            save = file( os.path.join(path.strip(), name.strip()), "wb")

            # We MUST un-escape our data
            # Order we apply escape sequences matter here
            contents = self.unescape(contents)
            save.write(contents)
            save.close()

        for dir in data.get_all("dir"):
            name = dir.get_text_val("name", "?")
            shutil.rmtree(os.path.join(path, name), True)
            mk = os.path.join(path, name).strip()
            if path == ".": mk = name.strip()
            os.mkdir(mk.strip())
            if verbose:
                sys.stderr.write(i * " " + name + "\n")
            self.unpackdir(dir, os.path.join(path, name), i + 2, verbose)

# vim: tabstop=4: shiftwidth=4: expandtab: softtabstop=4: autoindent:
