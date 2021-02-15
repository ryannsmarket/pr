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

#ifndef CRASHPAD_UTIL_LINUX_EXCEPTION_INFORMATION_H_
#define CRASHPAD_UTIL_LINUX_EXCEPTION_INFORMATION_H_

#include <sys/types.h>

#include "util/linux/address_types.h"

namespace crashpad {

#pragma pack(push, 1)

//! \brief Structure read out of the client process by the crash handler when an
//!     exception occurs.
struct ExceptionInformation {
  //! \brief The address of the `siginfo_t` passed to the signal handler in the
  //!     crashed process.
  LinuxVMAddress siginfo_address;

  //! \brief The address of the `ucontext_t` passed to the signal handler in the
  //!     crashed process.
  LinuxVMAddress context_address;

  //! \brief The thread ID of the thread which received the signal.
  pid_t thread_id;
};

#pragma pack(pop)

}  // namespace crashpad

#endif  // CRASHPAD_UTIL_LINUX_EXCEPTION_INFORMATION_H_
