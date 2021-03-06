// Copyright (C) 2013 PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#pragma once

#include "Common/CommonTypes.h"

// Broken out of MemMap.h to avoid a bad include dependency.

namespace Memory {

struct Opcode {
    Opcode() {
    }

    explicit Opcode(u32 v) : encoding(v) {
    }

    u32 operator & (const u32 &arg) const {
        return encoding & arg;
    }

    u32 operator >> (const u32 &arg) const {
        return encoding >> arg;
    }

    bool operator == (const u32 &arg) const {
        return encoding == arg;
    }

    bool operator != (const u32 &arg) const {
        return encoding != arg;
    }

    u32 encoding;
};

}
