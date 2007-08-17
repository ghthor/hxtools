#!/usr/bin/perl
#
#	png2wx - embed png in C++
#	by Jan Engelhardt <jengelh [at] gmx de>, 2004 - 2007
#	http://jengelh.hopto.org/
#	released in the Public Domain
#

use Getopt::Long;
use strict;

&main(\@ARGV);

#------------------------------------------------------------------------------
sub main($)
{
	my($cpp_file, $hpp_file, $hpp_include, $Marker, $main, $tmp);
	&Getopt::Long::Configure(qw(bundling));
	&GetOptions(
		"C=s"  => \$cpp_file,
		"H=s"  => \$hpp_file,
		"M=s" => \$Marker,
	);

	if ($cpp_file eq "" || $hpp_file eq "" || $Marker eq "") {
		die "You need to specify -C, -H and -M options.\n";
	}

	$hpp_include = ($hpp_file =~ m{/([^/]*)$})[0];

	#
	# C++ header
	#
	open(HPP, "> $hpp_file") || warn "Could not open $hpp_file: $!\n";
	$tmp = uc $hpp_file;
	$tmp =~ s/[^A-Z0-9]/_/g;
	print HPP "/* Autogenerated by png2wx.pl on ",
	          scalar(localtime()), " */\n";
	print HPP <<"--EOF";
#ifndef $Marker
#define $Marker 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

extern void initialize_images(void);

--EOF

	#
	# C++/WX file
	#
	open(CPP, "> $cpp_file") || warn "Could not open $cpp_file: $!\n";
	print CPP "/* Autogenerated by png2wx.pl on ",
	          scalar(localtime()), " */\n";
	print CPP <<"--EOF";
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif
#include <wx/mstream.h>
#include "$hpp_include"

--EOF

	#
	# Process files from the command line
	#
	foreach my $file (@ARGV) {
		my $base = $file;
		$base =~ s{^.*/}{};
		$base =~ s{(.*)\.(?:jpg|png)$}{$1}gis;
		$base =~ s/[^a-z]/_/gio;

		print HPP "extern wxBitmap *_img_$base;\n";
		print CPP "wxBitmap *_img_$base;\n";

		$main .= "	{\n".
		         "		wxMemoryInputStream sm(\"".&encoded($file)."\", ".(-s $file).");\n".
		         "		_img_$base = new wxBitmap(wxImage(sm));\n".
		         "	}\n";
	}

	#
	# Fixup C++ header
	#
	print HPP "\n", "#endif /* $Marker */\n";
	close HPP;

	print CPP
		"\n",
		"void initialize_images(void) {\n",
		$main,
		"	return;\n",
		"}\n";
	close CPP;
	return;
}

sub encoded {
	my $file = shift @_;
	my $data;
	local *FH;

	if (!open(FH, "< $file")) {
		warn "Could not open $file: $!\n";
		return;
	}

	binmode FH;
	$data = join(undef, <FH>);
	$data =~ s/\\/\\\\/go;
	$data =~ s/([^\x21\x23-\x7e])/sprintf "\\%03o", ord $1/egs;
	$data =~ s/\?\?(?=[-\(\)<>=\/'!])/?\\077/g;
	close FH;
	return $data;
}
