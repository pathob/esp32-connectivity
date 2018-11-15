#
# "ota" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_EMBED_TXTFILES := certs/s3_amazonaws_com.pem

CFLAGS += -DPROJECT_NAME=\"$(PROJECT_NAME)\"

ifdef TRAVIS_REPO_SLUG
CFLAGS += -DTRAVIS_REPO_SLUG=\"$(TRAVIS_REPO_SLUG)\"
endif

ifdef TRAVIS_TAG
CFLAGS += -DTRAVIS_TAG=\"$(TRAVIS_TAG)\"
endif