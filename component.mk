# "connectivity" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_SRCDIRS := src
COMPONENT_PRIV_INCLUDEDIRS := include/connectivity
COMPONENT_EMBED_TXTFILES := certs/s3_amazonaws_com.pem

ifdef GITHUB_REPO_SLUG
CFLAGS_REPO_SLUG ?= $(GITHUB_REPO_SLUG)
endif
ifdef TRAVIS_REPO_SLUG
CFLAGS_REPO_SLUG ?= $(TRAVIS_REPO_SLUG)
endif
ifdef CFLAGS_REPO_SLUG
CFLAGS += -DGITHUB_REPO_SLUG=\"$(CFLAGS_REPO_SLUG)\"
endif

ifdef GITHUB_TAG
CFLAGS_TAG ?= $(GITHUB_TAG)
endif
ifdef TRAVIS_TAG
CFLAGS_TAG ?= $(TRAVIS_TAG)
endif
ifdef CFLAGS_TAG
CFLAGS += -DGITHUB_TAG=\"$(CFLAGS_TAG)\"
endif