# This demonstration shows a way to source a script in a makefile,
# and export environment variables set by that script to make's
# environment:
#  - form a unique name $LMI_ENV_FILE for a file to hold the value
#  - add a 'source_env_vars' prerequisite for the top-level makefile,
#    with recipe "$(eval include $LMI_ENV_FILE)"
#  - add a phony 'source_env_vars' target that sources the script
#  - make the script write 'make' assignments like "export foo := bar"
#    for each desired environment variable to $LMI_ENV_FILE iff that
#    filename is of nonzero length
# To test:
#   $export LMI_IN=Russia;   make -f parent.make all
#   $export LMI_IN=Mongolia; make -f parent.make all
# and check what appears on stdout.

export LMI_ENV_FILE := env_$(shell date -u +'%s_%N').eraseme

parent.make:: source_env_vars ;
	$(eval include $(LMI_ENV_FILE))
	@echo "eval in 'parent.make': LMI_IN $$LMI_IN; LMI_OUT1 $$LMI_OUT1; LMI_OUT2 $$LMI_OUT2"
# '--force': the file won't be there when this makefile is remade
	rm --force $(LMI_ENV_FILE)

.PHONY: source_env_vars
source_env_vars:
	@echo "Sourcing 'set.sh'"; \
	. ./set.sh ; \
	echo "sourced in 'parent.make': LMI_IN $$LMI_IN; LMI_OUT1 $$LMI_OUT1; LMI_OUT2 $$LMI_OUT2"

all:
	@echo "targets in 'parent.make': LMI_IN $$LMI_IN; LMI_OUT1 $$LMI_OUT1; LMI_OUT2 $$LMI_OUT2"
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
