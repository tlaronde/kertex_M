#!/bin/sh
# tangling tangle with text processing tools.
#
# $Id: tangleboot.sh,v 1.8 2009/12/27 13:56:08 tlaronde Exp $
# C) 2009 Thierry Laronde <tlaronde@polynum.com>
#	All rights reserved ; no guarantees!
#

: ${TMPDIR:=/tmp}
tmp=$TMPDIR/$$.boot
test $# -eq 1 || { echo "usage: $0 tangle.web" >&2; Exit 1; }

# There is the chicken and egg problem: to compile tangle(1), you need
# tangle.
# But, as expressed by Donald E. Knuth in the documentation, since the
# process is relatively simple (at least for a file like tangle.web),
# one can do it "by hand". But since we have text processing tools, what
# can be done, repetitively by hand, can be made with these tools.
#
# It is for bootstrapping and for a particular file that, once
# converted, will give access to the full specification and power of
# tangle(1). There are two obvious limits:
#
#	- no verifications, no checks and no help; for a developing tool, it
#	would be a fault;
#	- rough: nested comments are not taken into account etc. Only basic
#	use will do;
#	- not efficient at all:
#
# tangle          0.06s user  0.01s system  26% cpu  0.255 total
# boot/tangle.sh 16.17s user 17.13s system 111% cpu 29.754 total
#
# But at least in theory, it could be tested with other WEB files as
# well---for example to increase its robustness.
#
prog=$(basename $1)
prog=${prog%.web}
test ! -f $prog.p \
	|| { echo "$prog.p exists! Move one of us out of the path!"; exit 1; }

# If debugging, the trace will contain enough information. Just set in
# the environment the DEBUG variable to YES.
#
test "x$DEBUG" != xYES || set -x

printf "\n\tTangling tangle by hand (almost...). May take a minute...\n\n" >&2

# Be sure files are empty because no files...
#
rm -f $tmp.*

# What is the process? There is a top level chunk, introduced by "@p".
# We need to gather, in sequence, all the chunks tagged by "@p" in the 
# same file that will be called: $prog.p. In this file,
# all the "@<some identifier@>" strings shall be replaced by all the
# chunks tagged "@<some id...@>=".
#
# What we see is that chunks can be, in a first step, considered as
# files. The tricky point is that the identifiers can be ellipsed that
# is appear in the file as prefix of various sizes.
#
# So for all the chunks, including the main introduced by "@p", we will
# gather the text in a file belonging to this identifier, registering
# the identifiers in the file $tmp.id, in two fields: 
#
#	identifier filename
#
# and when an identifier is found, we simply search if it matches
# another key or if another key is a prefix to this identifier, since
# we must ensure uniqueness and we must retain the _shortest_ prefix,
# that is guaranteed to uniquely identify the full name---it's a 
# request for tangle(1).
# The simplest way to create new names for distinct sections is to use
# as a suffix the cardinal of the identifiers found till now.
#
nid=0
cat /dev/null >$prog.p

# There is another type of information: the defines. Defines are a form
# of substitution. So we will gather the defines in the file $tmp.def 
# and, when the whole program file has been "explicated" (expanded), 
# we will apply these substitutions to the whole file.
#

# Since there are numerous commands that are of no use for tangle---the
# formatting commands, index, debug, stat etc.--- and since we will have
# the obligation to remove them to obtain the source, we do this at the
# beginning creating a prune version of the source to work with.
# This is done by the sed script embedded in the end of this file, after
# the exit so that it is not processed by the shell but stay with us.
#
# Another action is to format the file so that retrieving the
# identifiers is simpler: we put the beginning tag at the beginning of a
# line, and the end tag at the end... of _a_ line, since long
# identifiers can span multiple lines. So we will have to _join_ in 
# order to have an identifier alone, the whole on one line. This is 
# done by the second sed script.
#
# The other tags like '^@$' are also rewritten appending a `_' in order
# to test in one expression for two characters starting by `@'.
#
# Look at the scripts embedded at the end of this file, they have
# comments. We extract all of them at once.
#
echo "Step 0: formatting input to ease scanning." >&2
junk="$(command -v $0)"
ed -s "$junk" <<EOT
/^BEGINSED1/+,/^ENDSED1/-w $tmp.prune
/^BEGINSED2/+,/^ENDSED2/-w $tmp.join
q
EOT

# Since some shell implementation have problems read'ing from a pipe,
# we write to a tmp file we cat afterwards.
#
sed -f $tmp.prune $1  \
	| sed -f $tmp.join >$tmp.tmp

cp $tmp.tmp $tmp.step0

# When using the shortest prefix as the regexp for the insertion of the
# file we need to escape the meta-characters for ed.
#
# The escaping club...
#
escape1 ()
{
	echo "$@" | sed -e 's!/!\\/!g' -e 's!\.!\\.!g' -e 's!\*!\\*!g'
}

# And we need to store the records so that the separator will not be
# some string likely to be found in the identifiers.
#
# `@' is forbidden.
#
sep="%%%%"

# The problem with the identifiers is that they can appear with a
# variable string as long as the prefix is not ambiguous. 
# And they do not appear ellipsed in the definition (@>=) but also in
# the declaration/insertion. Hence, when we detect an identifier name,
# even if it is not a definition (a supplementary chunk), we need to
# verify if this is not a shortest prefix!
#
# But we can also add the declaration/insertion, since nothing will be
# added in the file if the state is not the correct one, but at least
# the suffix number will be set.
#
# an ellipsed is shorter; hence this is the shortest one that we must
# keep in order to use this shortest as a regexp that will match every
# variation when inserting the corresponding file chunk.
#
# But the name can appear shorter before longer... Hence we need to
# look for the reverse too.
#
# The routine removes any ellipse from the identifier and search
# existing pairs, so that if current matches a stored, or a stored
# matches the current, the shortest prefix is kept (but fname is
# unchanged. If nothing, the fname takes the current nid that is 
# incremented afterwards.
#
# There is an obvious mean to hash: take the first letter as the file
# gathering all the identifiers starting by the very same letter. Since
# they are all prefix, at least one letter is here...
#
# i: an int for loop
# f: the fname
# h: the hash i.e. the first letter
# k: a key (registered prefix)
# l: the line
# p: the regexp pattern
#
lookup ()
{
	local i f k l p

	# remove ellipse if any
	id="${id%...}"

	# the hash is the first letter
	h=$(expr "$id" : '\(.\)')
	touch "$tmp.id.$h"

	# look if we match something existing. 
	i=0
	p="$(escape1 "$id")"
	while read l; do
		i=$(($i + 1))
		k="$(echo $l | sed "s@$sep@	@" | cut -f1)"
		if expr "$k" : "$p" >/dev/null; then
			fname=$(echo $l | sed "s@$sep@	@" | cut -f2)
			ed -s $tmp.id.$h <<EOT
${i}d
a
$id$sep$fname
.
w
q
EOT
		return
		fi
	done <$tmp.id.$h

	# Perhaps existing key is shorter.
	#
	while read l; do
		k="$(echo $l | sed "s@$sep@	@" | cut -f1)"
		p="$(escape1 "$k")"
		if expr "$id" : "$p" >/dev/null; then
			fname=$(echo $l | sed "s@$sep@	@" | cut -f2)
			return
		fi
	done <$tmp.id.$h
	# So it is a new entry.
	# Set fname, empty it, adjust nid and add record.
	#
	fname=$tmp.$nid
	cat /dev/null >$fname
	nid=$(($nid + 1))
	echo "$id$sep$fname" >>$tmp.id.$h
}


# We need to know if we are in a chunk or not, since if we are we need
# to add the line to the current file. If not, we look if we found
# another identifier, a define or if we can simply drop the line.
#
id=
inchunk=NO
fname=	# the current file to add to if inchunk is YES

# We don't know a priori how many files we need to open. So we will
# simply use dumbly the shell and read line by line instead of using
# for example awk(1).
#
# We have done some formatting to simplify scanning. Here too we make
# the basic optimization to test for /^@/ since all the tags we are
# interested in share this expression.
#
echo "Step 1: extracting and gathering the related chunks; please wait..." >&2
while read line; do
	if expr "$line" : '@' >/dev/null; then
		case "$line" in
			@|@ |@"*") inchunk=NO;
				fname=;;
			@p) fname=$prog.p;
				inchunk=YES;;
			@d*) echo "${line#@d }" >>$tmp.def;;
			@'<'*@'>'=) id=$(expr "$line" : '@<\(.*\)@>=$');
				lookup;
				inchunk=YES;;
			@'<'*@'>') if test $inchunk = YES ; then
					id=$(expr "$line" : '@<\(.*\)@>$');
					junk=$fname; # save since lookup will set
					lookup;	# will set fname so we need to reset
					fname=$junk; # reset
					echo "$line" >>$fname; # the tag must be in the chunk!
					fi;;
			*) test $inchunk = NO || echo "$line" >>$fname;;
		esac
	else
		test $inchunk = NO || echo "$line" >>$fname
	fi
done <$tmp.tmp

# We will know recursively replace the occurrences of "@<some string@>"
# by inserting the content of the corresponding file.
#
# Since the lookup() has---normally...---put the shortest prefix as the
# uniq key for an identifier the sed(1) replacing script is simple.
#
# Note: a line deleted is defunct and nothing more is done to it, hence
# the script passes to the end of the script jumping the next
# instruction. So we insert first, and delete after.
#
echo "Step 2: replacing the named section in the main chunk" >&2
cat $tmp.id.* >$tmp.id
while read line; do
	id="$(echo $line | sed "s@$sep@	@" | cut -f1)"
	id="$(escape1 "$id")"
	fname=$(echo $line | sed "s@$sep@	@" | cut -f2)
	cat - <<EOT >>$tmp.sed
/^@<$id.*@>\$/ {
	r $fname
	d
}
EOT
done <$tmp.id

# We need to do this recursively, since in the text inserted may appear
# identifiers that are not replaced by the sequence of commands.
#
# Since we may introduced other identifiers not beginning a line or
# spanning multiple lines, we need to redo the normalisation.
#
while grep -q '^@<' $prog.p; do
	sed -f $tmp.sed $prog.p >$tmp.tmp
	mv $tmp.tmp $prog.p
done

# Change the special formats: @@, octals and hexadecimals, and finally 
# do what C makes as pre-processing... the def substitutions.
#
echo "Step 3: "pre-processing" the resulting source (defines)" >&2

printf "\n\t\t\t\tDONE\n\n" >&2

test "x$DEBUG" = xYES || rm $tmp.*

exit 0

# This part is not proceed by the shell and has our data.
#
# Pruning comment, formating, index and hints, and putting are control
# strings so that /@[p>]/ is at the beginning of the line, and the
# end /@>=?/ at the end... of a line. Tags spanned accross multiple
# lines will be join with SED2.
#
# There are formatting sequences that can wreak havoc when compiling:
# \&{ } have comment delimiters that we need to delete.
#
# We treat the sequences not containing '@' first, and then we test if
# a line contains the character or not, since all the remaining commands
# depend on that; if not we branch, i.e. we pass directly to another 
# line. This is a small optimization.
#
# Since we don't care, for compilation, about blanks we replace a
# sequence suppressed by a space in order to keep token delimiters right
# (not catenating by inadvertance words that should be left alone).
# Except for "@&" where it must be a no space in the output.
#
BEGINSED1
s/\\&{\([^}]*\)}/\1/g
/@/!b
s/\([^@]\)@&/\1/g
s/@{\([^}]*\)@}/{\1}/g
s/@t[^@]*@>/ /g
s/@\.[^@]*@>/ /g
s/@^[^@]*@>/ /g
s/^\(@[*p ]\)/\1\
/
s/^@<[^<]/\
&/g
s/\([^@]\)\(@<[^<]\)/\1\
\2/g
s/\([^@]@>\)\([^=]\)/\1\
\2/g
s/[^@]@>=/&\
/
s!^@/! !g
s!\([^@]\)@/!\1 !g
s/^@!/ /g
s/\([^@]\)@!/\1 /g
s/^@+/ /g
s/\([^@]\)@+/\1 /g
s/^@;/ /g
s/\([^@]\)@;/\1 /g
ENDSED1

# Joining the identifiers spanned on multiple lines.
#
BEGINSED2
/^@</ {
:more
	/@>/b
	N
	s/\n//
	b more
}
ENDSED2
