SRCS:=src/main.cpp src/input.cpp src/state.cpp
INDIR:=input
OUTDIR:=output
# INPUTS := $(INDIR)/problem-8.json $(INDIR)/problem-11.json $(INDIR)/problem-12.json $(INDIR)/problem-25.json $(INDIR)/problem-26.json $(INDIR)/problem-27.json $(INDIR)/problem-28.json $(INDIR)/problem-29.json $(INDIR)/problem-30.json
INPUTS=$(wildcard $(INDIR)/problem-*.json)
OUTPUTS=$(patsubst $(INDIR)/%,$(OUTDIR)/%,$(INPUTS))
# CXXFLAGS := -Wall -std=c++17 -g -fsanitize=address
CXXFLAGS := -Ofast -march=native -Wall -std=c++17

all: output

bin/solve: $(SRCS)
	g++ $(CXXFLAGS) $^ -o $@

.PHONY: output
output: bin/solve $(OUTPUTS)

# https://stackoverflow.com/questions/65094616/gnu-make-path-substitution-directory-flattening
define DEP_rule
$(1)-out := $$(patsubst $$(INDIR)/%,$$(OUTDIR)/%,$(1))

$$($(1)-out): $(1) bin/solve
	bin/solve $$< > $$@ 2> $$@.err
	python3 tools/submit.py $$@
endef
$(foreach src,$(INPUTS),$(eval $(call DEP_rule,$(src))))