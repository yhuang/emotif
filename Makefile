# Generated automatically from Makefile.in by configure.
PACKAGE		= eMOTIF-3.6


#
# ----------
# The use of the standard Bourne shell maximizes protability and offers
# adequate programming constructs.
# ----------
#

SHELL		= /bin/sh

prefix		= /usr/local
exec_prefix	= ${prefix}
top_srcdir      = .
srcdir		= .
testdir         = ./test
bindir       	= ${exec_prefix}/bin
datadir		= ${prefix}/share

CC              = gcc $(PROFILING)
CCLINKER        = gcc $(PROFILING)
CFLAGS          = -g -O2
CPPFLAGS        =  -DSTDC_HEADERS=1 

CXX 		= c++ $(PROFILING)
CXXLINKER	= c++ $(PROFILING)
CXXFLAGS	= -g -O2
CXXCPPFLAGS	=  -DSTDC_HEADERS=1 

PROFILING	=
LDFLAGS		= 
LOCALDEFS       = -DDATA_DIR=\"/usr/share/emotif\" \
                  -DGROUPS_FILE=\"default\"


INSTALL		= /usr/bin/install -c
INSTALL_PROGRAM	= ${INSTALL}
INSTALL_DATA	= ${INSTALL} -m 644


#
# ----------
# The first .SUFFIXES: deletes all currently recognized suffixes.  The
# second .SUFFIXES: replaces the curent suffixes with .txt, .cc, .c, and
# .o.
# ----------
#

.SUFFIXES:
.SUFFIXES: .cc .c .o


.cc.o:
	$(CXX) $(CXXFLAGS) $(LOCALDEFS) -o $*.o -c $*.cc


#
# The configure program decides what CFLAGS should be for each Unix
# machine.  On Tom's Silicon Graphics machine, CFLAGS is set to
# -g -O2
#

.c.o:
	$(CC) $(CFLAGS) $(LOCALDEFS) -o $*.o -c $*.c


########################################################################
##                            Variables                               ##
########################################################################

RAW_BLOCKS_INPUT      = ../einputs/blocks/blocks.dat
RAW_PRINTS_INPUT      = ../einputs/prints/prints31_0.dat
SPROT_INPUT           = 
SEQUENCE_INPUT        = ../einputs/seqs/sprot.fas

DATADIR_EMOTIF        = /usr/share/emotif
DATA_SUBDIR_EMOTIFS   = emotifs
DATA_SUBDIR_SEQUENCES = sequences
HTMLDIR               = /var/www/html
HTMLSUBDIR            = emotif
CGIDIR                = /var/www/cgi-bin
CGISUBDIR             = emotif

AVAIL_GROUPS          = ./share/default ./share/HSSP+beta

HTDOCS_CONFIG         = ./html/*.html ./html/menu
CGIBIN_CONFIG         = ./cgi-bin/cgi-lib.pl ./cgi-bin/nph-emotif-maker \
                        ./cgi-bin/nph-emotif-scan ./cgi-bin/nph-emotif-search \
                        ./cgi-bin/nph-make-page ./cgi-bin/nph-reformat \
                        ./cgi-bin/nph-top-scores


########################################################################

all: build bin seqfiles alignments emotiffiles binaryfiles


########################################################################

seqfiles: $(SEQUENCE_INPUT)
	@echo Be patient: This next step takes a while ...

	for i in $(SEQUENCE_INPUT); do \
	    fileroot=`echo $$i | sed -e 's/^.*\///' -e 's/^\(.*\)\..*/\1/'`; \
	    if test ! -r ./sequences/$$fileroot.scan ; then \
	        cat $$i | ./build/reformat-fasta.pl > ./sequences/$$fileroot.scan ; \
	    fi \
	done;


########################################################################

BUILD_PROGRAMS = ./bin/emotif-maker \
                 ./build/fwrite-emotifs ./build/fwrite-sequences

LIBRARY_OBJS =  ./src/scanner.o ./src/darray.o ./src/emotif.o


build: $(BUILD_PROGRAMS) perl 

./bin/emotif-maker: ./src/emotif-maker.o ./src/dejavu.h
	$(CXX) $(CXXFLAGS) -o $@ ./src/emotif-maker.o $(LDFLAGS) -lm

./build/fwrite-emotifs: ./src/fwrite-emotifs.c $(LIBRARY_OBJS)
	$(CC) $(CFLAGS) -o $@ ./src/fwrite-emotifs.c -lm $(LIBRARY_OBJS)

./build/fwrite-sequences: ./src/fwrite-sequences.c $(LIBRARY_OBJS)
	$(CC) $(CFLAGS) -o $@ ./src/fwrite-sequences.c -lm $(LIBRARY_OBJS)

perl:
	for i in ./build/*.pl; do \
	    chmod a+x $$i; \
	done;


########################################################################

BIN_PROGRAMS = ./bin/gradient ./bin/myppmchange \
               ./bin/emotif-search ./bin/motifgrep ./bin/list-non-biased-emotifs

LIBRARY_OBJS =  ./src/scanner.o ./src/darray.o ./src/emotif.o

bin: $(BIN_PROGRAMS)

./bin/gradient: ./src/gradient.o
	$(CXX) $(CXXFLAGS) -o $@ ./src/gradient.o $(LDFLAGS)

./bin/myppmchange: ./src/myppmchange.o
	$(CC) $(CFLAGS) -o $@ ./src/myppmchange.o $(LDFLAGS)

./bin/motifgrep: ./src/motifgrep.o
	$(CXX) $(CXXFLAGS) -o $@ ./src/motifgrep.o $(LDFLAGS)

./bin/emotif-search: ./src/emotif-search.c $(LIBRARY_OBJS)
	$(CC) $(CFLAGS) -o $@ ./src/emotif-search.c -lm $(LIBRARY_OBJS)

./bin/list-non-biased-emotifs: ./src/list-non-biased-emotifs.o ./src/scanner.o
	$(CC) $(CFLAGS) -o $@ ./src/list-non-biased-emotifs.o -lm ./src/scanner.o


########################################################################

PRINTSLESS_BLOCKS = ../einputs/blocks/printsless-blocks.dat
PRINTS_DAT = ../einputs/prints/prints.dat
PROCESSED_BLOCKS = ../einputs/blocks/processed-blocks.dat
PROCESSED_PRINTS = ../einputs/prints/processed-prints.dat


alignments: $(PROCESSED_BLOCKS) $(PROCESSED_PRINTS)

$(PROCESSED_BLOCKS): $(RAW_BLOCKS_INPUT)
	cat $(RAW_BLOCKS_INPUT) | ./build/remove-prints.pl > $(PRINTSLESS_BLOCKS)
	./build/eliminate-shoddy-sequences.pl $(PRINTSLESS_BLOCKS) > $@

$(PROCESSED_PRINTS): $(RAW_PRINTS_INPUT)
	cat $(RAW_PRINTS_INPUT) | ./build/prints-to-blocks.pl > $(PRINTS_DAT)
	./build/eliminate-shoddy-sequences.pl $(PRINTS_DAT) > $@


########################################################################

STRINGENCY_LIST = 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22
TMP_EMOTIFS_DATA = ./tmp/emotifs.0 ./tmp/emotifs.1 ./tmp/emotifs.2 \
                   ./tmp/emotifs.3 ./tmp/emotifs.4 ./tmp/emotifs.5 \
                   ./tmp/emotifs.6 ./tmp/emotifs.7 ./tmp/emotifs.8 \
                   ./tmp/emotifs.9 ./tmp/emotifs.10 ./tmp/emotifs.11 \
                   ./tmp/emotifs.12 ./tmp/emotifs.13 ./tmp/emotifs.14 \
                   ./tmp/emotifs.15 ./tmp/emotifs.16 ./tmp/emotifs.17 \
                   ./tmp/emotifs.18 ./tmp/emotifs.19 ./tmp/emotifs.20 \
                   ./tmp/emotifs.21 ./tmp/emotifs.22
EMOTIFS_TMP = ./emotifs/emotifs.tmp
EMOTIFS_UNIQ = ./emotifs/emotifs.uniq
EMOTIFS_BLOCKS_TXT = ./emotifs/emotifs-blocks.txt
EMOTIFS_PRINTS_TXT = ./emotifs/emotifs-prints.txt
EMOTIFS_BLOCKSPLUS_TXT = ./emotifs/emotifs-blocksplus.txt


emotiffiles: $(EMOTIFS_BLOCKSPLUS_TXT)

$(EMOTIFS_BLOCKS_TXT): $(PROCESSED_BLOCKS)
	for stringency in $(STRINGENCY_LIST); do \
	    if [ ! -f ./tmp/emotifs.$$stringency ]; then \
	        cat $(PROCESSED_BLOCKS) | ./build/emotif-build.pl -a ./share/default -s $$stringency > ./tmp/emotifs.$$stringency.tmp ; \
	        mv ./tmp/emotifs.$$stringency.tmp ./tmp/emotifs.$$stringency ; \
	    fi \
	done;

	cat $(TMP_EMOTIFS_DATA) > $(EMOTIFS_TMP)

	sort -n $(EMOTIFS_TMP) | uniq > $(EMOTIFS_UNIQ)

	./build/process-raw-emotifs.pl $(EMOTIFS_UNIQ) $(PROCESSED_BLOCKS) > $(EMOTIFS_TMP)

	sort -n $(EMOTIFS_TMP) > $@

	for i in ./tmp/*; do \
	    /bin/rm -f $$i; \
	done;

	/bin/rm -f $(EMOTIFS_UNIQ)
	/bin/rm -f $(EMOTIFS_TMP)

$(EMOTIFS_PRINTS_TXT): $(PROCESSED_PRINTS)
	for stringency in $(STRINGENCY_LIST); do \
	    if [ ! -f ./tmp/emotifs.$$stringency ]; then \
	        cat $(PROCESSED_PRINTS) | ./build/emotif-build.pl -a ./share/default -s $$stringency > ./tmp/emotifs.$$stringency.tmp ; \
	        mv ./tmp/emotifs.$$stringency.tmp ./tmp/emotifs.$$stringency ; \
	    fi \
	done;

	cat $(TMP_EMOTIFS_DATA) > $(EMOTIFS_TMP)

	sort -n $(EMOTIFS_TMP) | uniq > $(EMOTIFS_UNIQ)

	./build/process-raw-emotifs.pl $(EMOTIFS_UNIQ) $(PROCESSED_PRINTS) > $(EMOTIFS_TMP)

	sort -n $(EMOTIFS_TMP) > $@

	for i in ./tmp/*; do \
	    /bin/rm -f $$i; \
	done;

	/bin/rm -f $(EMOTIFS_UNIQ)
	/bin/rm -f $(EMOTIFS_TMP)

$(EMOTIFS_BLOCKSPLUS_TXT): $(EMOTIFS_BLOCKS_TXT) $(EMOTIFS_PRINTS_TXT)
	cat $(EMOTIFS_BLOCKS_TXT) $(EMOTIFS_PRINTS_TXT) > $(EMOTIFS_TMP)
	sort -n $(EMOTIFS_TMP) | uniq > $@	
	/bin/rm -f $(EMOTIFS_TMP)


########################################################################

TRIMMED_SUBBLOCKS = ./tmp/trimmed-subblocks.dat
EMOTIFS_NON_BIASED_BLOCKSPLUS_TXT = ./emotifs/emotifs-non-biased-blocksplus.txt

$(TRIMMED_SUBBLOCKS): $(PROCESSED_BLOCKS) $(PROCESSED_PRINTS)
	cat $(PROCESSED_BLOCKS) $(PROCESSED_PRINTS) | ./build/enumerate-all-subblocks.pl $(EMOTIFS_BLOCKSPLUS_TXT) > $@

$(EMOTIFS_NON_BIASED_BLOCKSPLUS_TXT): $(TRIMMED_SUBBLOCKS)
	cat $(TRIMMED_SUBBLOCKS) | ./bin/list-non-biased-emotifs > $(EMOTIFS_TMP)
	sort -n $(EMOTIFS_TMP) > $@
	/bin/rm -f $(EMOTIFS_TMP)

binaryfiles: $(EMOTIFS_NON_BIASED_BLOCKSPLUS_TXT) $(EMOTIFS_BLOCKSPLUS_TXT)
	cat $(EMOTIFS_NON_BIASED_BLOCKSPLUS_TXT) | ./build/fwrite-emotifs -d non-biased-blocksplus
	cat $(EMOTIFS_BLOCKSPLUS_TXT) | ./build/fwrite-emotifs -d blocksplus


########################################################################

install: installdirs installprogs web installdata

installdirs:
	$(SHELL) ${srcdir}/mkinstalldirs $(DATADIR_EMOTIF)
	$(SHELL) ${srcdir}/mkinstalldirs $(DATADIR_EMOTIF)/$(DATA_SUBDIR_EMOTIFS)
	$(SHELL) ${srcdir}/mkinstalldirs $(DATADIR_EMOTIF)/$(DATA_SUBDIR_SEQUENCES)
	$(SHELL) ${srcdir}/mkinstalldirs $(CGIDIR)/$(CGISUBDIR)
	$(SHELL) ${srcdir}/mkinstalldirs $(HTMLDIR)/$(HTMLSUBDIR)
	$(SHELL) ${srcdir}/mkinstalldirs $(HTMLDIR)/$(HTMLSUBDIR)/motifs
	chown nobody $(HTMLDIR)/$(HTMLSUBDIR)/motifs
	chgrp nobody $(HTMLDIR)/$(HTMLSUBDIR)/motifs

installprogs:
	for i in $(BIN_PROGRAMS); do \
	    root=`echo $$i | sed -e 's/^.*\///'`; \
	    $(INSTALL_PROGRAM) $$i $(bindir)/$$root; \
	done;

	$(INSTALL_PROGRAM) ./bin/emotif-maker $(bindir)/emotif-maker;

	for i in ./bin/*.pl; do \
	    root=`echo $$i | sed -e 's/^.*\///'`; \
	    $(INSTALL_PROGRAM) $$i $(bindir)/$$root; \
	done;

installdata:
	for i in ./emotifs/*.bin ./emotifs/*-stats; do \
	    root=`echo $$i | sed -e 's/^.*\///'`; \
	    $(INSTALL_DATA) $$i $(DATADIR_EMOTIF)/$(DATA_SUBDIR_EMOTIFS)/$$root; \
	done;

	for i in $(AVAIL_GROUPS); do \
	    root=`echo $$i | sed -e 's/^.*\///'`; \
	    $(INSTALL_DATA) $$i $(DATADIR_EMOTIF)/$$root; \
	done;

	for i in ./sequences/*.scan; do \
	    root=`echo $$i | sed -e 's/^.*\///'`; \
	    $(INSTALL_DATA) $$i $(DATADIR_EMOTIF)/$(DATA_SUBDIR_SEQUENCES)/$$root; \
	done;

web:
	for i in $(HTDOCS_CONFIG); do \
	    root=`echo $$i | sed -e 's/^.*\///'`; \
	    $(INSTALL_DATA) $$i $(HTMLDIR)/$(HTMLSUBDIR)/$$root; \
	done;

	for i in ./html/*.gif ./html/*.txt ./html/*.pdf; do \
	    root=`echo $$i | sed -e 's/^.*\///'`; \
	    $(INSTALL_DATA) $$i $(HTMLDIR)/$(HTMLSUBDIR)/$$root; \
	done;

	for i in $(CGIBIN_CONFIG); do \
	    root=`echo $$i | sed -e 's/^.*\///'`; \
	    $(INSTALL_PROGRAM) $$i $(CGIDIR)/$(CGISUBDIR)/$$root; \
	done;


check:
	@echo Testing emotif-maker ... 
	$(bindir)/emotif-maker -a ./share/default < $(testdir)/test-block
	@echo Testing emotif-search  ...
	@echo ... finished testing


uninstall:
	/bin/rm -fr $(CGIDIR)/$(CGISUBDIR)/
	/bin/rm -fr $(HTMLDIR)/$(HTMLSUBDIR)/
	/bin/rm -fr $(DATADIR_EMOTIF)/
	/bin/rm -f $(bindir)/emotif-maker
	/bin/rm -f $(bindir)/emotif-scan
	/bin/rm -f $(bindir)/emotif-search
	/bin/rm -f $(bindir)/gap.pl
	/bin/rm -f $(bindir)/gradient
	/bin/rm -f $(bindir)/motifgrep
	/bin/rm -f $(bindir)/myppmchange
	/bin/rm -f $(bindir)/plot.pl


clean:
	/bin/rm -f core
	/bin/rm -f $(BIN_PROGRAMS)
	/bin/rm -f ./bin/*.pl
	/bin/rm -f $(BUILD_PROGRAMS)
	/bin/rm -f ./build/*.pl ./build/*.o
	/bin/rm -f $(CGIBIN_CONFIG)
	/bin/rm -f $(HTDOCS_CONFIG)
	/bin/rm -f ./src/*.o

distclean: clean
	/bin/rm -f ./src/*.c ./src/*.cc ./src/*.h
	/bin/rm -f *~ */*~ Makefile config.*
	/bin/rm -f ./sequences/*.scan
	/bin/rm -f ./emotifs/*.bin ./emotifs/*-stats
	/bin/rm -f LICENSE README

