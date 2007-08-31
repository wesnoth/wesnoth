#!/usr/bin/ruby
# A script to create the "Terrain Table" on the TerrainLettersWML wiki page.


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

#create a hash where each 1.3+ terrain string is pointing to the correspondending 1.2 terrain string(e.g. "Gg"=>"g")
def string_to_oldstring_hash(terrains,old_terrains)
	rehash={}
	#look for identical IDs in the old_terrain array and the terrain array
	terrains.each do |terrain|
		id=terrain["id"]
		old_terrains.each do |old_terrain|
			old_id=old_terrain["id"]
			if old_id==id then
				#if two terrains with the same id are found, add a key with the name of the new string to the hash that points to the old char
				rehash[terrain["string"]]=old_terrain["char"]
			end
		end
	end
	return rehash
end

def create_table_line(string,old_letter,name,stats_from,show_old_letter)
	return "<tr>
<td>#{string}</td>#{"
<td>#{old_letter}</td>" if show_old_letter}
<td>#{name}</td>
<td>#{stats_from}</td>
</tr>"
end

#create wiki text from array in that terrain tag data is stored
def create_wiki(terrains,old_terrains=nil)

	string_to_name=string_to_name_hash(terrains)
	string_to_oldstring=string_to_oldstring_hash(terrains,old_terrains) if old_terrains

	table_lines=""
	terrains.each do |terrain|
		string=terrain["string"]
		oldstring=string_to_oldstring[terrain["string"]] if old_terrains#if old terrains should be shown, get the correspondending old terrain from the hash
		name=terrain["name"]
		stats_from=""
		#convert the terrain strings from the aliasof into terrain names
		terrain["aliasof"]="" if !terrain["aliasof"]
		terrain["aliasof"].split(",").each do |item|
			stats_from+=string_to_name[item.strip].to_s+", "
		end
		stats_from.chomp!(", ")
		table_lines+=create_table_line(string,oldstring,name,stats_from,old_terrains)+"\n"
	end

return "
<table border=\"1\"><tr>
<th>String</th>#{"
<th>Old letter</th>" if old_terrains}
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

$old_terrain_file=ARGV[2]
$terrain_file=ARGV[1]
$output_file=ARGV[0]


while !$terrain_file
	print "Path of terrain.cfg: "
	$terrain_file=gets.chomp("\n")
end

if !$old_terrain_file
	print "Terrain.cfg used for creating old path chars(optional): "
	$old_terrain_file=gets.chomp("\n")
end

while !$output_file
	print "Where will the wiki text be saved?"
	$output_file=gets.chomp("\n")
end

terrain=File.new($terrain_file).read
if File.exists?($old_terrain_file)
	old_terrain=File.new($old_terrain_file).read
else
	print "No old terrain.cfg found.\n\n"
end
terrainarray=tags_to_hashes(terrain)
old_terrainarray=tags_to_hashes(old_terrain) if old_terrain
output=create_wiki(terrainarray,old_terrainarray) if old_terrain
output=create_wiki(terrainarray) if !old_terrain
write_file($output_file,output)
