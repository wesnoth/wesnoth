package wml;
use strict;

$wml::open_element = 0, $wml::close_element = 1, $wml::schema_item = 2,
$wml::literal_word = 3, $wml::first_word = 4, $wml::end_words = 256;

$wml::max_schema_size = $wml::end_words - $wml::first_word;
$wml::max_schema_item_length = 20;

sub read_literal_word
{
	my $word = "";
	my ($chars) = @_;
	while(@$chars) {
		my $char = shift @$chars;
		if((ord $char) == 0) {
			return $word;
		}

		$word .= $char;
	}

	die "Unexpected end of input";
}

sub read_word
{
	my ($schema,$chars) = @_;
	@$chars or die "Unexpected end of input";
	my $char = shift @$chars;
	my $code = ord $char;
	if($code == $wml::literal_word) {
		return &read_literal_word($chars);
	} elsif($code == $wml::schema_item) {
		my $word = &read_literal_word($chars);
		push @$schema, $word;
		return &read_word($schema,$chars);
	} elsif($code < $wml::first_word) {
		die "Unexpected character '$code' when word was expected";
	} else {
		my $index = $code - $wml::first_word;
		if($index >= @$schema) {
			die "Word '$index' ($code) not found in schema";
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

	my @chars = split //, $input;

	while(@chars) {
		my $char = shift @chars;
		my $code = ord $char;
		my $remaining = $#chars + 1;
		if($code == $wml::open_element) {
			my $word = &read_word($schema,\@chars);
			$cur = &add_child($cur,$word);
			push @stack, $cur;
		} elsif($code == $wml::close_element) {
			pop @stack or die "Illegal close element found";
			$cur = $stack[$#stack] or die "Illegal close element found";
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
		die "Unexpected end of input: $stack items still unclosed";
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


	foreach my $char (@chars) {
		if($state eq 'SEEK') {
			next if $char =~ /\s/;
			if($char eq '[') {
				$state = 'ELEMENT';
				$token = '';
			} else {
				$state = 'NAME';
				$token .= $char;
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
				$expected eq $token or die "close element '$token' doesn't match current open element '$expected'";
				pop @stack or die "illegal close element '$token'";
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
		die "unexpected end of WML document: state is '$state', stack size is $#stack";
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

'Wesnoth Markup Language parser';
