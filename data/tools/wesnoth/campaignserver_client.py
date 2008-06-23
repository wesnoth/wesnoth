import gzip, zlib, StringIO
import socket, struct, glob, sys, shutil, threading, os
import wesnoth.wmldata as wmldata
import wesnoth.wmlparser as wmlparser


EMPTY_STRING = ''
GZ_WRITE_MODE = 'wb'
GZ_READ_MODE = 'rb'
BWML_PREFIXES = "\x00\x01\x02\x03"


class CampaignClient:
    # First port listed will be used as default.
    portmap = (("15003", "1.5.x"), ("15004", "1.2.x"), ("15005", "1.4.x"))
    # Files with these suffixes will not be downloaded
    excluded = ("~", "-bak", ".pbl", ".exe", ".com", ".bat", ".scr", ".sh")

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
        self.gzIn = StringIO.StringIO()
        self.gzOut = StringIO.StringIO()
        self.event = None
        self.name = None
        self.args = None
        self.cs = None

        if address != None:
            s = address.split(":")
            if len(s) == 2:
                self.host, self.port = s
            else:
                self.host = s[0]
                self.port = self.portmap[0][0]
            self.port = int(self.port)
            self.canceled = False
            self.error = False
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
        else:
            sys.stderr.write("Closing socket.\n")
        try:
            self.sock.shutdown(2)
        except socket.error:
            pass # Well, what can we do?

    def isBWML( self, packet ):
        """
        Return True if packet is encoded as binary WML. Else
        return False.
        """
        if packet[0] in BWML_PREFIXES:
            return True

        return False

    def makePacket( self, doc ):
        root = wmldata.DataSub( "WML" )
        root.insert( doc )
        return root.make_string()

    def send_packet(self, packet):
        """
        Send binary data to the server.
        """
        # Compress the packet before we send it
        z = gzip.GzipFile( EMPTY_STRING,
                           mode=GZ_WRITE_MODE,
                           fileobj=self.gzOut )
        z.write( packet )
        z.close()
        zdata = self.gzOut.getvalue()
        self.gzOut.seek(0)
        self.gzOut.truncate()
        zpacket = struct.pack("!l", len(zdata)) + zdata
        self.sock.sendall( zpacket )

    def read_packet(self, shortRead=False):
        """
        Read binary data from the server.
        shortRead is a hack to work around extra data delivered for unknown reason!
        -oracle
        """
        lenPacket = self.sock.recv(4)
        self.length = l = struct.unpack("!l", lenPacket)[0]
        packet = ""
        while len(packet) < l and not self.canceled:
            packet += self.sock.recv(l - len(packet))
            self.counter = len(packet)
        if self.canceled:
            return None

        if packet.startswith( '\x1F\x8B' ):
            # If GZIP compressed, decompress - ignoring last byte
            if shortRead:
                self.gzIn.write( packet[:-1] )

            else:
                self.gzIn.write( packet )
            self.gzIn.seek(0)
            z = gzip.GzipFile( EMPTY_STRING,
                               mode=GZ_READ_MODE,
                               fileobj=self.gzIn )
            packet = z.read()
            z.close()
            self.gzIn.seek(0)
            self.gzIn.truncate()

        elif packet.startswith( '\x78\x9C' ):
            # If ZLIB compressed, decompress
            packet = zlib.decompres( packet )

        return packet

    def decode( self, data ):
        if self.isBWML( data ):
            data = self.decode_BWML( data )

        else:
            data = self.decode_WML( data )

        return data

    def decode_WML( self, data ):
        p = wmlparser.Parser( None, no_macros_in_string=True )
        p.verbose = False
        p.do_preprocessor_logic = True
        p.no_macros = True
        p.parse_text( data, binary=True )
        doc = wmldata.DataSub( "WML" )
        p.parse_top( doc )

        return doc

    def decode_BWML(self, data):
        """
        Given a block of binary data, decode it as binary WML 
        and return it as a WML object.

        The binary WML format is a byte stream. The format is:
        0 3 <name> means a new element <name> is opened
        0 4..255 means the same as 0 3 but with a coded name
        1 means the current element is closed
        2 <name> means, add a new code for <name> to the dictionary
        3 <name> <data> means, a new attribute <name> is added with <data>
        4..255 <data> means, the same as above but with a coded name

        For example if we got:
        [message]text=blah[/message]
        [message]text=bleh[/message]
        It would look like:
        0 2 message 4 2 text 5 blah 1 0 4 5 bleh 1
        This would be the same: 
        0 3 message 3 text blah 1 0 3 message 3 text bleh 1
        """
        WML = wmldata.DataSub("campaign_server")
        pos = [0]
        tag = [WML]
        open_element = False

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

    def encode_BWML(self, data):
        """
        Given a WML object, encode it into binary WML 
        and return it as a python string.
        """
        packet = str("")

        def literal(data):
            if not data:
                return str("")
            data = data.replace("\01", "\01\02")
            data = data.replace("\00", "\01\01")
            return str(data)

        def encode(name):
            if name in self.codes:
                return self.codes[name]
            what = literal(name)
            # Gah.. why didn't nobody tell me about the 20 characters limit?
            # Without adhering to this (hardcoded in the C++ code) limit, the
            # campaign server simply will crash if we talk to it.
            if len(what) <= 20 and self.codescount < 256:
                data = "\02" + what + "\00" + chr(self.codescount)
                self.codes[name] = chr(self.codescount)
                self.codescount += 1
                return data
            return "\03" + what + "\00"

        if isinstance(data, wmldata.DataSub):
            packet += "\00" + encode(data.name)
            for item in data.data:
                encoded = self.encode_BWML(item)
                packet += encoded
            packet += "\01"
        elif isinstance(data, wmldata.DataText):
            packet += encode(data.name)
            packet += literal(data.data) + "\00"
        elif isinstance(data, wmldata.DataBinary):
            packet += encode(data.name)
            packet += literal(data.data) + "\00"
        return packet

    def encode( self, data ):
        """
        Always encode GZIP compressed - future use ZLIB
        """
        pass

    def list_campaigns(self):
        """
        Returns a WML object containing all available info from the server.
        """
        if self.error:
            return None
        request = wmldata.DataSub("request_campaign_list")
        self.send_packet( self.makePacket( request ) )

        # Passing True to read_packet is a hack - likely a campaignd bug
        return self.decode( self.read_packet( True ) )

    def validate_campaign(self, name, passphrase):
        """
        Validates python scripts in the named campaign.
        """
        request = wmldata.DataSub("validate_scripts")
        request.set_text_val("name", name)
        request.set_text_val("master_password", passphrase)
        self.send_packet( self.makePacket( request ) )

        return self.decode( self.read_packet() )

    def delete_campaign(self, name, passphrase):
        """
        Deletes the named campaign on the server.
        """
        request = wmldata.DataSub("delete")
        request.set_text_val("name", name)
        request.set_text_val("passphrase", passphrase)

        self.send_packet( self.makePacket( request ) )
        return self.decode( self.read_packet( True) )

    def change_passphrase(self, name, old, new):
        """
        Changes the passphrase of a campaign on the server.
        """
        request = wmldata.DataSub("change_passphrase")
        request.set_text_val("name", name)
        request.set_text_val("passphrase", old)
        request.set_text_val("new_passphrase", new)

        self.send_packet( self.makePacket( request ) )
        return self.decode( self.read_packet() )

    def get_campaign_raw(self, name):
        """
        Downloads the named campaign and returns it as a raw binary WML packet.
        """
        request = wmldata.DataSub("request_campaign")
        request.insert( wmldata.DataText("name", name) )
        self.send_packet( self.makePacket( request ) )
        raw_packet = self.read_packet()
        packet = self.decode( raw_packet )

        if self.canceled:
            return None

        return packet

    def get_campaign(self, name):
        """
        Downloads the named campaign and returns it as a WML object.
        """

        packet = self.get_campaign_raw(name)

        if packet:
            return self.decode( packet )

        return None

    def put_campaign(self, title, name, author, passphrase, description,
            version, icon, cfgfile, directory):
        """
        Uploads a campaign to the server. The title, name, author, passphrase,
        description, version and icon parameters are what would normally be
        found in a .pbl file.

        The cfgfile is the name of the main .cfg file of the campaign.

        The directory is the name of the campaign's directory.
        """
        request = wmldata.DataSub("upload")
        request.set_text_val("title", title)
        request.set_text_val("name", name)
        request.set_text_val("author", author)
        request.set_text_val("passphrase", passphrase)
        request.set_text_val("description", description)
        request.set_text_val("version", version)
        request.set_text_val("icon", icon)
        dataNode = wmldata.DataSub( "data" )
        request.insert( dataNode )

        def put_file(name, f):
            fileNode = wmldata.DataSub("file")
            fileNode.set_text_val("name", name)

            # Order we apply escape sequences matters
            contents = f.read().replace( "\x01", "\x01\x02" ).replace( "\x00", "\x01\x01" )
##            import time
##            contents = str(time.ctime())
            fileContents = wmldata.DataText( "contents", contents )
            fileNode.insert( fileContents )
            
            return fileNode

        def put_dir(name, path):
            print "put_dir(", name, path, ")"
            dataNode = wmldata.DataSub("dir")
            dataNode.set_text_val("name", name)
            for fn in glob.glob(path + "/*"):
                if os.path.isdir(fn):
                    sub = put_dir(os.path.basename(fn), fn)
                elif filter(lambda x: fn.endswith(x), CampaignClient.excluded):
                    continue
                else:
                    sub = put_file(os.path.basename(fn), file(fn))
                dataNode.insert(sub)
            return dataNode

        # Only used if it's an old-style campaign directory
        # with an external config.
        if os.path.exists(cfgfile):
            dataNode.insert( put_file(name + ".cfg", file(cfgfile)) )

        print "putting dir", name, os.path.basename(directory)
        dataNode.insert( put_dir(name, directory) )

        request.debug()
        print
##        print "packet:", self.makePacket( request )
        print "packet len:", len(self.makePacket( request ))
        print
        self.send_packet( self.makePacket( request ) )

        return self.decode( self.read_packet( True ) )

    def get_campaign_async(self, name, raw = False):
        """
        This is like get_campaign, but returns immediately, 
        doing server communications in a background thread.
        """
        class MyThread(threading.Thread):
            def __init__( self, name, client ):
                threading.Thread.__init__( self, name=name )
                self.name = name
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

        mythread = MyThread( name, self )
        mythread.start()

        return mythread

    def put_campaign_async(self, *args):
        """
        This is like put_campaign, but returns immediately, 
        doing server communications in a background thread.
        """
        class MyThread(threading.Thread):
            def __init__( self, name, cs, args ):
                threading.Thread.__init__( self, name=name )
                self.name = name
                self.cs = cs
                self.args = args
                self.data = None

                self.event = threading.Event()
                
            def run(self):
                self.data = self.cs.put_campaign(*self.args)
                self.event.set()

            def cancel(self):
                self.data = None
                self.event.set()
                self.cs.async_cancel()

        mythread = MyThread( args[1], self, args )
        mythread.start()

        return mythread

    def unpackdir(self, data, path, i = 0, verbose = False):
        """
        Call this to unpack a campaign contained in a WML object 
        to the filesystem. The data parameter is the WML object, 
        path is the path under which it will be placed.
        """

        data.debug()
        try:
            os.mkdir(path)
        except OSError:
            pass
        for f in data.get_all("file"):
            name = f.get_text_val("name", "?")
            contents = f.get_text("contents").get_value()
            if verbose:
                print i * " " + name + " (" +\
                      str(len(contents)) + ")"
            save = file( os.path.join(path, name), "wb")

            # We MUST un-escape our data
            # Order we apply escape sequences matter here
            contents = contents.replace( "\x01\x01", "\x00" )
            contents = contents.replace( "\x01\x02", "\x01" )
            save.write( contents )
            save.close()

        for dir in data.get_all("dir"):
            name = dir.get_text_val("name", "?")
            shutil.rmtree(os.path.join(path, name), True)
            os.mkdir(os.path.join(path, name))
            if verbose:
                print i * " " + name
            self.unpackdir(dir, os.path.join(path, name), i + 2, verbose)

# vim: tabstop=4: shiftwidth=4: expandtab: softtabstop=4: autoindent:
