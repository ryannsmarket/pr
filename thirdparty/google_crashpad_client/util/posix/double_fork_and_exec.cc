// Copyright 2017 The Crashpad Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "util/posix/double_fork_and_exec.h"

#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "base/check_op.h"
#include "base/logging.h"
#include "base/posix/eintr_wrapper.h"
#include "base/strings/stringprintf.h"
#include "util/posix/close_multiple.h"

namespace crashpad {

bool DoubleForkAndExec(const std::vector<std::string>& argv,
                       const std::vector<std::string>* envp,
                       int preserve_fd,
                       bool use_path,
                       void (*child_function)()) {
  DCHECK(!envp || !use_path);

  // argv_c contains const char* pointers and is terminated by nullptr. This is
  // suitable for passing to execv(). Although argv_c is not used in the parent
  // process, it must be built in the parent process because it’s unsafe to do
  // so in the child or grandchild process.
  std::vector<const char*> argv_c;
  argv_c.reserve(argv.size() + 1);
  for (const std::string& argument : argv) {
    argv_c.push_back(argument.c_str());
  }
  argv_c.push_back(nullptr);

  std::vector<const char*> envp_c;
  if (envp) {
    envp_c.reserve(envp->size() + 1);
    for (const std::string& variable : *envp) {
      envp_c.push_back(variable.c_str());
    }
    envp_c.push_back(nullptr);
  }

  // Double-fork(). The three processes involved are parent, child, and
  // grandchild. The grandchild will call execv(). The child exits immediately
  // after spawning the grandchild, so the grandchild becomes an orphan and its
  // parent process ID becomes 1. This relieves the parent and child of the
  // responsibility to reap the grandchild with waitpid() or similar. The
  // grandchild is expected to outlive the parent process, so the parent
  // shouldn’t be concerned with reaping it. This approach means that accidental
  // early termination of the handler process will not result in a zombie
  // process.
  pid_t pid = fork();
  if (pid < 0) {
    PLOG(ERROR) << "fork";
    return false;
  }

  if (pid == 0) {
    // Child process.

    if (child_function) {
      child_function();
    }

    // Call setsid(), creating a new process group and a new session, both led
    // by this process. The new process group has no controlling terminal. This
    // disconnects it from signals generated by the parent process’ terminal.
    //
    // setsid() is done in the child instead of the grandchild so that the
    // grandchild will not be a session leader. If it were a session leader, an
    // accidental open() of a terminal device without O_NOCTTY would make that
    // terminal the controlling terminal.
    //
    // It’s not desirable for the grandchild to have a controlling terminal. The
    // grandchild manages its own lifetime, such as by monitoring clients on its
    // own and exiting when it loses all clients and when it deems it
    // appropraite to do so. It may serve clients in different process groups or
    // sessions than its original client, and receiving signals intended for its
    // original client’s process group could be harmful in that case.
    PCHECK(setsid() != -1) << "setsid";

    pid = fork();
    if (pid < 0) {
      PLOG(FATAL) << "fork";
    }

    if (pid > 0) {
      // Child process.

      // _exit() instead of exit(), because fork() was called.
      _exit(EXIT_SUCCESS);
    }

    // Grandchild process.

    CloseMultipleNowOrOnExec(STDERR_FILENO + 1, preserve_fd);

    // &argv_c[0] is a pointer to a pointer to const char data, but because of
    // how C (not C++) works, execvp() wants a pointer to a const pointer to
    // char data. It modifies neither the data nor the pointers, so the
    // const_cast is safe.
    char* const* argv_for_execv = const_cast<char* const*>(&argv_c[0]);

    if (envp) {
      // This cast is safe for the same reason that the argv_for_execv cast is.
      char* const* envp_for_execv = const_cast<char* const*>(&envp_c[0]);
      execve(argv_for_execv[0], argv_for_execv, envp_for_execv);
      PLOG(FATAL) << "execve " << argv_for_execv[0];
    }

    if (use_path) {
      execvp(argv_for_execv[0], argv_for_execv);
      PLOG(FATAL) << "execvp " << argv_for_execv[0];
    }

    execv(argv_for_execv[0], argv_for_execv);
    PLOG(FATAL) << "execv " << argv_for_execv[0];
  }

  // waitpid() for the child, so that it does not become a zombie process. The
  // child normally exits quickly.
  //
  // Failures from this point on may result in the accumulation of a zombie, but
  // should not be considered fatal. Log only warnings, but don’t treat these
  // failures as a failure of the function overall.
  int status;
  pid_t wait_pid = HANDLE_EINTR(waitpid(pid, &status, 0));
  if (wait_pid == -1) {
    PLOG(WARNING) << "waitpid";
    return true;
  }
  DCHECK_EQ(wait_pid, pid);

  if (WIFSIGNALED(status)) {
    int sig = WTERMSIG(status);
    LOG(WARNING) << base::StringPrintf(
        "intermediate process terminated by signal %d (%s)%s",
        sig,
        strsignal(sig),
        WCOREDUMP(status) ? " (core dumped)" : "");
  } else if (!WIFEXITED(status)) {
    LOG(WARNING) << base::StringPrintf(
        "intermediate process: unknown termination 0x%x", status);
  } else if (WEXITSTATUS(status) != EXIT_SUCCESS) {
    LOG(WARNING) << "intermediate process exited with code "
                 << WEXITSTATUS(status);
  }

  return true;
}

}  // namespace crashpad
