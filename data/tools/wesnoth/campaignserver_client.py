import socket, struct, glob, sys, shutil, threading, os
import wesnoth.wmldata as wmldata

class CampaignClient:
    # First port listed will be used as default.
    portmap = (("15003", "1.3.x"), ("15004", "1.2.x"))

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

    def send_packet(self, packet):
        """
        Send binary data to the server.
        """
        packet = struct.pack("!l", len(packet)) + packet
        l = len(packet)
        self.length = l
        s = 0
        while s < l and not self.canceled:
            s += self.sock.send(packet[s:])
            self.counter = s

    def read_packet(self):
        """
        Read binary data from the server.
        """
        l = struct.unpack("!l", self.sock.recv(4))[0]
        self.length = l
        packet = ""
        while len(packet) < l and not self.canceled:
            packet += self.sock.recv(l - len(packet))
            self.counter = len(packet)
        if self.canceled: return None
        return packet

    def decode_WML(self, data):
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
                if code == 3: word = literal() # literal word
                else: word = self.words[code] # code
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

    def encode_WML(self, data):
        """
        Given a WML object, encode it into binary WML 
        and return it as a python string.
        """
        packet = str("")

        def literal(data):
            if not data: return str("")
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
                encoded = self.encode_WML(item)
                packet += encoded
            packet += "\01"
        elif isinstance(data, wmldata.DataText):
            packet += encode(data.name)
            packet += literal(data.data) + "\00"
        elif isinstance(data, wmldata.DataBinary):
            packet += encode(data.name)
            packet += literal(data.data) + "\00"
        return packet

    def list_campaigns(self):
        """
        Returns a WML object containing all available info from the server.
        """
        if self.error: return None
        request = wmldata.DataSub("request_campaign_list")
        packet = self.encode_WML(request)
        self.send_packet(packet);
        packet = self.read_packet()
        data = self.decode_WML(packet)
        return data

    def validate_campaign(self, name, passphrase):
        """
        Validates python scripts in the named campaign.
        """
        request = wmldata.DataSub("validate_scripts")
        request.set_text_val("name", name)
        request.set_text_val("master_password", passphrase)
        packet = self.encode_WML(request)
        self.send_packet(packet);
        packet = self.read_packet()
        data = self.decode_WML(packet)
        return data

    def delete_campaign(self, name, passphrase):
        """
        Deletes the named campaign on the server.
        """
        request = wmldata.DataSub("delete")
        request.set_text_val("name", name)
        request.set_text_val("passphrase", passphrase)
        packet = self.encode_WML(request)
        self.send_packet(packet);
        packet = self.read_packet()
        data = self.decode_WML(packet)
        return data

    def change_passphrase(self, name, old, new):
        """
        Changes the passphrase of a campaign on the server.
        """
        request = wmldata.DataSub("change_passphrase")
        request.set_text_val("name", name)
        request.set_text_val("passphrase", old)
        request.set_text_val("new_passphrase", new)
        packet = self.encode_WML(request)
        self.send_packet(packet);
        packet = self.read_packet()
        data = self.decode_WML(packet)
        return data

    def get_campaign_raw(self, name):
        """
        Downloads the named campaign and returns it as a raw binary WML packet.
        """

        request = wmldata.DataSub("request_campaign")
        request.insert(wmldata.DataText("name", name))
        packet = self.encode_WML(request)

        self.send_packet(packet);
        packet = self.read_packet()

        if self.canceled: return None

        return packet

    def get_campaign(self, name):
        """
        Downloads the named campaign and returns it as a WML object.
        """

        packet = self.get_campaign_raw(name)

        if packet: return self.decode_WML(packet)
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
        data = wmldata.DataSub("data")

        def put_file(name, f):
            data = wmldata.DataSub("file")
            data.set_text_val("name", name)
            data.insert(wmldata.DataBinary("contents", f.read()))
            return data

        def put_dir(name, path):
            data = wmldata.DataSub("dir")
            data.set_text_val("name", name)
            for fn in glob.glob(path + "/*"):
                if os.path.isdir(fn):
                    sub = put_dir(os.path.basename(fn), fn)
                elif fn.endswith("~") or fn.endswith("-bak") or ".pbl" in fn:
                    continue
                else:
                    sub = put_file(os.path.basename(fn), file(fn))
                data.insert(sub)
            return data

        # Only used if it's an old-style campaign directory
        # with an external config.
        if os.path.exists(cfgfile):
            data.insert(put_file(name + ".cfg", file(cfgfile)))
        data.insert(put_dir(name, directory))
        request.insert(data)

        packet = self.encode_WML(request)

        self.send_packet(packet);
        packet = self.read_packet()
        data = self.decode_WML(packet)
        return data

    def get_campaign_async(self, name, raw = False):
        """
        This is like get_campaign, but returns immediately, 
        doing server communications in a background thread.
        """
        class MyThread(threading.Thread):
            def run(self):
                if raw:
                    data = self.cs.get_campaign_raw(self.name)
                else:
                    data = self.cs.get_campaign(self.name)
                self.data = data
                self.event.set()

            def cancel(self):
                self.data = None
                self.event.set()
                self.cs.async_cancel()

        mythread = MyThread()
        mythread.cs = self
        mythread.name = name
        mythread.event = threading.Event()
        mythread.start()

        return mythread

    def put_campaign_async(self, *args):
        """
        This is like put_campaign, but returns immediately, 
        doing server communications in a background thread.
        """
        class MyThread(threading.Thread):
            def run(self):
                data = self.cs.put_campaign(*self.args)
                self.data = data
                self.event.set()

            def cancel(self):
                self.data = None
                self.event.set()
                self.cs.async_cancel()

        mythread = MyThread()
        mythread.cs = self
        mythread.name = args[1]
        mythread.args = args
        mythread.event = threading.Event()
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
        except:
            pass
        for f in data.get_all("file"):
            name = f.get_text_val("name", "?")
            contents = f.get_binary_val("contents")
            if contents:
                if verbose:
                    print i * " " + name + " (" +\
                        str(len(contents)) + ")"
                file(path + "/" + name, "wb").write(contents)
        for dir in data.get_all("dir"):
            name = dir.get_text_val("name", "?")
            shutil.rmtree(path + "/" + name, True)
            os.mkdir(path + "/" + name)
            if verbose:
                print i * " " + name
            self.unpackdir(dir, path + "/" + name, i + 2, verbose)

# vim: tabstop=4: shiftwidth=4: expandtab: softtabstop=4: autoindent:
