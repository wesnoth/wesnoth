#!/usr/bin/env python3

import gzip, zlib, io
import socket, struct, glob, sys, shutil, threading, os, fnmatch
import wesnoth.wmlparser3 as wmlparser

# See the following files (among others):
# src/addon/*
# src/network.cpp

def append_attributes(tag, **attributes):
    for k, v in list(attributes.items()):
        if isinstance(k, str): k = k.encode("utf8")
        if isinstance(v, str): v = v.encode("utf8")
        kn = wmlparser.AttributeNode(k)
        vn = wmlparser.StringNode(v)
        kn.value.append(vn)
        tag.append(kn)

def append_tag(tag : wmlparser.TagNode, sub : str) -> wmlparser.TagNode:
    subtag = wmlparser.TagNode(sub.encode("utf8"))
    if tag:
        tag.append(subtag)
    return subtag

dumpi = 0
class CampaignClient:
    # First port listed will be used as default.
    portmap = (
        ("15008", "1.13.x"),
        ("15007", "1.12.x"),
        ("15006", "1.11.x"),
        ("15002", "1.10.x"),
        ("15002", "1.9.x"),
        ("15004", "trunk"),
        ("15001", "1.8.x"),
        ("15003", "1.6.x"),
        ("15005", "1.4.x"),
        )

    def __init__(self, address = None, quiet=False):
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
        self.quiet = quiet

        if address != None:
            self.canceled = False
            self.error = False
            s = address.split(":")
            if len(s) == 2:
                self.host, self.port = s
            else:
                self.host = s[0]
                self.port = self.portmap[0][0]
            self.port = int(self.port)
            addr = socket.getaddrinfo(self.host, self.port, socket.AF_INET,
                socket.SOCK_STREAM, socket.IPPROTO_TCP)[0]
            if not self.quiet:
                sys.stderr.write("Opening socket to %s" % address)
            bfwv = dict(self.portmap).get(str(self.port))
            if not self.quiet:
                if bfwv:
                    sys.stderr.write(" for " + bfwv + "\n")
                else:
                    sys.stderr.write("\n")
            self.sock = socket.socket(addr[0], addr[1], addr[2])
            self.sock.connect(addr[4])
            self.sock.send(struct.pack("!I", 0))
            try:
                connection_num = self.sock.recv(4)
            except socket.error:
                connection_num = struct.pack("!I", -1)
                self.error = True
            if not self.quiet:
                sys.stderr.write("Connected as %d.\n" % struct.unpack(
                    "!I", connection_num))

    def async_cancel(self):
        """
        Call from another thread to cancel any current network operation.
        """
        self.canceled = True

    def __del__(self):
        if not self.quiet:
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
        except AttributeError:
            pass # Socket not yet created


    def make_packet(self, doc):
        return doc.wml()

    def send_packet(self, packet):
        """
        Send binary data to the server.
        """
        # Compress the packet before we send it
        fio = io.BytesIO()
        z = gzip.GzipFile(mode = "wb", fileobj = fio)
        z.write(packet)
        z.close()
        zdata = fio.getvalue()

        zpacket = struct.pack("!I", len(zdata)) + zdata
        self.sock.sendall(zpacket)

    def read_packet(self):
        """
        Read binary data from the server.
        """
        packet = b""
        while len(packet) < 4 and not self.canceled:
            r = self.sock.recv(4 - len(packet))
            if not r:
                return None
            packet += r
        if self.canceled:
            return None

        self.length = l = struct.unpack("!I", packet)[0]

        if not self.quiet:
            sys.stderr.write("Receiving %d bytes.\n" % self.length)

        packet = b""
        while len(packet) < l and not self.canceled:
            r = self.sock.recv(l - len(packet))
            if not r:
                return None
            packet += r
            self.counter = len(packet)
        if self.canceled:
            return None

        if not self.quiet:
            sys.stderr.write("Received %d bytes.\n" % len(packet))

        if packet.startswith(b"\x1F\x8B"):
            if self.verbose:
                sys.stderr.write("GZIP compression found...\n")
            bio = io.BytesIO(packet)
            z = gzip.GzipFile("rb", fileobj = bio)
            unzip = z.read()
            z.close()
            packet = unzip

        elif packet.startswith(b'\x78\x9C' ):
            if self.verbose:
                sys.stderr.write("ZLIB compression found...\n")
            packet = zlib.decompres( packet )

        return packet

    def decode( self, data ):
        if self.verbose:
            sys.stderr.write("Decoding text WML...\n")
        data = self.decode_WML( data )

        return data

    def unescape(self, data):
        data2 = bytearray()
        # 01 is used as escape character
        pos = 0
        while True:
            i = data.find(b"\x01", pos)
            if i < 0:
                break
            data2 += data[pos:i]
            data2 += bytes([data[i + 1] - 1])
            pos = i + 2
        data2 += data[pos:]
        return data2

    def decode_WML(self, data):
        p = wmlparser.Parser()
        p.parse_binary(data)
        return p.root

    def encode_WML( self, data ):
        """
        Not needed - not sure this method should even be here.
        """
        pass

    def encode( self, data ):
        """
        Always encode GZIP compressed - future use ZLIB
        """
        pass

    def list_campaigns(self, addon=None):
        """
        Returns a WML object containing all available info from the server.
        """
        if self.error:
            return None
        request = append_tag(None, "request_campaign_list")
        if addon:
            append_attributes(request, name = addon)
        self.send_packet(self.make_packet(request))

        return self.decode(self.read_packet())

    def delete_campaign(self, name, passphrase):
        """
        Deletes the named campaign on the server.
        """
        request = append_tag(None, "delete")
        append_attributes(request, name = name,
            passphrase = passphrase)

        self.send_packet(self.make_packet(request))
        return self.decode(self.read_packet())

    def change_passphrase(self, name, old, new):
        """
        Changes the passphrase of a campaign on the server.
        """
        request = append_tag(None, "change_passphrase")
        append_attributes(request, name = name,
            passphrase = old, new_passphrase = new)

        self.send_packet(self.make_packet(request))
        return self.decode(self.read_packet())

    def get_campaign_raw(self, name):
        """
        Downloads the named campaign and returns it as a raw binary WML packet.
        """
        request = append_tag(None, "request_campaign")
        append_attributes(request, name = name)
        self.send_packet(self.make_packet(request))
        raw_packet = self.read_packet()

        if self.canceled:
            return None

        return raw_packet

    def get_campaign(self, name):
        """
        Downloads the named campaign and returns it as a WML object.
        """

        packet = self.get_campaign_raw(name)

        if packet:
            return self.decode(packet)

        return None

    def put_campaign(self, name, cfgfile, directory, ign, pbl):
        """
        Uploads a campaign to the server.

        The cfgfile is the name of the main .cfg file of the campaign.

        The directory is the name of the campaign's directory.
        """
        request = pbl
        request.name = b"upload"
        append_attributes(request, name = name)
        data = append_tag(request, "data")

        def put_file(name, f):
            for ig in ign:
                if ig and ig[-1] != "/" and fnmatch.fnmatch(name, ig):
                    print(("Ignored file", name))
                    return None
            fileNode = append_tag(None, "file")

            # Order in which we apply escape sequences matters.
            contents = f.read()
            contents = contents.replace(b"\x01", b"\x01\x02" )
            contents = contents.replace(b"\x00", b"\x01\x01")
            contents = contents.replace(b"\x0d", b"\x01\x0e")
            contents = contents.replace(b"\xfe", b"\x01\xff")

            append_attributes(fileNode, name = name)
            append_attributes(fileNode, contents = contents)

            return fileNode

        def put_dir(name, path):
            for ig in ign:
                if ig and ig[-1] == "/" and fnmatch.fnmatch(name, ig[:-1]):
                    print(("Ignored dir", name))
                    return None

            dataNode = append_tag("dir")
            append_attributes(dataNode, name = name)
            for fn in glob.glob(path + "/*"):
                if os.path.isdir(fn):
                    sub = put_dir(os.path.basename(fn), fn)
                else:
                    sub = put_file(os.path.basename(fn), open(fn, 'rb'))
                if sub:
                    dataNode.append(sub)
            return dataNode

        # Only used if it's an old-style campaign directory
        # with an external config.
        if cfgfile:
            data.insert(put_file(name + ".cfg", file(cfgfile)))

        if not self.quiet:
            sys.stderr.write("Adding directory %s as %s.\n" % (directory, name))
        data.append(put_dir(name, directory))

        packet = self.make_packet(request)
        open("packet.dump", "wb").write(packet)
        self.send_packet(packet)

        response = self.read_packet()
        if not response:
            response = b""
        return self.decode(response)

    def get_campaign_raw_async(self, name):
        """
        This is like get_campaign_raw, but returns immediately,
        doing server communications in a background thread.
        """
        class MyThread(threading.Thread):
            def __init__(self, name, client):
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
            def __init__(self, name, cs, args):
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

        mythread = MyThread(args[1], self, args)
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
        for f in data.get_all(tag = "file"):
            name = f.get_text_val("name", "?")
            contents = f.get_binary("contents")
            if not contents:
                contents = b""
                if not self.quiet:
                    sys.stderr.write("File %s is empty.\n" % name)
                    sys.stderr.write(f.debug() + "\n")
            if verbose:
                sys.stderr.write(i * " " + name + " (" +
                      str(len(contents)) + ")\n")
            save = open( os.path.join(path, name), "wb")

            # We MUST un-escape our data
            # Order we apply escape sequences matter here
            contents = self.unescape(contents)
            save.write(contents)
            save.close()

        for dir in data.get_all(tag = "dir"):
            name = dir.get_text_val("name", "?")
            shutil.rmtree(os.path.join(path, name), True)
            os.mkdir(os.path.join(path, name))
            if verbose:
                sys.stderr.write(i * " " + name + "\n")
            self.unpackdir(dir, os.path.join(path, name), i + 2, verbose)

# vim: tabstop=4: shiftwidth=4: expandtab: softtabstop=4: autoindent:
