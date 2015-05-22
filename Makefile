# Makefile for DOE proposals
#
# Basic usage:
# 1) Generate a onfiguration file for subsequent builds
#     make [participant] [content] [version]
#     Defaults: <lead institution> <lead institution's content type> draft
# where the values for the targets are listed below.  This
# configuration will persist until you reconfigure.
#
# Note that nameing a participant implicitly sets the type of content
# (to lab or grants depending on the type of institution).  If you
# want to override this, the content target must appear AFTER the
# participant, as shown.
#
# 2) Build proposal.pdf with the existing configuration
#     make [proposal|proposal.pdf|reuse]
#
# 3) Reconfigure and build a separate abstract.pdf page (for grants.gov)
#     make abstract
#
# Additional targets
#     deconfigure -- remove configure.tex file (document should build
#     		     based on internal defaults)
#     default -- reconfigure to nobody technical draft
#     clean -- remove uninteresting LaTeX intermediate files
#     realclean -- clean and also remove all PDF files
#     svnignore -- update svn property from .svnignore file

# Participating institutions grouped by how they submit their proposal
# 'nobody' is meant as a default
PARTICIPANTS_LAB = ornl
PARTICIPANTS_GRANTS = nobody gatech oregon
PARTICIPANTS = $(PARTICIPANTS_LAB) $(PARTICIPANTS_GRANTS)

# Lead institution may need to include some additional content
LEAD = gatech

CONTENTS = lab
VERSIONS = draft final

###############################################################################
# Should not need to modify below this point
###############################################################################
# Note; if suffix.sty complains about needing eTeX v2, you may need
# to redefine this to 'pdfelatex'.  This can happen on older TeX installations
# Please override variable on command line, not in Makefile!
LATEX = pdflatex

###############################################################################
# Automatic File dependencies 
#
# Organize by directories and wildcard to simplify our life.  This is
# conservative in so far as it is possible not all of the files are
# actually used directly in the document.
###############################################################################

NARRATIVE = ${wildcard narrative/*.tex}
GRAPHICS = ${wildcard graphics/*}
BIBS = ${wildcard bib/*.bib}

# These are based on the listed participant
COVER = ${wildcard covers/$(PARTICIPANT).*}
FWP = ${wildcard fwps/$(PARTICIPANT).*}
FACE = ${wildcard faces/$(PARTICIPANT).*}

# Just use a wildcard on these because people are adding at random
BIOS_TEX = bios.tex $(wildcard bios/*.tex)
BIOS_PDF = $(wildcard bios/*.pdf)

SUPPORT_TEX = other-support.tex $(wildcard other-support/*.tex)
SUPPORT_PDF = $(wildcard other-support/*.pdf)

FACILITIES_TEX = facilities.tex $(wildcard facilities/*.tex)
FACILITIES_PDF = $(wildcard facilities/*.pdf)

BUDGETS_TEX = $(wildcard budgets/*.tex)
BUDGETS_PDF = $(wildcard budgets/*.pdf)

SOWS_TEX = $(wildcard sows.tex) $(wildcard sows/*.tex)
SOWS_PDF = $(wildcard sows/*.pdf)

APPENDICES_TEX = $(wildcard appendices.tex) $(wildcard appendices/*.tex) \
	$(wildcard collaborations.tex) $(wildcard collaborations/*.tex)
APPENDICES_PDF = $(wildcard appendices/*.pdf) \
	$(wildcard letters/*.pdf)

OPTIONAL_TEX = $(wildcard measure.tex) \
	$(wildcard equipment.tex) \
	$(wildcard revised-sow.tex)

# Main proposal TeX files
PROPOSAL_TEX = \
	$(BIOS_TEX) $(BIOS_PDF) \
	$(BUDGETS_TEX) $(BUDGETS_PDF) \
	$(FACILITIES_TEX) $(FACILITIES_PDF) \
	$(SUPPORT_TEX) $(SUPPORT_PDF) \
	$(SOWS_TEX) $(SOWS_PDF) \
	$(APPENDICES_TEX) $(APPENDICES_PDF) \
	$(OPTIONAL_TEX) \
	$(COVER) $(FACE) $(FWP) \
	titlepage.tex \
	summary-budget.tex \
	abstract.tex \
	acronym.tex \
	output-controls.tex \
	preamble.tex \
	proposal.tex \
	$(NARRATIVE)

.PHONY: proposal reuse reconfigure generic abstract\
	$(PARTICIPANTS) $(CONTENTS) $(VERSIONS) \
	$(BIOS_PDF) $(BUDGETS_PDF) $(FACILITIES_PDF) $(SUPPORT_PDF) \
	$(SOWS_PDF) $(APPENDICES_PDF) \
	autogen autogen-institutions autogen-personnel \
	check quality missingincludes badrefs \
	clean realclean svnignore 

###############################################################################
# Primary targets
###############################################################################
proposal reuse:		proposal.pdf

###############################################################################
# Configuration targets
###############################################################################
default:	$(LEAD) draft

# Remove configuration file
deconfigure: 
	$(RM) configure.tex

# Reset configuration file
reconfigure:	deconfigure
	/bin/echo "% Generated by Makefile `date`" >> configure.tex
	/bin/echo "\renewcommand{\proposallead}{$(LEAD)}" >> configure.tex

# Add participant info to configuration file
$(PARTICIPANTS)::	reconfigure
	/bin/echo "\renewcommand{\proposalinstitution}{$@}" >> configure.tex

$(PARTICIPANTS_GRANTS)::	grants
$(PARTICIPANTS_LAB)::		lab

# Add content selection to configuration file
$(CONTENTS)::	reconfigure
	/bin/echo "\renewcommand{\proposalcontent}{$@}" >> configure.tex

abstract::	abstract.pdf

# Add version selection to configuration file
$(VERSIONS)::	reconfigure
	/bin/echo "\renewcommand{\proposalversion}{$@}" >> configure.tex

###############################################################################
# Autogeneration of files based on institution and personnel lists
###############################################################################
# There are a bunch of places where fundamentally the same information
# about the institutions and personnel involved in the proposal
# appears in the proposal.  This targets and the underlying scripts
# automate their generation from compact configuration files.

autogen: autogen-institutions autogen-personnel autogen-leadpi-slug \
	 autogen-participant-table

# Currently generates: acronym-inst.tex budgets.tex facilities.tex sows.tex
autogen-institutions: institutions.config
	bin/config-institutions $(LEAD)

# Currently generates: bios.tex other-support.tex acronym-pis.tex
autogen-personnel: personnel.config
	bin/config-personnel

# Currently generates: leadpi-slug.tex
autogen-leadpi-slug: personnel.config
	bin/config-leadpi-slug

# Currently generates: participant-table.tex
autogen-participant-table: personnel.config
	bin/config-participant-table $(LEAD)

###############################################################################
# Quality assurance checks after compiling the proposal document
###############################################################################
check quality: missingincludes badrefs badbibs

# These dependencies end up forcing a complete rebuild every time you
# 'make check'
# proposal.log proposal.blg: proposal.pdf

# Extract a list of missing smartinclude files
missingincludes: proposal.log
	@(grep -i "smartinclude: warning:" $< | \
	  sed 's/smartinclude: WARNING: //' > make.tmp.$$$$ ;\
	if [ -s make.tmp.$$$$ ]; then \
	  echo -e "\n===== SMARTINCLUDE WARNINGS =====" ; \
	  cat make.tmp.$$$$; \
	fi; \
	$(RM) make.tmp.$$$$)

# Check log for: 
# - Multiply defined labels
# - Undefined labels
# - Undefined citations
# - Undefined acronyms (no messages for multiply-defined acronyms)
badrefs: proposal.log
	@(grep -i "latex warning:" $< |\
	  grep -v -i "there were multiply-defined labels" | \
	  grep "multiply defined" |\
	  sed 's/LaTeX Warning: //' > make.tmp.$$$$ ;\
	if [ -s make.tmp.$$$$ ]; then \
	  echo -e "\n===== MULTIPLY-DEFINED LABELS =====" ; \
	  cat make.tmp.$$$$; \
	fi; \
	$(RM) make.tmp.$$$$)
	@(grep -i "latex warning:" $< | \
	  grep -v -i "there were undefined references" | \
	  grep -i "reference" | grep -i "undefined" |\
	  sed 's/LaTeX Warning: //' > make.tmp.$$$$ ;\
	if [ -s make.tmp.$$$$ ]; then \
	  echo -e "\n===== UNDEFINED LABELS =====" ; \
	  cat make.tmp.$$$$; \
	fi; \
	$(RM) make.tmp.$$$$)
	@(grep -i "package acronym warning:" $< | \
	  grep -i "acronym" | grep -i "not defined" |\
	  sed 's/Package acronym Warning: //' |\
	  sort | uniq > make.tmp.$$$$ ;\
	if [ -s make.tmp.$$$$ ]; then \
	  echo -e "\n===== UNDEFINED ACRONYMS =====" ; \
	  cat make.tmp.$$$$; \
	fi; \
	$(RM) make.tmp.$$$$)

# Check BibTeX log for:
# - missing .bib files
# - missing database entries
# - repeated entries
# - string name undefined
# - errors in entries ("I was expecting...")
badbibs: proposal.blg
	@(grep -i "i couldn't open database file " $< > make.tmp.$$$$ ;\
	if [ -s make.tmp.$$$$ ]; then \
	  echo -e "\n===== MISSING BIB FILES =====" ; \
	  cat make.tmp.$$$$; \
	fi; \
	$(RM) make.tmp.$$$$)
	@(grep -i "warning--i didn't find a database entry for" \
	  $< > make.tmp.$$$$ ;\
	if [ -s make.tmp.$$$$ ]; then \
	  echo -e "\n===== MISSING BIB ENTRIES =====" ; \
	  cat make.tmp.$$$$; \
	fi; \
	$(RM) make.tmp.$$$$)
	@(grep -i "repeated entry--"  $< > make.tmp.$$$$ ;\
	if [ -s make.tmp.$$$$ ]; then \
	  echo -e "\n===== REPEATED BIB ENTRIES =====" ; \
	  cat make.tmp.$$$$; \
	fi; \
	$(RM) make.tmp.$$$$)
	@(grep -i "warning--string name" $< > make.tmp.$$$$ ;\
	if [ -s make.tmp.$$$$ ]; then \
	  echo -e "\n===== UNDEFINED STRINGS IN BIB =====" ; \
	  cat make.tmp.$$$$; \
	fi; \
	$(RM) make.tmp.$$$$)
	@(grep -i "i was expecting"  $< > make.tmp.$$$$ ;\
	if [ -s make.tmp.$$$$ ]; then \
	  echo -e "\n===== ERRORS IN BIB ENTRIES =====" ; \
	  cat make.tmp.$$$$; \
	fi; \
	$(RM) make.tmp.$$$$)

###############################################################################
# Actually build the core documents!
###############################################################################

# Dependencies
abstract.pdf:	proposal.pdf
	mv proposal.pdf abstract.pdf

proposal.pdf: $(PROPOSAL_TEX) $(GRAPHICS) $(BIBS)

# Abstract for ARO/NSA BAA Grants.gov
field7.pdf: $(PROPOSAL_TEX) $(GRAPHICS) $(BIBS)

# Bibliography
field9.pdf: $(PROPOSAL_TEX) $(GRAPHICS) $(BIBS)

# Budget for ARO/NSA BAA Grants.gov
field12.pdf: $(PROPOSAL_TEX) $(GRAPHICS) $(BIBS)

%-bio.pdf: $(PROPOSAL_TEX) $(GRAPHICS) $(BIBS)
%-support.pdf: $(PROPOSAL_TEX) $(GRAPHICS) $(BIBS)

%.pdf: %.tex
	${LATEX} $(basename $<)
	-bibtex $(basename $<)
	${LATEX} $(basename $<)
	${LATEX} $(basename $<)

dvi: ${PROPOSAL_TEX}
	latex proposal
	bibtex proposal
	latex proposal
	latex proposal

###############################################################################
# Good hygine
###############################################################################

clean:
	$(RM) *.aux *.log *.bbl *.blg *.toc *.out narrative/*.aux *.maf *.mtc*

realclean: clean
	$(RM) *.pdf

svnignore: .svnignore
	svn propset -R svn:ignore -F .svnignore . # Don't miss the final '.'
