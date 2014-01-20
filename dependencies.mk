# Rocket2d dependencies
LOCAL_LIBNAME 		:= rocket
LOCAL_LIBVERSION	:= 0.0.1
include $(BUILD_ROOT)/common/repo_dependency.mk

# Box2d dependencies
LOCAL_LIBNAME 		:= Box2D
LOCAL_LIBVERSION	:= 2.2.1
include $(BUILD_ROOT)/common/repo_dependency.mk

# cppjson
LOCAL_LIBNAME		:= cppjson
LOCAL_LIBVERSION	:= 20110525
include $(BUILD_ROOT)/common/repo_dependency.mk