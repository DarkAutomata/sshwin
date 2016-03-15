# Copyright (c) 2016, Jonathan Ward
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
# 
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# 
# Setup all build definitions before including the project's generic
# make file.
#
CLIENT_TARGET=$(BUILD_OUT)\ssh.exe
CLIENT_PDB=$(BUILD_OUT)\ssh.pdb

BUILD_DEPENDS=\

LOCAL_LIBS=\

COMMON_BUILD_OBJS=\
	$(BUILD_OUT)\sshtransport.obj\

CLIENT_BUILD_OBJS=\
	$(COMMON_BUILD_OBJS)\
	$(BUILD_OUT)\clientmain.obj\

#
# Include the generic makefile to pull in global build rules, like .C
# compilation, etc.
#
!INCLUDE makefile.def

#
# Define build targets to be referenced by the global build rules, or
# for custom targets (like clean).
#
build: $(CLIENT_TARGET)

$(CLIENT_TARGET): $(CLIENT_BUILD_OBJS)
	link.exe /nologo /out:$(CLIENT_TARGET) $(CLIENT_BUILD_OBJS) $(LN_LIBS) /DEBUG /PDB:$(CLIENT_PDB) $(LOCAL_LIBS)

clean:
	-@rmdir /s /q out > nul 2>&1
	-@del /s /q *~ > nul 2>&1

