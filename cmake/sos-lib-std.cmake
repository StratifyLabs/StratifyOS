
if(SOS_ARCH_ARM_ALL OR SOS_ARCH_ARM_V7M)
set(SOS_ARCH v7m)
include(sos-lib)
endif()

if(SOS_ARCH_ARM_ALL OR SOS_ARCH_ARM_V7EM)
set(SOS_ARCH v7em)
include(sos-lib)
endif()

#set(SOS_ARCH v7em_f4ss)
#include(sos-lib)

if(SOS_ARCH_ARM_ALL OR SOS_ARCH_ARM_V7EM_F4SH)
set(SOS_ARCH v7em_f4sh)
include(sos-lib)
endif()

#set(SOS_ARCH v7em_f5ss)
#include(sos-lib)


if(SOS_ARCH_ARM_ALL OR SOS_ARCH_ARM_V7EM_F5SH)
set(SOS_ARCH v7em_f5sh)
include(sos-lib)
endif()

#set(SOS_ARCH v7em_f5ds)
#include(sos-lib)

if(SOS_ARCH_ARM_ALL OR SOS_ARCH_ARM_V7EM_F5DH)
set(SOS_ARCH v7em_f5dh)
include(sos-lib)
endif()
