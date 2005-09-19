#!/usr/bin/ruby

core = ["wesnoth","wesnoth-lib","wesnoth-httt"]
other = ["wesnoth-editor","wesnoth-ei","wesnoth-sotbe","wesnoth-trow"]

def complete?(lang, pack)
	complete = true
	output = `msgfmt -o /dev/null --statistics #{pack}/#{lang}.po 2>&1`
	#print output, "\n"
	if output=~/^\s*(\d+)\s*translated[^\d]+(\d+)\s*fuzzy[^\d]+(\d+)\s*untranslated/
		complete = false
		#print "a)", $1, " ", $2, " ", $3, "\n"
	elsif output=~/^\s*(\d+)\s*translated[^\d]+(\d+)\s*fuzzy[^\d]/
		complete = false
		#print "b)", $1, " ", $2, "\n"
	elsif output=~/^\s*(\d+)\s*translated[^\d]+(\d+)\s*untranslated[^\d]/
		complete = false
		#print "c)", $1, " ", $2, "\n"
	elsif output=~/^\s*(\d+)\s*translated[^\d]+/
		#print "d)", $1, "\n"
	end
	complete
end

print "Uncompressing wesnoth\n"
tarball = Dir.glob("wesnoth*.tar.gz").first
system("tar zxf #{tarball}")
dir = ""
Dir.glob("wesnoth*") { |file|
	if File.directory?(file) 	
		dir = file
	end
}

print "Changing to ", dir, "\n"
Dir.chdir(dir)
print "Erasing unused stuff (The Dark Hordes campaign, unused fonts)\n"
system("rm -rf data/campaigns/The_Dark_Hordes.cfg data/scenarios/The_Dark_Hordes data/maps/The_Dark_Hordes/ images/portraits/The_Dark_Hordes po/wesnoth-tdh")
system("rm -rf fonts/FreeSans.ttf")
system("sed -i -e 's/po\\/wesnoth-tdh\\/Makefile.in //g' configure")
system("sed -i -e '/wesnoth-tdh/d' configure")
system("sed -i -e '/wesnoth-tdh/d' configure.ac")
system("sed -i -e 's/\\$(distdir)\\/po\\/wesnoth-tdh //g' Makefile.in")
print "Changing to po\n"
Dir.chdir("po")
system("sed -i -e 's/wesnoth-tdh //g' Makefile.in Makefile.am")
langs = IO.read("wesnoth/LINGUAS").split(" ")
tokeep = []
toremove = []
langs.each { |lang|
	complete = true
	core.each { |pack|
		complete = (complete and complete?(lang,pack))
	}
	if complete
		tokeep.push(lang)	
	else
		toremove.push(lang)	
	end
}

print "Languages to keep: ", tokeep.join(" "), "\n"
print "Languages to remove: ", toremove.join(" "), "\n"

#Remove language po files
toremove.each { |lang|
	system("rm -rf */#{lang}.po")	
}

#Rewrite LINGUAS file for core packages
core.each { |pack|
	File.open("#{pack}/LINGUAS","w") { |linguas|
		linguas.write("#{tokeep.join(" ")}\n")
	}
}

other.each { |pack|
	tokeepforpack = []
	toremoveforpack = []
	tokeep.each { |lang|
		complete = complete?(lang,pack)
		if complete
			tokeepforpack.push(lang)
		else
			toremoveforpack.push(lang)
		end
	}
	print "Languages to keep for pack #{pack}: ", tokeepforpack.join(" "), "\n"
	print "Languages to remove for pack #{pack}: ", toremoveforpack.join(" "), "\n"
	toremoveforpack.each { |lang|
		system("rm -f #{pack}/#{lang}.po")
	}
	File.open("#{pack}/LINGUAS","w") { |linguas|
		linguas.write("#{tokeepforpack.join(" ")}\n")
	}
}

Dir.chdir("../..")
print "Compressing wesnoth\n"
system("mv #{tarball} #{tarball}.old")
system("tar zcf #{tarball} #{dir}")
system("rm -rf #{dir}")
