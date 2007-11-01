#!/usr/bin/perl -w

if($#ARGV !=1){
    die"TeamColorizer.pl input-file output-file\n";
}
$infilename=shift(@ARGV);
$outfilename=shift(@ARGV);

$team_red=255;
$team_green=0;
$team_blue=0;
$team_red_max=255;
$team_green_max=255;
$team_blue_max=255;
$team_red_min=0;
$team_green_min=0;
$team_blue_min=0;

$flag_rgb="244,154,193,63,0,22,85,0,42,105,0,57,123,0,69,140,0,81,158,0,93,177,0,105,195,0,116,214,0,127,236,0,140,238,61,150,239,91,161,241,114,172,242,135,182,246,173,205,248,193,217,250,213,229,253,233,241";
#$flag_rgb="236,0,140,244,154,193,63,0,22,85,0,42,105,0,57,123,0,69,140,0,81,158,0,93,177,0,105,195,0,116,214,0,127,238,61,150,239,91,161,241,114,172,242,135,182,246,173,205,248,193,217,250,213,229,253,233,241";

@flag=split(/,/,$flag_rgb);

if($#flag<3){die "error, flag_rgb not well defined";}

$base_red=$flag[0];
$base_green=$flag[1];
$base_blue=$flag[2];
$base_sum=$base_red+$base_green+$base_blue;

while($#flag>2){
    $red=shift(@flag);
    $green=shift(@flag);
    $blue=shift(@flag);
    $old_rgb=sprintf("%lx", $red*256*256+$green*256+$blue);

    $sum=$red+$green+$blue;
    if($sum<=$base_sum){
	$ratio=$sum/$base_sum;
	$new_red=$team_red*$ratio+$team_red_min*(1-$ratio);
	$new_green=$team_green*$ratio+$team_green_min*(1-$ratio);
	$new_blue=$team_blue*$ratio+$team_blue_min*(1-$ratio);
    }else{
	$ratio=$base_sum/$sum;
	$new_red=$team_red*$ratio+$team_red_max*(1-$ratio);
	$new_green=$team_green*$ratio+$team_green_max*(1-$ratio);
	$new_blue=$team_blue*$ratio+$team_blue_max*(1-$ratio);
    }
    $new_red=sprintf("%d",$new_red);
    $new_green=sprintf("%d",$new_green);
    $new_blue=sprintf("%d",$new_blue);
    $new_rgb=sprintf("%lx", $new_red*256*256+$new_green*256+$new_blue);

#    print "red: $red\tgreen: $green\tblue: $blue\t$old_rgb\n";
#    print "\tred: $new_red\tgreen: $new_green\tblue: $new_blue\t$new_rgb\n";
    $fill_cmd="-fill \"#$new_rgb\" -opaque \"#$old_rgb\"";
#    print "$fill_cmd\n";
    push(@fill,$fill_cmd);
}

$convert="convert ";
foreach(@fill){
#    print "$_";
    $convert="$convert $_";
}
$convert = "$convert $infilename $outfilename";
#print "$convert\n";
system($convert);
