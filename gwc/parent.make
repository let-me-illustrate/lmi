# This demonstration shows a way to source a script in a makefile,
# and export environment variables set by that script to make's
# environment:
#  - form a unique name $LMI_ENV_FILE for a file to hold the value
#  - give the top-level makefile a target to remake itself, with
#     - $(eval include $(LMI_ENV_FILE)) in its recipe, and
#     - $(LMI_ENV_FILE) as a prerequisite
#  - add a phony $(LMI_ENV_FILE) target whose recipe sources the
#    script and writes 'make' assignments like "export foo := bar"
#    into the file it names
# To test:
#   $export LMI_IN=Russia;   make -f parent.make all
#   $export LMI_IN=Mongolia; make -f parent.make all
# and check what appears on stdout.

export LMI_ENV_FILE := env_$(shell date -u +'%s_%N').eraseme

parent.make:: $(LMI_ENV_FILE)
	$(eval include $(LMI_ENV_FILE))
	@echo "'$$LMI_IN' --> '$$LMI_OUT1', '$$LMI_OUT2' : eval in 'parent.make'"
	rm $(LMI_ENV_FILE)

$(LMI_ENV_FILE):
	@echo "Sourcing 'set.sh'"
	@. ./set.sh ; \
	  { \
	    echo "export LMI_OUT1 := $$LMI_OUT1"; \
	    echo "export LMI_OUT2 := $$LMI_OUT2"; \
	  } > $@ ; \
	echo "'$$LMI_IN' --> '$$LMI_OUT1', '$$LMI_OUT2' : sourced in 'parent.make'"

all:
	@echo "'$$LMI_IN' --> '$$LMI_OUT1', '$$LMI_OUT2' : targets in 'parent.make'"
	$(MAKE) --no-print-directory -f child.make

# For real-world use, commit 3ff6c008 seems preferable. It writes
# 'make' assignments in the script rather than in this makefile,
# facilitating maintenance by keeping the list of variables in a
# single file. See this discussion:
#   https://lists.nongnu.org/archive/html/lmi/2019-05/msg00052.html

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
