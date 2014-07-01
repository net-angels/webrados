#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc -Wall  -zmuldefs -g -ggdb -O0
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1776471976/config.o \
	${OBJECTDIR}/_ext/1776471976/logs.o \
	${OBJECTDIR}/_ext/1776471976/memory.o \
	${OBJECTDIR}/_ext/1776471976/meta.o \
	${OBJECTDIR}/_ext/1776471976/method.o \
	${OBJECTDIR}/_ext/1776471976/signal.o \
	${OBJECTDIR}/utceph.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=/usr/lib/libfcgi.so /usr/lib/librados.so `pkg-config --libs libconfig` -lpthread   

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk /var/lib/apache2/fastcgi/fastupload.fcgi

/var/lib/apache2/fastcgi/fastupload.fcgi: /usr/lib/libfcgi.so

/var/lib/apache2/fastcgi/fastupload.fcgi: /usr/lib/librados.so

/var/lib/apache2/fastcgi/fastupload.fcgi: ${OBJECTFILES}
	${MKDIR} -p /var/lib/apache2/fastcgi
	${LINK.c} -o /var/lib/apache2/fastcgi/fastupload.fcgi ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/1776471976/config.o: nbproject/Makefile-${CND_CONF}.mk /home/freeman/NetBeansProjects/radosweb/config.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1776471976
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/include/rados `pkg-config --cflags libconfig`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1776471976/config.o /home/freeman/NetBeansProjects/radosweb/config.c

${OBJECTDIR}/_ext/1776471976/logs.o: nbproject/Makefile-${CND_CONF}.mk /home/freeman/NetBeansProjects/radosweb/logs.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1776471976
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/include/rados `pkg-config --cflags libconfig`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1776471976/logs.o /home/freeman/NetBeansProjects/radosweb/logs.c

${OBJECTDIR}/_ext/1776471976/memory.o: nbproject/Makefile-${CND_CONF}.mk /home/freeman/NetBeansProjects/radosweb/memory.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1776471976
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/include/rados `pkg-config --cflags libconfig`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1776471976/memory.o /home/freeman/NetBeansProjects/radosweb/memory.c

${OBJECTDIR}/_ext/1776471976/meta.o: nbproject/Makefile-${CND_CONF}.mk /home/freeman/NetBeansProjects/radosweb/meta.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1776471976
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/include/rados `pkg-config --cflags libconfig`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1776471976/meta.o /home/freeman/NetBeansProjects/radosweb/meta.c

${OBJECTDIR}/_ext/1776471976/method.o: nbproject/Makefile-${CND_CONF}.mk /home/freeman/NetBeansProjects/radosweb/method.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1776471976
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/include/rados `pkg-config --cflags libconfig`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1776471976/method.o /home/freeman/NetBeansProjects/radosweb/method.c

${OBJECTDIR}/_ext/1776471976/signal.o: nbproject/Makefile-${CND_CONF}.mk /home/freeman/NetBeansProjects/radosweb/signal.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1776471976
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/include/rados `pkg-config --cflags libconfig`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1776471976/signal.o /home/freeman/NetBeansProjects/radosweb/signal.c

${OBJECTDIR}/utceph.o: nbproject/Makefile-${CND_CONF}.mk utceph.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/include/rados `pkg-config --cflags libconfig`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/utceph.o utceph.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} /var/lib/apache2/fastcgi/fastupload.fcgi

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
