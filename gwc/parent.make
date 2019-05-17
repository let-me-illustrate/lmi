# IMPORTANT: the former 'env.make' has been replaced by
#    env.$(LMI_TIMESTAMP).eraseme
# in the non-commented-out code, but not in any comments.
# Yet 'env.make' remains as a target; is that target's name
# actually arbitrary?

# This demonstration shows a way to source a script in a makefile,
# and export environment variables set by that script to make's
# environment, thus:
#  - add an 'env.make' prerequisite for the top-level makefile,
#    with recipe "$(eval include env.make)"
#  - add a phony target for 'env.make' that sources the script,
#    then writes 'make' assignments like "export foo := bar"
#    for each desired environment variable
# To test:
#   $export LMI_IN=Russia; LANG=ru_RU make -f parent.make all
#   $export LMI_IN=Mongolia; LANG=mn_MN make -f parent.make all
# and check what appears on stdout.

# A file named 'env.make' is created in the source directory.
# That unwanted side effect can be avoided by replacing it
# with, say, '/run/var/lmi/env.make'.

# lmi's 'GNUmakefile' uses a double-colon rule like this:
#parent.make $(srcdir)/parent.make:: ;
# which becomes relevant below. Possible problem: adding a
# prerequisite may impair the efficiency that was gained by
# writing the double-colon rule.

export LMI_TIMESTAMP := $(shell date -u +'%s.%N')

# No good: syntax error.
#include set.sh

# Neither necessary nor sufficient.
#include env.make

# This can't coexist with the desired double-colon rule (see above):
#parent.make: env.make
parent.make parent.make:: env.make ;
# $(eval include) here is necessary and sufficient.
	$(eval include env.$(LMI_TIMESTAMP).eraseme)
	@echo "eval: LMI_IN in 'parent.make': $$LMI_IN"
	@echo "eval: LMI_OUT1 in 'parent.make': $$LMI_OUT1"
	@echo "eval: LMI_OUT2 in 'parent.make': $$LMI_OUT2"
# '--force': the file won't be there when this makefile is remade
	rm --force env.$(LMI_TIMESTAMP).eraseme

# Doesn't execute recipe without ".PHONY":
.PHONY: env.make
# This prerequisite is unnecessary:
#env.make: set.sh
env.make:
	@echo "Sourcing 'set.sh'"
	. ./set.sh
	@echo "target: LMI_IN in 'parent.make': $$LMI_IN"
	@echo "target: LMI_OUT1 in 'parent.make': $$LMI_OUT1 ...but wait..."
	@echo "target: LMI_OUT2 in 'parent.make': $$LMI_OUT2 ...but wait..."

all:
	@echo "LMI_OUT1 for targets in 'parent.make': $$LMI_OUT1"
	@echo "LMI_OUT2 for targets in 'parent.make': $$LMI_OUT2"
	$(MAKE) --no-print-directory -f child.make

# Obviously one could simply write a cover script to replace direct
# invocation of 'make', but that's nasty. See:
#   https://lists.gnu.org/archive/html/help-make/2006-04/msg00142.html
# which suggests 'eval'.
#
# This is unhelpful:
#   https://stackoverflow.com/questions/7507810/how-to-source-a-script-in-a-makefile
# except that it has a link to:
#   https://blog.153.io/2016/04/18/source-a-shell-script-in-make/
# which doesn't actually work in its final, simplified version;
# but earlier in that article it uses 'eval include', which does work.
