# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

###############
## Docker Images.
DSE_CDOCGEN_IMAGE ?= dse-cdocgen:latest
DSE_PLANTUML_IMAGE ?= dse-plantuml:latest


###############
## Document Modules

# File "_index.md"
define _INDEX_MD_FILE =
---
title: "C Lib API Reference"
linkTitle: "CLib"
weight: 180
---

## C Lib API Reference
endef
export _INDEX_MD_FILE
DOC_OUTPUT_index := doc/content/apis/clib/_index.md


# Module "marshal"
DOC_INPUT_marshal := dse/clib/data/marshal.h
DOC_CDIR_marshal := dse/clib/data/marshal.c
DOC_OUTPUT_marshal := doc/content/apis/clib/marshal/index.md
DOC_LINKTITLE_marshal := Marshal
DOC_TITLE_marshal := "Marshal API Reference"

# Module "mdf"
DOC_INPUT_mdf := dse/clib/mdf/mdf.h
DOC_CDIR_mdf := dse/clib/mdf/mdf.c
DOC_OUTPUT_mdf := doc/content/apis/clib/mdf/index.md
DOC_LINKTITLE_mdf := Mdf
DOC_TITLE_mdf := "Mdf API Reference"

# Module "schedule"
DOC_INPUT_schedule := dse/clib/schedule/schedule.h
DOC_CDIR_schedule := dse/clib/schedule/schedule.c
DOC_OUTPUT_schedule := doc/content/apis/clib/schedule/index.md
DOC_LINKTITLE_schedule := Schedule
DOC_TITLE_schedule := "Schedule API Reference"

# Targets
DOC_C_MODULES := marshal mdf schedule


.PHONY: examples
examples:
	cd ..;mkdir -p doc/content/apis/clib/examples
	cd ..;cp dse/clib/examples/mdf_file/mdf_file.c doc/content/apis/clib/examples/mdf_file.c
	cd ..;cp dse/clib/examples/schedule/schedule.c doc/content/apis/clib/examples/schedule.c

.PHONY: index
index:
	@cd ..; mkdir -p $$(dirname $(DOC_OUTPUT_index))
	@cd ..; echo "$$_INDEX_MD_FILE" > $(DOC_OUTPUT_index)

.PHONY: $(DOC_C_MODULES)
$(DOC_C_MODULES):
	@cd ..; mkdir -p $$(dirname $(DOC_OUTPUT_$@))
	@cd ..; docker run -it --rm \
		--volume $$(pwd):/tmp/repo \
		--workdir /tmp/repo \
		$(DSE_CDOCGEN_IMAGE) \
			--input $(DOC_INPUT_$@) \
			--output $(DOC_OUTPUT_$@) \
			--cdir $(DOC_CDIR_$@) \
			--title $(DOC_TITLE_$@) \
			--linktitle $(DOC_LINKTITLE_$@)
	@cd ../$$(dirname $(DOC_OUTPUT_$@)); docker run -it --rm \
		--volume $$(pwd):/tmp/repo \
		--workdir /tmp/repo \
		$(DSE_PLANTUML_IMAGE) \
			-tpng  $$(basename $(DOC_OUTPUT_$@))

.PHONY: generate
generate: index examples $(DOC_C_MODULES)
