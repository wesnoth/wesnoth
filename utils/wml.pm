package wml;
use strict;
use Carp qw(confess);

$wml::open_element = 0, $wml::close_element = 1, $wml::schema_item = 2,
$wml::literal_word = 3, $wml::first_word = 4, $wml::end_words = 256;

$wml::max_schema_size = $wml::end_words - $wml::first_word;
$wml::max_schema_item_length = 20;

sub read_literal_word
{
	my $word = '';
	my ($chars) = @_;
	while(@$chars) {
		my $char = shift @$chars;
		if((ord $char) == 0) {
			return $word;
		}

		$word .= $char;
	}

	confess "Unexpected end of input";
}

sub read_word
{
	my ($schema,$chars) = @_;
	@$chars or confess "Unexpected end of input";
	my $char = shift @$chars;
	my $code = ord $char;
	if($code == $wml::literal_word) {
		return &read_literal_word($chars);
	} elsif($code == $wml::schema_item) {
		my $word = &read_literal_word($chars);
		push @$schema, $word;
		return &read_word($schema,$chars);
	} elsif($code < $wml::first_word) {
		confess "Unexpected character '$code' when word was expected";
	} else {
		my $index = $code - $wml::first_word;
		if($index >= @$schema) {
			confess "Word '$index' ($code) not found in schema";
		}

		return $schema->[$index];
	}
}

sub add_child
{
	my ($doc,$child_name) = @_;
	my $new_element = {'name' => $child_name, 'children' => [], 'attr' => {}};
	push(@{$doc->{'children'}}, $new_element);
	return $new_element;
}

sub read_binary
{
	my ($schema,$input) = @_;

	my $doc = {'name' => '', 'children' => [], 'attr' => {}};
	my @stack = ($doc);
	my $cur = $doc;

	my $len = length $input;
	my @chars;
	for (my $i = 0; $i < $len; $i++) {
		push @chars, substr($input, $i, 1);
	}

	#to see all the characters input for debugging
	#print STDERR "INPUT: " . (join ', ', (map {ord} @chars)) . "\n";

	while(@chars) {
		my $char = shift @chars;
		my $code = ord $char;
		my $remaining = @chars;
		if($code == $wml::open_element) {
			my $word = &read_word($schema,\@chars);
			$cur = &add_child($cur,$word);
			push @stack, $cur;
		} elsif($code == $wml::close_element) {
			pop @stack or confess "Illegal close element found";
			$cur = $stack[$#stack] or confess "Illegal close element found";
		} elsif($code == $wml::schema_item) {
			my $word = &read_literal_word(\@chars);
			push @$schema, $word;
		} else {
			unshift @chars, $char;
			my $name = &read_word($schema,\@chars);
			my $value = &read_literal_word(\@chars);
			$cur->{'attr'}->{$name} = $value;
		}
	}

	if($#stack != 0) {
		my $stack = $#stack;
		confess "Unexpected end of input: $stack items still unclosed";
	}

	return $doc;
}

sub read_text
{
	my ($text) = @_;
	my @chars = split //, $text;

	my $doc = {'name' => '', 'children' => [], 'attr' => {}};
	my @stack = ($doc);
	my $cur = $doc;

	my $token = '';
	my $state = 'SEEK';
	my $name = '';
	my $macro=0;

	foreach my $char (@chars) {
		if($state eq 'SEEK') {
			next if $char =~ /\s/;
			if($char eq '[') {
			    $state = 'ELEMENT';
			    $token = '';
			} elsif($char eq '{') {
				$state = 'MACRO';
				$token = '';
				$macro++;
			} else {
				$state = 'NAME';
				$token .= $char;
			}
		 } elsif($state eq 'MACRO') {
			if($char eq '{'){
			    $macro++;
			}elsif($char eq '}'){
			    $macro--;
			}
			if(0 == $macro){
			    $state = 'SEEK';
			    $token = '';
			}
		} elsif($state eq 'NAME') {
			if($char eq '=') {
				$name = $token;
				$state = 'VALUE';
				$token = '';
			} else {
				$token .= $char unless $char =~ /\s/;
			}
		} elsif($state eq 'VALUE') {
			if($char eq "\n" or $char eq "\r") {
				my @names = ($name);
				my @values = ($token);

				if($name =~ /,/) {
					@names = split /,/, $name;
					@values = split /,/, $token;
				}

				while(@names and @values) {
					my $name = shift @names;
					my $value = shift @values;

					$cur->{'attr'}->{$name} = $value;
				}
				$state = 'SEEK';
				$token = '';
			} elsif($char eq '"') {
				$state = 'VALUE_QUOTE';
			} else {
				$token .= $char;
			}
		} elsif($state eq 'VALUE_QUOTE') {
			if($char eq "\\") {
				$state = 'VALUE_BACKSLASH';
			} elsif($char eq '"') {
				$state = 'VALUE';
			} else {
				$token .= $char;
			}
		} elsif($state eq 'VALUE_BACKSLASH') {
			$token .= $char;
			$state = 'VALUE_QUOTE';
		} elsif($state eq 'ELEMENT') {
			if($char eq '/' and $token eq '') {
				$state = 'CLOSE_ELEMENT';
			} elsif($char eq ']') {
				$cur = &add_child($cur,$token);
				push @stack, $cur;
				$state = 'SEEK';
				$token = '';
			} else {
				$token .= $char;
			}
		}  elsif($state eq 'CLOSE_ELEMENT') {
		    if($char eq ']') {
				my $expected = $cur->{'name'};
				$expected eq $token or confess "close element '$token' doesn't match current open element '$expected'";
				pop @stack or confess "illegal close element '$token'";
				$cur = $stack[$#stack];
				$token = '';
				$state = 'SEEK';
			} else {
				$token .= $char;
			}
		}
	}

	if($state eq 'VALUE') {
		my @names = ($name);
		my @values = ($token);

		if($name =~ /,/) {
			@names = split /,/, $name;
			@values = split /,/, $token;
		}

		while(@names and @values) {
			my $name = shift @names;
			my $value = shift @values;

			$cur->{'attr'}->{$name} = $value;
		}
	} elsif($state ne 'SEEK' or $#stack != 0) {
		confess "unexpected end of WML document: state is '$state', stack size is $#stack";
	}

	return $doc;
}

sub write_word_literal($)
{
	my ($word) = @_;
	return $word . "\0";
}

sub write_word
{
	my $res = '';
	my ($schema,$schema_lookup,$word) = @_;
	if(exists $schema_lookup->{$word}) {
		return chr($wml::first_word + $schema_lookup->{$word});
	}

	my $pos = scalar(@$schema);
	if($pos < $wml::max_schema_size and length($word) < $wml::max_schema_item_length) {
		push @$schema, $word;
		$schema_lookup->{$word} = $pos;
		return chr($wml::schema_item) . &write_word_literal($word) . chr($wml::first_word + $pos);
	} else {
		return chr($wml::literal_word) . &write_word_literal($word);
	}
}

sub write_binary_internal
{
	my $res = '';
	my ($schema,$schema_lookup,$doc) = @_;
	foreach my $name (keys %{$doc->{'attr'}}) {
		my $value = $doc->{'attr'}->{$name};
		$res .= &write_word($schema,$schema_lookup,$name) . &write_word_literal($value);
	}

	foreach my $child (@{$doc->{'children'}}) {
		my $name = $child->{'name'};
		$res .= chr($wml::open_element) .
		        &write_word($schema,$schema_lookup,$name) .
			&write_binary_internal($schema,$schema_lookup,$child) .
		        chr($wml::close_element);
	}

	return $res;
}

sub write_binary
{
	my ($schema,$doc) = @_;

	my %schema_lookup = ();
	my $i = 0;
	foreach my $word (@$schema) {
		$schema_lookup{$word} = $i;
		++$i;
	}

	return &write_binary_internal($schema,\%schema_lookup,$doc);
}

sub write_text
{
	my $doc = shift @_;
	my $indent = shift @_ or 0;
	my $res = '';
	foreach my $name (keys %{$doc->{'attr'}}) {
		my $value = $doc->{'attr'}->{$name};
		$res .= "\t" x $indent;
		$res .= "$name=\"$value\"\n";
	}

	foreach my $child (@{$doc->{'children'}}) {
		my $name = $child->{'name'};
		$res .= "\t" x $indent;
		$res .= "[$name]\n";
		$res .= &write_text($child,$indent+1);
		$res .= "\t" x $indent;
		$res .= "[/$name]\n";
	}

	return $res;
}

sub output_document
{
	my $doc = shift @_;
	my $indent = 0;
	$indent = shift @_ if @_;
	foreach my $name (keys %{$doc->{'attr'}}) {
		my $value = $doc->{'attr'}->{$name};
		print '  ' x $indent;
		print "$name=\"$value\"\n";
	}

	foreach my $child (@{$doc->{'children'}}) {
		my $name = $child->{'name'};
		print '  ' x $indent;
		print "[$name]\n";
		&output_document($child,$indent+1);
		print '  ' x $indent;
		print "[/$name]\n";
	}
}

sub has_child
{
	my ($doc,$name) = @_;
	foreach my $child (@{$doc->{'children'}}) {
		if($child->{'name'} eq $name) {
			return $child;
		}
	}

	return 0;
}

sub get_children
{
	my ($doc,$name) = @_;
	my @res = ();

	foreach my $child (@{$doc->{'children'}}) {
	     if($child->{'name'} eq $name) {
			push @res, $child;
		}
	}

	return @res;
}

sub deep_copy
{
	my ($doc) = @_;
	my $res = {'name' => $doc->{'name'}, 'children' => [], 'attr' => {}};
	my $attrsrc = $doc->{'attr'};
	my $attrdst = $res->{'attr'};
	while(my ($key,$val) = each %$attrsrc) {
		$attrdst->{$key} = $val;
	}

	my $childsrc = $doc->{'children'};
	my $childdst = $res->{'children'};
	foreach my $child (@$childsrc) {
		push @$childdst, &deep_copy($child);
	}

	return $res;
}

sub docs_equal
{
	my ($doc1, $doc2) = @_;
	return 0 if $doc1->{'name'} ne $doc2->{'name'};
	my $attr1 = $doc1->{'attr'};
	my $attr2 = $doc2->{'attr'};
	while(my ($key, $value) = each %$attr1) {
		return 0 if $attr2->{$key} ne $value;
	}

	while(my ($key, $value) = each %$attr2) {
		return 0 if $attr1->{$key} ne $value;
	}

	my $children1 = $doc1->{'children'};
	my $children2 = $doc2->{'children'};

	if(scalar(@$children1) != scalar(@$children2)) {
		return 0;
	}

	for(my $n = 0; $n < @$children1; ++$n) {
		return 0 if not &docs_equal($children1->[$n], $children2->[$n]);
	}

	return 1;
}

sub get_diff
{
	my ($doc1, $doc2, $diff_name) = @_;
	my $diffs = {'name' => $diff_name, 'children' => [], 'attr' => {}};
	my $attr1 = $doc1->{'attr'};
	my $attr2 = $doc2->{'attr'};
	my $d = 0;
	my @keys1 = keys %$attr1;
	my @keys2 = keys %$attr2;

	foreach my $key (@keys1) {
		if($attr1->{$key} ne $attr2->{$key}) {
			my $child = {'name' => 'insert', 'children' => [], 'attr' => {$key => $attr1->{$key}}};
			push @{$diffs->{'children'}}, $child;
			++$d;
		}
	}

	foreach my $key (@keys2) {
		if(undef $attr1->{$key}) {
			my $child = {'name' => 'delete', 'children' => [], 'attr' => {$key => 'x'}};
			push @{$diffs->{'children'}}, $child;
			++$d;
		}
	}

	my $children1 = $doc1->{'children'};
	my $children2 = $doc2->{'children'};

	my %childmap1 = {};
	my %childmap2 = {};
	my %entities = {};

	foreach my $child (@$children1) {
		my $a = $childmap1{$child->{'name'}};
		if(not $a) {
			$a = [$child];
			$childmap1{$child->{'name'}} = $a;
			$entities{$child->{'name'}} = 1;
		} else {
			push @$a, $child;
		}
	}

	foreach my $child (@$children2) {
		my $a = $childmap2{$child->{'name'}};
		if(not $a) {
			$a = [$child];
			$childmap2{$child->{'name'}} = $a;
			$entities{$child->{'name'}} = 1;
		} else {
			push @$a, $child;
		}
	}

	foreach my $key (keys %entities) {
		my $ndeletes = 0;
		my $a1 = ($childmap1{$key} or []);
		my $a2 = ($childmap2{$key} or []);
		for(my $n = 0; $n < @$a1 or $n < @$a2; ++$n) {
			if($n < @$a1 and $n < @$a2) {
				if(not &docs_equal($a1->[$n], $a2->[$n])) {
					my $diff = &get_diff($a1->[$n], $a2->[$n], $key);
					my $change = {'name' => 'change_child', 'children' => $diff->{'children'}, 'attr' => {'index' => $n}};
					push @{$diffs->{'children'}}, $change;
					++$d;
				}
			} elsif($n < @$a1) {
				my $insert = {'name' => 'insert_child', 'children' => [$a1->[$n]], 'attr' => {'index' => $n}};

				push @{$diffs->{'children'}}, $insert;
				++$d;
			} else {
				my $attr = $a2->[$n];
				my $delete = {'name' => 'delete_child', 'children' => [{'name' => $attr->{'name'}, 'children' => [], 'attr' => {}}], 'attr' => {'index' => $n - $ndeletes}};
				++$ndeletes;
				push @{$diffs->{'children'}}, $delete;
				++$d;
			}
		}
	}

	return undef unless $d;
	return {'name' => '', 'children' => [$diffs], 'attr' => {}};
}

sub single_child_doc
{
	my ($name) = @_;
	my $res = {'name' => '', 'children' => [{'name' => $name, 'children' => [], 'attr' => {}}], 'attr' => {}};
	return $res;
}

'Wesnoth Markup Language parser';
