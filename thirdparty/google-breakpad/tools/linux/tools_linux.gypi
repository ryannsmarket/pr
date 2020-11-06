# Copyright 2014 Google Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

{
  'target_defaults': {
    'include_dirs': [
      '../..',
    ],
  },
  'targets': [
    {
      'target_name': 'dump_syms',
      'type': 'executable',
      'sources': [
        'dump_syms/dump_syms.cc',
      ],
      'dependencies': [
        '../common/common.gyp:common',
      ],
    },
    {
      'target_name': 'md2core',
      'type': 'executable',
      'sources': [
        'md2core/minidump-2-core.cc',
        'md2core/minidump_memory_range.h',
      ],
      'dependencies': [
        '../common/common.gyp:common',
      ],
    },
    {
      'target_name': 'minidump_upload',
      'type': 'executable',
      'sources': [
        'symupload/minidump_upload.cc',
      ],
      'dependencies': [
        '../common/common.gyp:common',
      ],
    },
    {
      'target_name': 'symupload',
      'type': 'executable',
      'sources': [
        'symupload/sym_upload.cc',
      ],
      'link_settings': {
        'libraries': [
          '-ldl',
        ],
      },
      'dependencies': [
        '../common/common.gyp:common',
      ],
    },
  ],
}
