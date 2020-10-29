// Copyright (c) 2012, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// linux_ptrace_dumper.cc: Implement google_breakpad::LinuxPtraceDumper.
// See linux_ptrace_dumper.h for detals.
// This class was originally splitted from google_breakpad::LinuxDumper.

// This code deals with the mechanics of getting information about a crashed
// process. Since this code may run in a compromised address space, the same
// rules apply as detailed at the top of minidump_writer.h: no libc calls and
// use the alternative allocator.

#include "client/linux/minidump_writer/linux_ptrace_dumper.h"

#include <asm/ptrace.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#if defined(__i386)
#include <cpuid.h>
#endif

#include "client/linux/minidump_writer/directory_reader.h"
#include "client/linux/minidump_writer/line_reader.h"
#include "common/linux/linux_libc_support.h"
#include "third_party/lss/linux_syscall_support.h"

// Suspends a thread by attaching to it.
static bool SuspendThread(pid_t pid) {
  // This may fail if the thread has just died or debugged.
  errno = 0;
  if (sys_ptrace(PTRACE_ATTACH, pid, NULL, NULL) != 0 &&
      errno != 0) {
    return false;
  }
  while (sys_waitpid(pid, NULL, __WALL) < 0) {
    if (errno != EINTR) {
      sys_ptrace(PTRACE_DETACH, pid, NULL, NULL);
      return false;
    }
  }
#if defined(__i386) || defined(__x86_64)
  // On x86, the stack pointer is NULL or -1, when executing trusted code in
  // the seccomp sandbox. Not only does this cause difficulties down the line
  // when trying to dump the thread's stack, it also results in the minidumps
  // containing information about the trusted threads. This information is
  // generally completely meaningless and just pollutes the minidumps.
  // We thus test the stack pointer and exclude any threads that are part of
  // the seccomp sandbox's trusted code.
  user_regs_struct regs;
  if (sys_ptrace(PTRACE_GETREGS, pid, NULL, &regs) == -1 ||
#if defined(__i386)
      !regs.esp
#elif defined(__x86_64)
      !regs.rsp
#endif
      ) {
    sys_ptrace(PTRACE_DETACH, pid, NULL, NULL);
    return false;
  }
#endif
  return true;
}

// Resumes a thread by detaching from it.
static bool ResumeThread(pid_t pid) {
  return sys_ptrace(PTRACE_DETACH, pid, NULL, NULL) >= 0;
}

namespace google_breakpad {

LinuxPtraceDumper::LinuxPtraceDumper(pid_t pid)
    : LinuxDumper(pid),
      threads_suspended_(false) {
}

bool LinuxPtraceDumper::BuildProcPath(char* path, pid_t pid,
                                      const char* node) const {
  if (!path || !node || pid <= 0)
    return false;

  size_t node_len = my_strlen(node);
  if (node_len == 0)
    return false;

  const unsigned pid_len = my_uint_len(pid);
  const size_t total_length = 6 + pid_len + 1 + node_len;
  if (total_length >= NAME_MAX)
    return false;

  my_memcpy(path, "/proc/", 6);
  my_uitos(path + 6, pid, pid_len);
  path[6 + pid_len] = '/';
  my_memcpy(path + 6 + pid_len + 1, node, node_len);
  path[total_length] = '\0';
  return true;
}

void LinuxPtraceDumper::CopyFromProcess(void* dest, pid_t child,
                                        const void* src, size_t length) {
  unsigned long tmp = 55;
  size_t done = 0;
  static const size_t word_size = sizeof(tmp);
  uint8_t* const local = (uint8_t*) dest;
  uint8_t* const remote = (uint8_t*) src;

  while (done < length) {
    const size_t l = (length - done > word_size) ? word_size : (length - done);
    if (sys_ptrace(PTRACE_PEEKDATA, child, remote + done, &tmp) == -1) {
      tmp = 0;
    }
    my_memcpy(local + done, &tmp, l);
    done += l;
  }
}

// Read thread info from /proc/$pid/status.
// Fill out the |tgid|, |ppid| and |pid| members of |info|. If unavailable,
// these members are set to -1. Returns true iff all three members are
// available.
bool LinuxPtraceDumper::GetThreadInfoByIndex(size_t index, ThreadInfo* info) {
  if (index >= threads_.size())
    return false;

  pid_t tid = threads_[index];

  assert(info != NULL);
  char status_path[NAME_MAX];
  if (!BuildProcPath(status_path, tid, "status"))
    return false;

  const int fd = sys_open(status_path, O_RDONLY, 0);
  if (fd < 0)
    return false;

  LineReader* const line_reader = new(allocator_) LineReader(fd);
  const char* line;
  unsigned line_len;

  info->ppid = info->tgid = -1;

  while (line_reader->GetNextLine(&line, &line_len)) {
    if (my_strncmp("Tgid:\t", line, 6) == 0) {
      my_strtoui(&info->tgid, line + 6);
    } else if (my_strncmp("PPid:\t", line, 6) == 0) {
      my_strtoui(&info->ppid, line + 6);
    }

    line_reader->PopLine(line_len);
  }
  sys_close(fd);

  if (info->ppid == -1 || info->tgid == -1)
    return false;

  if (sys_ptrace(PTRACE_GETREGS, tid, NULL, &info->regs) == -1) {
    return false;
  }

  if (sys_ptrace(PTRACE_GETFPREGS, tid, NULL, &info->fpregs) == -1) {
    return false;
  }

#if defined(__i386)
#if !defined(bit_FXSAVE)  // e.g. Clang
#define bit_FXSAVE bit_FXSR
#endif
  // Detect if the CPU supports the FXSAVE/FXRSTOR instructions
  int eax, ebx, ecx, edx;
  __cpuid(1, eax, ebx, ecx, edx);
  if (edx & bit_FXSAVE) {
    if (sys_ptrace(PTRACE_GETFPXREGS, tid, NULL, &info->fpxregs) == -1) {
      return false;
    }
  } else {
    memset(&info->fpxregs, 0, sizeof(info->fpxregs));
  }
#endif  // defined(__i386)

#if defined(__i386) || defined(__x86_64)
  for (unsigned i = 0; i < ThreadInfo::kNumDebugRegisters; ++i) {
    if (sys_ptrace(
        PTRACE_PEEKUSER, tid,
        reinterpret_cast<void*> (offsetof(struct user,
                                          u_debugreg[0]) + i *
                                 sizeof(debugreg_t)),
        &info->dregs[i]) == -1) {
      return false;
    }
  }
#endif

#if defined(__mips__)
  for (int i = 0; i < 3; ++i) {
    sys_ptrace(PTRACE_PEEKUSER, tid,
               reinterpret_cast<void*>(DSP_BASE + (i * 2)), &info->hi[i]);
    sys_ptrace(PTRACE_PEEKUSER, tid,
               reinterpret_cast<void*>(DSP_BASE + (i * 2) + 1), &info->lo[i]);
  }
  sys_ptrace(PTRACE_PEEKUSER, tid,
             reinterpret_cast<void*>(DSP_CONTROL), &info->dsp_control);
#endif

  const uint8_t* stack_pointer;
#if defined(__i386)
  my_memcpy(&stack_pointer, &info->regs.esp, sizeof(info->regs.esp));
#elif defined(__x86_64)
  my_memcpy(&stack_pointer, &info->regs.rsp, sizeof(info->regs.rsp));
#elif defined(__ARM_EABI__)
  my_memcpy(&stack_pointer, &info->regs.ARM_sp, sizeof(info->regs.ARM_sp));
#elif defined(__mips__)
  stack_pointer =
      reinterpret_cast<uint8_t*>(info->regs.regs[MD_CONTEXT_MIPS_REG_SP]);
#else
#error "This code hasn't been ported to your platform yet."
#endif
  info->stack_pointer = reinterpret_cast<uintptr_t>(stack_pointer);

  return true;
}

bool LinuxPtraceDumper::IsPostMortem() const {
  return false;
}

bool LinuxPtraceDumper::ThreadsSuspend() {
  if (threads_suspended_)
    return true;
  for (size_t i = 0; i < threads_.size(); ++i) {
    if (!SuspendThread(threads_[i])) {
      // If the thread either disappeared before we could attach to it, or if
      // it was part of the seccomp sandbox's trusted code, it is OK to
      // silently drop it from the minidump.
      my_memmove(&threads_[i], &threads_[i+1],
                 (threads_.size() - i - 1) * sizeof(threads_[i]));
      threads_.resize(threads_.size() - 1);
      --i;
    }
  }
  threads_suspended_ = true;
  return threads_.size() > 0;
}

bool LinuxPtraceDumper::ThreadsResume() {
  if (!threads_suspended_)
    return false;
  bool good = true;
  for (size_t i = 0; i < threads_.size(); ++i)
    good &= ResumeThread(threads_[i]);
  threads_suspended_ = false;
  return good;
}

// Parse /proc/$pid/task to list all the threads of the process identified by
// pid.
bool LinuxPtraceDumper::EnumerateThreads() {
  char task_path[NAME_MAX];
  if (!BuildProcPath(task_path, pid_, "task"))
    return false;

  const int fd = sys_open(task_path, O_RDONLY | O_DIRECTORY, 0);
  if (fd < 0)
    return false;
  DirectoryReader* dir_reader = new(allocator_) DirectoryReader(fd);

  // The directory may contain duplicate entries which we filter by assuming
  // that they are consecutive.
  int last_tid = -1;
  const char* dent_name;
  while (dir_reader->GetNextEntry(&dent_name)) {
    if (my_strcmp(dent_name, ".") &&
        my_strcmp(dent_name, "..")) {
      int tid = 0;
      if (my_strtoui(&tid, dent_name) &&
          last_tid != tid) {
        last_tid = tid;
        threads_.push_back(tid);
      }
    }
    dir_reader->PopEntry();
  }

  sys_close(fd);
  return true;
}

}  // namespace google_breakpad
