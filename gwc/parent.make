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

# How does this work?
#
# Only a few lines of code are required, and the subtlest one:
#   $(eval include $(LMI_ENV_FILE))
# isn't very difficult: it simply expands to an appropriate 'include'
# directive at a particular time. The real challenge is understanding
# the implicit synchronization. It is instructive to run test cases
# with 'make --trace', whose output is paraphrased here:
#
# begin reading parent.make
#   parent.make has $(LMI_ENV_FILE) as a prerequisite
#     therefore, must remake $(LMI_ENV_FILE)
#       source the script; in the same shell,
#       write 'make' variable definitions to $(LMI_ENV_FILE)
#     finished remaking $(LMI_ENV_FILE)
#   finished prerequisites of parent.make
# resume reading parent.make
#   now $(LMI_ENV_FILE) is newer than parent.make
#   therefore, must remake parent.make
#     its recipe contains $(eval include $(LMI_ENV_FILE))
#     which expands to a make directive
#     which is executed when parent.make is reread
#     which causes the definitions to take effect
#   finished remaking parent.make
# finished reading parent.make
#
# It is tempting to rearrange the code to make it simpler, but the
# '--trace' output shows why that doesn't work--for example:
#
#  - Can the two recipes be combined into one, sourcing the script
# and writing the temporary file just before $(eval)? No: $(eval)
# would take effect before the script has been sourced. 'include'
# is a make directive, not a shell command, so its effect occurs
# when the makefile is reread--before the recipe is executed.
#
#  - Can $(eval include) be moved outside the recipe it occurs in,
# and written outside any rule context (as directives typically are)?
# No: it would take effect the first time the makefile is read, before
# the script has been sourced, and it wouldn't be reconsidered later.
# Functions are usually expanded when read, but not in rule contexts,
# where expansion is deferred.

# Prior art; alternatives
#
# Obviously one could simply write a cover script to replace direct
# invocation of 'make', but the goal here is to avoid that. See:
#   https://lists.gnu.org/archive/html/help-make/2006-04/msg00142.html
# which suggests 'eval'.
#
# An online search for 'make "eval include"' finds few articles, and
# fewer still that use that construct in a recipe. This one:
#   https://blog.153.io/2016/04/18/source-a-shell-script-in-make/
# demonstrates the 'eval include' technique adapted here (though it
# doesn't literally source a script). Its earlier 'eval include'
# example works, though its final, simplified example seems not to.
