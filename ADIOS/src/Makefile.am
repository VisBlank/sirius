AM_CPPFLAGS=-I$(top_builddir)/src/public -I$(top_srcdir) -I$(top_srcdir)/src/core -I$(top_srcdir)/src/core/transforms -I$(top_srcdir)/src/core/query

if BUILD_MXML
SUBDIRS = mxml 
MXML_BUILDDIR=mxml/mxml-2.9
MXML_LIB=libmxml_a
MXML_OBJECTS = ${MXML_BUILDDIR}/${MXML_LIB}-mxml-attr.${OBJEXT}     \
                     ${MXML_BUILDDIR}/${MXML_LIB}-mxml-get.${OBJEXT}      \
                     ${MXML_BUILDDIR}/${MXML_LIB}-mxml-private.${OBJEXT}  \
                     ${MXML_BUILDDIR}/${MXML_LIB}-mxml-string.${OBJEXT}   \
                     ${MXML_BUILDDIR}/${MXML_LIB}-mxml-entity.${OBJEXT}   \
                     ${MXML_BUILDDIR}/${MXML_LIB}-mxml-index.${OBJEXT}    \
                     ${MXML_BUILDDIR}/${MXML_LIB}-mxml-search.${OBJEXT}   \
                     ${MXML_BUILDDIR}/${MXML_LIB}-mxml-file.${OBJEXT}     \
                     ${MXML_BUILDDIR}/${MXML_LIB}-mxml-node.${OBJEXT}     \
                     ${MXML_BUILDDIR}/${MXML_LIB}-mxml-set.${OBJEXT} 
endif

AUTOMAKE_OPTIONS = no-dependencies subdir-objects
METASOURCES=AUTO

bin_PROGRAMS=
#if HAVE_NSSI
#
#bin_PROGRAMS+=nssi-staging-server
#
#nssi_staging_server_SOURCES = nssi/nssi_staging_server.cpp nssi/aggregation.cpp
#nssi_staging_server_CFLAGS=$(ADIOSLIB_CFLAGS)
#nssi_staging_server_CPPFLAGS=$(ADIOSLIB_EXTRA_CPPFLAGS) $(ADIOSLIB_CPPFLAGS) $(MACRODEFFLAG)PTL_IFACE_CLIENT=CRAY_USER_NAL $(MACRODEFFLAG)PTL_IFACE_SERVER=CRAY_USER_NAL
#nssi_staging_server_LDFLAGS = $(ADIOSLIB_LDFLAGS) 
#nssi_staging_server_LDADD = $(top_builddir)/src/libadios.a
#nssi_staging_server_LDADD += $(ADIOSLIB_LDADD) $(NSSI_SERVER_LIBS)
#
#bin_PROGRAMS+=nssi-coupling-server
#
#nssi_coupling_server_SOURCES = nssi/nssi_coupling_server.cpp
#nssi_coupling_server_CFLAGS=$(ADIOSLIB_CFLAGS)
#nssi_coupling_server_CPPFLAGS=$(ADIOSLIB_EXTRA_CPPFLAGS) $(ADIOSLIB_CPPFLAGS) $(MACRODEFFLAG)PTL_IFACE_CLIENT=CRAY_USER_NAL $(MACRODEFFLAG)PTL_IFACE_SERVER=CRAY_USER_NAL # $(MACRODEFFLAG)USE_TIMERS
#nssi_coupling_server_LDFLAGS = $(ADIOSLIB_LDFLAGS) 
#nssi_coupling_server_LDADD = $(top_builddir)/src/libadios.a
#nssi_coupling_server_LDADD += $(ADIOSLIB_LDADD) $(NSSI_SERVER_LIBS)
#
#endif # HAVE_NSSI

CLEANFILES= 
include_HEADERS=
nodist_include_HEADERS=
lib_LIBRARIES=

libadiosread_a_CPPFLAGS=$(AM_CPPFLAGS)
libadiosreadf_a_CPPFLAGS=$(AM_CPPFLAGS)
libadiosread_nompi_a_CPPFLAGS=$(AM_CPPFLAGS)
libadiosreadf_nompi_a_CPPFLAGS=$(AM_CPPFLAGS)

libadiosread_a_CFLAGS= 
libadiosreadf_a_CFLAGS= 
libadiosread_nompi_a_CFLAGS= 
libadiosreadf_nompi_a_CFLAGS= 

## Transform plugin source files

transforms_write_method_SOURCES=
transforms_read_method_SOURCES=

include transforms/Makefile.plugins

##
## Core transform framework source files
##
transforms_common_HDRS = core/adios_copyspec.h \
                         core/adios_subvolume.h \
                         core/adios_selection_util.h \
                         core/transforms/adios_transforms_common.h \
                         core/transforms/adios_transforms_hooks.h \
                         core/transforms/adios_transforms_util.h 

transforms_read_HDRS = core/transforms/adios_transforms_read.h \
                       core/transforms/adios_transforms_hooks_read.h \
                       core/transforms/adios_transforms_reqgroup.h \
                       core/transforms/adios_transforms_datablock.h \
                       core/transforms/adios_transforms_transinfo.h \
                       core/transforms/adios_patchdata.h

transforms_write_HDRS = core/transforms/adios_transforms_write.h \
                        core/transforms/adios_transforms_hooks_write.h \
                        core/transforms/adios_transforms_specparse.h


transforms_common_SOURCES = $(transforms_common_HDRS) \
                            core/transforms/adios_transforms_common.c \
                            core/transforms/adios_transforms_hooks.c \
                            core/adios_copyspec.c \
                            core/adios_subvolume.c \
                            core/transforms/plugindetect/detect_plugin_infos.h \
                            core/transforms/plugindetect/detect_plugin_types.h \
                            core/transforms/plugindetect/plugin_info_types.h

transforms_read_SOURCES = $(transforms_read_HDRS) \
                          core/transforms/adios_transforms_read.c \
                          core/transforms/adios_transforms_hooks_read.c \
                          core/transforms/adios_transforms_reqgroup.c \
                          core/transforms/adios_transforms_datablock.c \
                          core/transforms/adios_patchdata.c \
                          core/adios_selection_util.c \
                          core/transforms/plugindetect/detect_plugin_read_hook_decls.h \
                          core/transforms/plugindetect/detect_plugin_read_hook_reg.h \
                          $(transforms_read_method_SOURCES)

transforms_write_SOURCES = $(transforms_write_HDRS) \
                           core/transforms/adios_transforms_write.c \
                           core/transforms/adios_transforms_hooks_write.c \
                           core/transforms/adios_transforms_util.c \
                           core/transforms/adios_transforms_specparse.c \
                           core/transforms/plugindetect/detect_plugin_write_hook_decls.h \
                           core/transforms/plugindetect/detect_plugin_write_hook_reg.h \
                           $(transforms_write_method_SOURCES)

#######Query source files 

query_common_HDRS = query/common_query.h query/adios_query_hooks.h query/query_utils.h
query_common_SOURCES = $(query_common_HDRS) \
                       query/common_query.c  \
                       query/common_query_read.c  \
                       query/adios_query_hooks.c \
                       query/query_utils.c

# Include source files that are specific to each query plugin
query_method_HDRS = 
query_method_SOURCES =
include query/Makefile.plugins

query_method_SOURCES += $(query_method_HDRS) 

query_SOURCES      = $(query_common_SOURCES) \
                     $(query_method_SOURCES)

query_C_SOURCES    = $(query_SOURCES) query/adios_query.c
query_F_SOURCES    = $(query_SOURCES) query/adiosf_query.c


noinst_LIBRARIES = libcoreonce.a 
libcoreonce_a_SOURCES = core/a2sel.c \
                            core/adios_bp_v1.c \
                            core/adios_clock.c \
                            core/adios_endianness.c \
                            core/adios_error.c \
                            core/adios_infocache.c \
                            core/adios_logger.c \
                            core/adios_socket.c \
                            core/buffer.c \
                            core/futils.c \
                            core/globals.c \
                            core/qhashtbl.c \
                            core/strutil.c \
                            core/util.c \
                            $(transforms_common_SOURCES) \
                            $(transforms_write_SOURCES) \
                            $(transforms_read_SOURCES) 

libcoreonce_a_CPPFLAGS = $(AM_CPPFLAGS) $(ADIOSLIB_EXTRA_CPPFLAGS) $(ADIOSLIB_CPPFLAGS) # $(MACRODEFFLAG)USE_TIMERS
libcoreonce_a_CFLAGS = $(ADIOSLIB_CFLAGS) 

if BUILD_WRITE

lib_LIBRARIES += libadios.a libadios_nompi.a
libadios_a_SOURCES = core/adios.c \
                     core/common_adios.c \
                     core/adios_internals.c \
                     core/adios_internals_mxml.c \
                     core/bp_utils.c \
                     core/adios_read.c \
                     core/adios_read_v1.c \
                     core/common_read.c \
                     core/adios_read_ext.c \
                     core/adios_timing.c \
                     core/adios_read_hooks.c \
                     core/adios_transport_hooks.c \
                     core/util_mpi.c \
                     $(query_C_SOURCES) \
                     read/read_bp.c \
                     read/read_bp_staged.c \
                     read/read_bp_staged1.c \
                     write/adios_mpi.c \
                     write/adios_mpi_lustre.c \
                     write/adios_mpi_amr.c \
                     write/adios_posix.c \
                     write/adios_var_merge.c 
if HAVE_BGQ
libadios_a_SOURCES += write/adios_mpi_bgq.c 
endif
libadios_nompi_a_SOURCES = core/adios.c \
                     core/common_adios.c \
                     core/adios_internals.c \
                     core/adios_internals_mxml.c \
                     $(query_C_SOURCES) \
                     core/bp_utils.c \
                     core/adios_read.c \
                     core/adios_read_v1.c \
                     core/common_read.c \
                     core/adios_read_ext.c \
                     core/mpidummy.c \
                     core/adios_timing.c \
                     core/adios_read_hooks.c \
                     core/adios_transport_hooks.c \
                     core/util_mpi.c \
                     read/read_bp.c \
                     read/read_bp_staged.c \
                     read/read_bp_staged1.c \
                     write/adios_posix.c 


libadios_a_LIBADD = $(libcoreonce_a_OBJECTS) 
libadios_nompi_a_LIBADD = $(libcoreonce_a_OBJECTS)
if BUILD_MXML
libadios_a_LIBADD += ${MXML_OBJECTS} 
libadios_nompi_a_LIBADD += ${MXML_OBJECTS} 
endif
# force building the noinst_LIBRARIES first, before libadios.a
EXTRA_libadios_a_DEPENDENCIES = libcoreonce.a 
EXTRA_libadios_nompi_a_DEPENDENCIES = libcoreonce.a 

if BUILD_FORTRAN
lib_LIBRARIES += libadiosf.a libadiosf_v1.a libadiosf_nompi.a libadiosf_nompi_v1.a
FortranLibSources = core/adiosf.c \
                     core/common_adios.c \
                     core/adios_internals.c \
                     core/adios_internals_mxml.c \
                     $(query_F_SOURCES) \
                     core/bp_utils.c \
                     core/common_read.c \
                     core/adios_infocache.c \
                     core/adios_read_ext.c \
                     core/adios_timing.c \
                     core/adios_read_hooks.c \
                     core/adios_transport_hooks.c \
                     core/util_mpi.c \
                     read/read_bp.c \
                     read/read_bp_staged.c \
                     read/read_bp_staged1.c \
                     write/adios_posix.c 

FortranLibMPISources =  write/adios_mpi.c \
                     write/adios_mpi_lustre.c \
                     write/adios_mpi_amr.c \
                     write/adios_var_merge.c
if HAVE_BGQ
FortranLibMPISources += write/adios_mpi_bgq.c  
endif
FortranLibSEQSources = core/mpidummy.c

endif # BUILD_FORTRAN

libadios_a_CPPFLAGS = $(AM_CPPFLAGS) $(ADIOSLIB_EXTRA_CPPFLAGS) $(ADIOSLIB_CPPFLAGS) # $(MACRODEFFLAG)USE_TIMERS
libadios_a_CFLAGS = $(ADIOSLIB_CFLAGS) 
libadios_nompi_a_CPPFLAGS = $(AM_CPPFLAGS) $(MACRODEFFLAG)_NOMPI $(ADIOSLIB_EXTRA_CPPFLAGS) $(ADIOSLIB_SEQ_CPPFLAGS) # $(MACRODEFFLAG)USE_TIMERS
libadios_nompi_a_CFLAGS = $(MACRODEFFLAG)_NOMPI $(ADIOSLIB_SEQ_CFLAGS) 
libadiosf_a_CPPFLAGS = $(AM_CPPFLAGS) $(ADIOSLIB_EXTRA_CPPFLAGS) $(ADIOSLIB_CPPFLAGS) # $(MACRODEFFLAG)USE_TIMERS
libadiosf_a_CFLAGS = $(ADIOSLIB_CFLAGS) 
libadiosf_nompi_a_CPPFLAGS = $(AM_CPPFLAGS) $(MACRODEFFLAG)_NOMPI $(ADIOSLIB_EXTRA_CPPFLAGS) $(ADIOSLIB_SEQ_CPPFLAGS) # $(MACRODEFFLAG)USE_TIMERS
libadiosf_nompi_a_CFLAGS = $(MACRODEFFLAG)_NOMPI $(ADIOSLIB_SEQ_CFLAGS) 

if HAVE_DATASPACES
libadios_a_SOURCES += write/adios_dataspaces.c read/read_dataspaces.c
#libadios_nompi_a_SOURCES += write/adios_dataspaces.c read/read_dataspaces.c
if BUILD_FORTRAN
FortranLibMPISources += write/adios_dataspaces.c read/read_dataspaces.c
endif
endif

if HAVE_DIMES
libadios_a_SOURCES += write/adios_dimes.c read/read_dimes.c
#libadios_nompi_a_SOURCES += write/adios_dimes.c read/read_dimes.c
if BUILD_FORTRAN
FortranLibMPISources += write/adios_dimes.c read/read_dimes.c
endif
endif

#if HAVE_DATATAP
#libadios_a_SOURCES += write/adios_datatap.c read/read_datatap.c
#libadios_nompi_a_SOURCES += write/adios_datatap.c read/read_datatap.c
#if BUILD_FORTRAN
#FortranLibSources += write/adios_datatap.c read/read_datatap.c
#endif
#endif 


if HAVE_PHDF5
libadios_a_SOURCES += write/adios_phdf5.c 
PHDF5_CPPFLAGS += $(MACRODEFFLAG)PHDF5 $(MACRODEFFLAG)H5_USE_16_API 
libadios_a_CPPFLAGS += $(PHDF5_CPPFLAGS)
if BUILD_FORTRAN
FortranLibMPISources += write/adios_phdf5.c 
libadiosf_a_CPPFLAGS += $(PHDF5_CPPFLAGS)
endif
endif

if HAVE_NC4PAR
libadios_a_SOURCES += write/adios_nc4.c 
if BUILD_FORTRAN
FortranLibMPISources += write/adios_nc4.c 
endif
endif

if HAVE_FLEXPATH
libadios_a_SOURCES += write/adios_flexpath.c  read/read_flexpath.c
libadios_nompi_a_SOURCES += write/adios_flexpath.c  read/read_flexpath.c
if BUILD_FORTRAN
FortranLibSources += write/adios_flexpath.c read/read_flexpath.c
endif
endif

if HAVE_ICEE
libadios_a_SOURCES += write/adios_icee.c read/read_icee.c
libadios_nompi_a_SOURCES += write/adios_icee.c  read/read_icee.c
if BUILD_FORTRAN
FortranLibSources += write/adios_icee.c read/read_icee.c
endif
endif

#if HAVE_NSSI
#dist_libadios_a_SOURCES = nssi/adios_nssi_args.x
#nodist_libadios_a_SOURCES = adios_nssi_args.c adios_nssi_args.h
#libadios_a_SOURCES += nssi/adios_nssi_config.c write/adios_nssi.c nssi/adios_nssi_filter.c read/read_nssi.c
#libadios_a_CPPFLAGS += $(MACRODEFFLAG)PTL_IFACE_CLIENT=CRAY_USER_NAL $(MACRODEFFLAG)PTL_IFACE_SERVER=CRAY_USER_NAL
#if BUILD_FORTRAN
#nodist_libadiosf_a_SOURCES = adios_nssi_args.c adios_nssi_args.h
#dist_libadiosf_a_SOURCES = nssi/adios_nssi_args.x
#FortranLibSources += nssi/adios_nssi_config.c write/adios_nssi.c nssi/adios_nssi_filter.c read/read_nssi.c
#libadiosf_a_CPPFLAGS += $(MACRODEFFLAG)PTL_IFACE_CLIENT=CRAY_USER_NAL $(MACRODEFFLAG)PTL_IFACE_SERVER=CRAY_USER_NAL
#endif
#
## The next line forces automake to build adios_nssi_args.[ch] before compiling adios_nssi_config.c
## Note, the order of dist_*SOURCES and nodist_*SOURCES above should but does not solve this alone.
#adios_nssi_config.c: adios_nssi_args.c
#
#adios_nssi_args.h:  $(srcdir)/nssi/adios_nssi_args.x Makefile.am
#        rm -f $@
###        @echo "#include <lwfs/lwfs_config.h>" >> $@
###        @echo "#ifndef LWFS_HAVE_STRUCT_SOCKADDR_IN" >> $@
###        @echo "#define LWFS_HAVE_STRUCT_SOCKADDR_IN" >> $@
###        @echo "struct sockaddr_in { " >> $@
###        @echo '  int a;' >> $@
###        @echo '};' >> $@
###        @echo '#endif' >> $@;
#        rpcgen -h $(srcdir)/nssi/adios_nssi_args.x >> $@
#
#adios_nssi_args.c: adios_nssi_args.h 
#        rm -f adios_nssi_args.c
#        rpcgen -c $(srcdir)/nssi/adios_nssi_args.x \
#                | grep -v "include \"\.\." > adios_nssi_args.c
#
#CLEANFILES += adios_nssi_args.c adios_nssi_args.h
#
#endif # HAVE_NSSI


if RESEARCH_TRANSPORTS
#libadios_a_SOURCES += write/adios_mpi_stagger.c \
#                      write/adios_mpi_aggregate.c \
#                      write/adios_provenance.c \
#                      write/adios_mpi_cio.c \
#                      write/adios_mpi_stripe.c \
#                      write/adios_mpi_amr1.c \
#                      write/adios_adaptive.c 
if BUILD_FORTRAN
#FortranLibMPISources += write/adios_mpi_stagger.c \
#                       write/adios_mpi_aggregate.c \
#                       write/adios_provenance.c \
#                       write/adios_mpi_cio.c \
#                       write/adios_mpi_stripe.c \
#                       write/adios_mpi_amr1.c \
#                       write/adios_adaptive.c 
endif
else
libadios_a_CPPFLAGS += $(MACRODEFFLAG)NO_RESEARCH_TRANSPORTS
libadios_nompi_a_CPPFLAGS += $(MACRODEFFLAG)NO_RESEARCH_TRANSPORTS
libadiosf_a_CPPFLAGS += $(MACRODEFFLAG)NO_RESEARCH_TRANSPORTS
libadiosf_nompi_a_CPPFLAGS += $(MACRODEFFLAG)NO_RESEARCH_TRANSPORTS
endif  # RESEARCH_TRANSPORTS

# Build four different Fortran libraries, for the two read APIs
# and their sequential versions
if BUILD_FORTRAN
libadiosf_a_SOURCES = $(FortranLibSources) $(FortranLibMPISources) core/adiosf_read.c
libadiosf_v1_a_SOURCES = $(FortranLibSources) $(FortranLibMPISources) core/adiosf_read_v1.c
libadiosf_nompi_a_SOURCES = $(FortranLibSources) $(FortranLibSEQSources) core/adiosf_read.c
libadiosf_nompi_v1_a_SOURCES = $(FortranLibSources) $(FortranLibSEQSources) core/adiosf_read_v1.c
libadiosf_v1_a_CPPFLAGS = $(libadiosf_a_CPPFLAGS) 
libadiosf_v1_a_CFLAGS = $(libadiosf_a_CFLAGS) 
libadiosf_nompi_v1_a_CPPFLAGS = $(libadiosf_nompi_a_CPPFLAGS) 
libadiosf_nompi_v1_a_CFLAGS = $(libadiosf_nompi_a_CFLAGS) 

libadiosf_a_SOURCES += core/adiosf_defs_mod.f90 \
                       core/adiosf_write_mod.f90 \
                       core/adiosf_read_mod.f90 \
                       query/adios_query_mod.f90

libadiosf_nompi_a_SOURCES += core/adiosf_defs_mod.f90 \
                             core/adiosf_write_mod.f90 \
                             core/adiosf_read_mod.f90  \
                             query/adios_query_mod.f90

libadiosf_v1_a_SOURCES += core/adiosf_defs_mod.f90 \
                          core/adiosf_write_mod.f90 

libadiosf_nompi_v1_a_SOURCES += core/adiosf_defs_mod.f90 \
                                core/adiosf_write_mod.f90 

libadiosf_a_LIBADD = $(libcoreonce_a_OBJECTS)
libadiosf_v1_a_LIBADD =  $(libcoreonce_a_OBJECTS)
libadiosf_nompi_a_LIBADD =  $(libcoreonce_a_OBJECTS)
libadiosf_nompi_v1_a_LIBADD = $(libcoreonce_a_OBJECTS)
if BUILD_MXML
libadiosf_a_LIBADD += ${MXML_OBJECTS}
libadiosf_v1_a_LIBADD += ${MXML_OBJECTS}
libadiosf_nompi_a_LIBADD += ${MXML_OBJECTS}
libadiosf_nompi_v1_a_LIBADD += ${MXML_OBJECTS} 
endif
EXTRA_libadiosf_a_DEPENDENCIES = libcoreonce.a 
EXTRA_libadiosf_v1_a_DEPENDENCIES = libcoreonce.a 
EXTRA_libadiosf_nompi_a_DEPENDENCIES = libcoreonce.a 
EXTRA_libadiosf_nompi_v1_a_DEPENDENCIES = libcoreonce.a 

#nodist_libadiosreadf_a_SOURCES += adios_write.mod
nodist_include_HEADERS += adios_write_mod.mod 
CLEANFILES += adios_write_mod.mod

core/adiosf_write_mod.$(OBJEXT): core/adiosf_defs_mod.$(OBJEXT)
adios_write_mod.mod: core/adiosf_write_mod.$(OBJEXT)

endif


endif   # BUILD_WRITE

include_HEADERS += public/adios.h \
                   public/adios_version.h \
                   public/adios_types.h \
                   public/adios_read.h \
                   public/adios_error.h \
                   public/adios_mpi.h \
                   public/mpidummy.h \
                   public/adios_read_v1.h \
                   public/adios_read_v1_defs.h \
                   public/adios_read_v2.h \
                   public/adios_read_v2_fwd.h \
                   public/adios_selection.h \
                   public/adios_schema.h \
                   public/adios_link.h \
                   public/adios_read_ext.h \
                   public/adios_query.h

lib_LIBRARIES += libadiosread.a
libadiosread_a_SOURCES = core/bp_utils.c \
                      core/adios_read.c \
                      core/adios_read_v1.c \
                      core/common_read.c \
                      core/adios_read_ext.c \
                      $(query_C_SOURCES) \
                      core/adios_read_hooks.c \
                      core/util_mpi.c \
                      read/read_bp.c \
                      read/read_bp_staged.c \
                      read/read_bp_staged1.c 
if HAVE_DATASPACES
libadiosread_a_SOURCES += read/read_dataspaces.c
endif
if HAVE_DIMES
libadiosread_a_SOURCES += read/read_dimes.c
endif
#if HAVE_DATATAP
#libadiosread_a_SOURCES += read/read_datatap.c
#endif
if HAVE_FLEXPATH
libadiosread_a_SOURCES += read/read_flexpath.c
endif
if HAVE_ICEE
libadiosread_a_SOURCES += read/read_icee.c
endif
#if HAVE_NSSI
#dist_libadiosread_a_SOURCES = nssi/adios_nssi_args.x
#nodist_libadiosread_a_SOURCES = adios_nssi_args.c adios_nssi_args.h
#libadiosread_a_SOURCES += nssi/adios_nssi_config.c read/read_nssi.c
#libadiosread_a_CPPFLAGS += $(MACRODEFFLAG)PTL_IFACE_CLIENT=CRAY_USER_NAL $(MACRODEFFLAG)PTL_IFACE_SERVER=CRAY_USER_NAL
##libadiosread_a_CPPFLAGS += $(ADIOSLIB_EXTRA_CPPFLAGS) $(ADIOSLIB_CPPFLAGS) # $(MACRODEFFLAG)USE_TIMERS
#libadiosread_a_CFLAGS   += $(ADIOSLIB_CFLAGS) 
#endif

libadiosread_a_CPPFLAGS += $(ADIOSLIB_EXTRA_CPPFLAGS) $(ADIOSREADLIB_CPPFLAGS)
libadiosread_a_LIBADD = $(libcoreonce_a_OBJECTS)
EXTRA_libadiosread_a_DEPENDENCIES = libcoreonce.a 

if BUILD_FORTRAN
lib_LIBRARIES += libadiosreadf.a libadiosreadf_v1.a
FortranReadLibSource = core/bp_utils.c \
                      core/common_read.c \
                      core/adios_read_ext.c \
                      $(query_F_SOURCES) \
                      core/adios_read_hooks.c \
                      core/util_mpi.c \
                      read/read_bp.c \
                      read/read_bp_staged.c \
                      read/read_bp_staged1.c 
if HAVE_DATASPACES
FortranReadLibSource += read/read_dataspaces.c
endif
if HAVE_DIMES
FortranReadLibSource += read/read_dimes.c
endif
#if HAVE_DATATAP
#FortranReadLibSource += read/read_datatap.c
#endif
if HAVE_FLEXPATH 
FortranReadLibSource += read/read_flexpath.c
endif
if HAVE_ICEE 
FortranReadLibSource += read/read_icee.c
endif
#if HAVE_NSSI
#dist_libadiosreadf_a_SOURCES = nssi/adios_nssi_args.x
#nodist_libadiosreadf_a_SOURCES = adios_nssi_args.c adios_nssi_args.h
#FortranReadLibSource += nssi/adios_nssi_config.c read/read_nssi.c
#libadiosreadf_a_CPPFLAGS += $(MACRODEFFLAG)PTL_IFACE_CLIENT=CRAY_USER_NAL $(MACRODEFFLAG)PTL_IFACE_SERVER=CRAY_USER_NAL
#libadiosreadf_a_CFLAGS   += $(ADIOSLIB_CFLAGS) 
#endif

libadiosreadf_a_CPPFLAGS += $(ADIOSLIB_EXTRA_CPPFLAGS) $(ADIOSREADLIB_CPPFLAGS)

libadiosreadf_a_SOURCES = $(FortranReadLibSource) core/adiosf_read.c
libadiosreadf_v1_a_SOURCES = $(FortranReadLibSource) core/adiosf_read_v1.c
libadiosreadf_v1_a_CPPFLAGS = $(libadiosreadf_a_CPPFLAGS)
libadiosreadf_v1_a_CFLAGS = $(libadiosreadf_a_CFLAGS)

libadiosreadf_a_SOURCES += core/adiosf_defs_mod.f90 \
                           core/adiosf_read_mod.f90 \
                           query/adios_query_mod.f90

libadiosreadf_a_LIBADD = $(libcoreonce_a_OBJECTS)
libadiosreadf_v1_a_LIBADD = $(libcoreonce_a_OBJECTS)
EXTRA_libadiosreadf_a_DEPENDENCIES = libcoreonce.a 
EXTRA_libadiosreadf_v1_a_DEPENDENCIES = libcoreonce.a 

#nodist_libadiosreadf_a_SOURCES += adios_read_mod.mod
nodist_include_HEADERS += adios_read_mod.mod adios_defs_mod.mod adios_query_mod.mod
CLEANFILES += adios_read_mod.mod adios_defs_mod.mod adios_query_mod.mod

core/adiosf_read_mod.$(OBJEXT): core/adiosf_defs_mod.$(OBJEXT)
adios_read_mod.mod: core/adiosf_read_mod.$(OBJEXT)

query/adios_query_mod.$(OBJEXT): core/adiosf_read_mod.$(OBJEXT) adios_read_mod.mod
adios_query_mod.mod: query/adios_query_mod.$(OBJEXT)

adios_defs_mod.mod: core/adiosf_defs_mod.$(OBJEXT)

endif

lib_LIBRARIES += libadiosread_nompi.a
libadiosread_nompi_a_SOURCES = core/mpidummy.c\
                      core/bp_utils.c \
                      core/adios_read.c \
                      core/adios_read_v1.c \
                      core/common_read.c \
                      core/adios_read_ext.c \
                      $(query_C_SOURCES) \
                      core/adios_read_hooks.c \
                      core/util_mpi.c \
                      read/read_bp.c 

                                          
#if HAVE_DATASPACES
#libadiosread_nompi_a_SOURCES += read/read_dataspaces.c
#endif
#if HAVE_DIMES
#libadiosread_nompi_a_SOURCES += read/read_dimes.c
#endif
#if HAVE_DATATAP
#libadiosread_nompi_a_SOURCES += read/read_datatap.c
#endif
if HAVE_FLEXPATH
libadiosread_nompi_a_SOURCES += read/read_flexpath.c
endif
if HAVE_ICEE
libadiosread_nompi_a_SOURCES += read/read_icee.c
endif
#if HAVE_NSSI
#dist_libadiosread_nompi_a_SOURCES = nssi/adios_nssi_args.x
#nodist_libadiosread_nompi_a_SOURCES = adios_nssi_args.c adios_nssi_args.h
#libadiosread_nompi_a_SOURCES += nssi/adios_nssi_config.c read/read_nssi.c
#libadiosread_nompi_a_CPPFLAGS += $(MACRODEFFLAG)PTL_IFACE_CLIENT=CRAY_USER_NAL $(MACRODEFFLAG)PTL_IFACE_SERVER=CRAY_USER_NAL
#libadiosread_nompi_a_CFLAGS   += $(ADIOSLIB_CFLAGS) 
#endif
libadiosread_nompi_a_CPPFLAGS  += $(MACRODEFFLAG)_NOMPI $(ADIOSLIB_EXTRA_CPPFLAGS) $(ADIOSREADLIB_CPPFLAGS)
libadiosread_nompi_a_LIBADD = $(libcoreonce_a_OBJECTS)
EXTRA_libadiosread_nompi_a_DEPENDENCIES = libcoreonce.a 

# Note: utils/adios_lint needs to link with adios_internals_mxml.o which depends
#       on adios_internals.c, buffer.c and adios_transport_hooks.c. 
#       We put these objects in libadiosread_nompi
#       so we do not need to make another _nompi lib just for adios_lint
if BUILD_FORTRAN
lib_LIBRARIES += libadiosreadf_nompi.a libadiosreadf_nompi_v1.a
FortranReadSeqLibSource = core/mpidummy.c\
                          core/bp_utils.c \
                          core/common_read.c \
                          core/adios_read_ext.c \
                          $(query_F_SOURCES) \
                          core/adios_read_hooks.c \
                          core/util_mpi.c \
                          read/read_bp.c 
#if HAVE_DATASPACES
#FortranReadSeqLibSource += read/read_dataspaces.c
#endif
#if HAVE_DIMES
#FortranReadSeqLibSource += read/read_dimes.c
#endif
#if HAVE_DATATAP
#FortranReadSeqLibSource += read/read_datatap.c
#endif
if HAVE_FLEXPATH
FortranReadSeqLibSource += read/read_flexpath.c
endif
if HAVE_ICEE
FortranReadSeqLibSource += read/read_icee.c
endif
#if HAVE_NSSI
#dist_libadiosreadf_nompi_a_SOURCES = nssi/adios_nssi_args.x
#nodist_libadiosreadf_nompi_a_SOURCES = adios_nssi_args.c adios_nssi_args.h
#FortranReadSeqLibSource += nssi/adios_nssi_config.c read/read_nssi.c
#libadiosreadf_nompi_a_CPPFLAGS += $(MACRODEFFLAG)PTL_IFACE_CLIENT=CRAY_USER_NAL $(MACRODEFFLAG)PTL_IFACE_SERVER=CRAY_USER_NAL
#libadiosreadf_nompi_a_CPPFLAGS += $(ADIOSLIB_EXTRA_CPPFLAGS) $(ADIOSLIB_CPPFLAGS) # $(MACRODEFFLAG)USE_TIMERS
#libadiosreadf_nompi_a_CFLAGS   += $(ADIOSLIB_CFLAGS) 
#endif

libadiosreadf_nompi_a_CPPFLAGS += $(MACRODEFFLAG)_NOMPI $(ADIOSLIB_EXTRA_CPPFLAGS) $(ADIOSREADLIB_CPPFLAGS)
libadiosreadf_nompi_a_SOURCES = $(FortranReadSeqLibSource) core/adiosf_read.c
libadiosreadf_nompi_v1_a_SOURCES = $(FortranReadSeqLibSource) core/adiosf_read_v1.c
libadiosreadf_nompi_v1_a_CPPFLAGS = $(libadiosreadf_nompi_a_CPPFLAGS)
libadiosreadf_nompi_v1_a_CFLAGS = $(libadiosreadf_nompi_a_CFLAGS)

libadiosreadf_nompi_a_SOURCES += core/adiosf_defs_mod.f90 \
                   	         core/adiosf_read_mod.f90 \
                        	 query/adios_query_mod.f90
libadiosreadf_nompi_a_LIBADD = $(libcoreonce_a_OBJECTS)
libadiosreadf_nompi_v1_a_LIBADD = $(libcoreonce_a_OBJECTS)
EXTRA_libadiosreadf_nompi_a_DEPENDENCIES = libcoreonce.a 
EXTRA_libadiosreadf_nompi_v1_a_DEPENDENCIES = libcoreonce.a 
endif  #BUILD_FORTRAN

noinst_LIBRARIES += libadios_internal_nompi.a
libadios_internal_nompi_a_SOURCES = core/mpidummy.c \
                                    core/bp_utils.c \
                                    core/adios_internals.c \
                                    core/util_mpi.c \
                                    $(query_C_SOURCES) \
                                    core/adios_timing.c \
                                    core/adios_transport_hooks.c 
libadios_internal_nompi_a_LIBADD = $(libcoreonce_a_OBJECTS)
if BUILD_WRITE
libadios_internal_nompi_a_SOURCES += core/adios_internals_mxml.c 
if BUILD_MXML
libadios_internal_nompi_a_LIBADD += ${MXML_OBJECTS}
endif
endif
EXTRA_libadios_internal_nompi_a_DEPENDENCIES = libcoreonce.a 


libadios_internal_nompi_a_CPPFLAGS = $(AM_CPPFLAGS) $(MACRODEFFLAG)_INTERNAL $(ADIOSLIB_EXTRA_CPPFLAGS) $(ADIOSLIB_INT_CPPFLAGS) $(ADIOSLIB_CPPFLAGS)
#Note: $(MACRODEFFLAG)NOMPI chokes IBM's bgxlf compiler but it can pass $(MACRODEFFLAG)_NOMPI. 

if HAVE_DMALLOC
libadiosread_a_CPPFLAGS += $(MACRODEFFLAG)DMALLOC 
libadiosread_nompi_a_CPPFLAGS += $(MACRODEFFLAG)DMALLOC 
#if BUILD_FORTRAN
#libadiosreadf_a_CPPFLAGS += $(MACRODEFFLAG)DMALLOC $(DMALLOC_CPPFLAGS)
#libadiosreadf_nompi_a_CPPFLAGS += $(MACRODEFFLAG)DMALLOC $(DMALLOC_CPPFLAGS)
#endif
endif

CC=$(MPICC)
CXX=$(MPICXX)


#bin_PROGRAMS+=adios_flags
#nodist_adios_flags_SOURCES = adios_flags.h 
#adios_flags_SOURCES = adios_flags.c 
#adios_flags.h:  
#        rm -f $@
#        @echo "#define ADIOS_CFLAGS \"$(libadios_a_CPPFLAGS)\"" >> $@
#        @echo "#define ADIOS_LDFLAGS \"$(libadios_a_LDFLAGS)\"" >> $@
#        @echo "#define ADIOSF_CFLAGS \"$(libadiosf_a_CPPFLAGS)\"" >> $@
#        @echo "#define ADIOSF_LDFLAGS \"$(libadiosf_a_LDFLAGS)\"" >> $@
#        @echo "#define ADIOSREAD_CFLAGS \"$(libadiosread_a_CPPFLAGS)\"" >> $@
#        @echo "#define ADIOSREAD_LDFLAGS \"$(libadiosread_a_LDFLAGS)\"" >> $@
#        @echo "#define ADIOSREADF_CFLAGS \"$(libadiosreadf_a_CPPFLAGS)\"" >> $@
#        @echo "#define ADIOSREADF_LDFLAGS \"$(libadiosreadf_a_LDFLAGS)\"" >> $@
#        @echo "#define ADIOSREAD_NOMPI_CFLAGS \"$(libadiosread_nompi_a_CPPFLAGS)\"" >> $@
#        @echo "#define ADIOSREAD_NOMPI_LDFLAGS \"$(libadiosread_nompi_a_LDFLAGS)\"" >> $@
#        @echo "#define ADIOSREADF_NOMPI_CFLAGS \"$(libadiosreadf_nompi_a_CPPFLAGS)\"" >> $@
#        @echo "#define ADIOSREADF_NOMPI_LDFLAGS \"$(libadiosreadf_nompi_a_LDFLAGS)\"" >> $@
#adios_flags.c: adios_flags.h


EXTRA_DIST = core/adios_bp_v1.h core/adios_endianness.h \
             core/adios_internals.h core/adios_internals_mxml.h core/adios_logger.h \
             core/adios_read_hooks.h core/adios_socket.h core/adios_timing.h \
             core/adios_icee.h core/a2sel.h core/adios_clock.h \
             core/adios_socket.h core/adios_transport_hooks.h \
             core/bp_types.h core/bp_utils.h core/buffer.h core/common_adios.h \
             core/common_read.h core/adios_infocache.h core/futils.h core/globals.h core/ds_metadata.h \
             core/types.h core/util.h core/strutil.h core/flexpath.h core/qhashtbl.h \
             public/adios_version.h.in core/util_mpi.h \
             $(transforms_common_HDRS) $(transforms_read_HDRS) $(transforms_write_HDRS) \
             $(query_common_HDRS) $(query_method_HDRS) \
             transforms/transform_plugins.h \
             transforms/adios_transform_identity_read.h \
             transforms/adios_transform_szip.h \
             transforms/adios_transform_alacrity_common.h \
             transforms/adios_transform_template_read.c \
             transforms/adios_transform_template_write.c \
             query/Makefile.plugins.cmake 
#             nssi/adios_nssi_config.h nssi/aggregation.h nssi/io_timer.h 

