#!/usr/bin/ruby
# A script to create the "Terrain Table" on the TerrainLettersWML wiki page.
# Run this and splice the outtput into the wiki whenever you add a new 
# terrain type to mainline.


#create an array of hashes, each hash representing a [terrain] tag
#removes "", _ "" and whitespaces from a value
def get_value(value)
	return "" if !value
	re=value
	if value=~/_ "(.*)"/ then
		re=$1
	elsif value=~/"(.*)"/ then
		re=$1
	end
	return re.strip
end

def tags_to_hashes(text)
	rearray=[]
	while text=~/\[terrain\]\s*\n(.*?)\n\[\/terrain\]/m
		text=$'#remove the text that has already been parsed
		content=$1
		#create a hash from the tag's attributes
		content_hash={}
		content.each do |line|
			line=line.chomp("\n").strip
			key,value=line.split("=")
			if key&&value then
				content_hash[key]=get_value(value)
			end
		end
		rearray<<content_hash
	end
	return rearray
end

#create a hash where each terrain string is pointing to the correspondending name(e.g. "Gg"=>"Grassland")
def string_to_name_hash(terrains)
	rehash={}
	terrains.each do |terrain|
		string=terrain["string"]
		name=terrain["name"]
		rehash[string]=name
	end
	return rehash
end

def create_table_line(string,name,stats_from)
	return "<tr>
<td>#{string}</td>
<td>#{name}</td>
<td>#{stats_from}</td>
</tr>"
end

#create wiki text from array in that terrain tag data is stored
def create_wiki(terrains)

	string_to_name=string_to_name_hash(terrains)

	table_lines=""
	terrains.each do |terrain|
		string=terrain["string"]
		name=terrain["name"]
		stats_from=""
		#convert the terrain strings from the aliasof into terrain names
		terrain["aliasof"]="" if !terrain["aliasof"]
		terrain["aliasof"].split(",").each do |item|
			stats_from+=string_to_name[item.strip].to_s+", "
		end
		stats_from.chomp!(", ")
		table_lines+=create_table_line(string,name,stats_from)+"\n"
	end

return "
<table border=\"1\"><tr>
<th>String</th>
<th>Name</th>
<th>Stats from</th>
</tr>
#{table_lines}
</table>"
end

##input/output engine
def write_file(pat,text)
	file=File.new(pat,"w")
	file.print(text)
	file.close
	return true
end

$terrain_file=ARGV[1]
$output_file=ARGV[0]

while !$terrain_file
	print "Path of terrain.cfg: "
	$terrain_file=gets.chomp("\n")
end

while !$output_file
	print "Where will the wiki text be saved?"
	$output_file=gets.chomp("\n")
end

terrain=File.new($terrain_file).read
terrainarray=tags_to_hashes(terrain)
output=create_wiki(terrainarray)
write_file($output_file,output)
