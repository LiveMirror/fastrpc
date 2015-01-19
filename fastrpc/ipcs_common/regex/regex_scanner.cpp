// Copyright (c) 2005, Google Inc.
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
//
// Author: Sanjay Ghemawat

#include "regex_scanner.h"
#include <vector>
#include <assert.h>

#include "regex_internal.h"
#include "config.h"

using std::vector;

// namespace common {

RegexScaner::RegexScaner()
    : m_data(),
    m_input(m_data),
    m_skip(NULL),
    m_should_skip(false),
    m_skip_repeat(false),
    m_save_comments(false),
    m_comments(NULL),
    m_comments_offset(0)
{
}

RegexScaner::RegexScaner(const std::string& in)
    : m_data(in),
    m_input(m_data),
    m_skip(NULL),
    m_should_skip(false),
    m_skip_repeat(false),
    m_save_comments(false),
    m_comments(NULL),
    m_comments_offset(0)
{
}

RegexScaner::~RegexScaner() {
    delete m_skip;
    delete m_comments;
}

void RegexScaner::SetSkipExpression(const char* re) {
    delete m_skip;
    if (re != NULL) {
        m_skip = new Regex(re);
        m_should_skip = true;
        m_skip_repeat = true;
        ConsumeSkip();
    } else {
        m_skip = NULL;
        m_should_skip = false;
        m_skip_repeat = false;
    }
}

void RegexScaner::Skip(const char* re) {
    delete m_skip;
    if (re != NULL) {
        m_skip = new Regex(re);
        m_should_skip = true;
        m_skip_repeat = false;
        ConsumeSkip();
    } else {
        m_skip = NULL;
        m_should_skip = false;
        m_skip_repeat = false;
    }
}

void RegexScaner::DisableSkip() {
    assert(m_skip != NULL);
    m_should_skip = false;
}

void RegexScaner::EnableSkip() {
    assert(m_skip != NULL);
    m_should_skip = true;
    ConsumeSkip();
}

int RegexScaner::LineNumber() const {
    // TODO: Make it more efficient by keeping track of the last point
    // where we computed line numbers and counting newlines since then.
    // We could use std:count, but not all systems have it. :-(
    int count = 1;
    for (const char* p = m_data.data(); p < m_input.data(); ++p)
        if (*p == '\n')
            ++count;
    return count;
}

int RegexScaner::Offset() const {
    return (int)(m_input.data() - m_data.c_str());
}

bool RegexScaner::LookingAt(const Regex& re) const {
    int consumed;
    return re.DoMatch(m_input, Regex::ANCHOR_START, &consumed, 0, 0);
}


bool RegexScaner::Consume(const Regex& re,
                          const ValueParser& capture_ptr0,
                          const ValueParser& capture_ptr1,
                          const ValueParser& capture_ptr2,
                          const ValueParser& capture_ptr3,
                          const ValueParser& capture_ptr4,
                          const ValueParser& capture_ptr5,
                          const ValueParser& capture_ptr6,
                          const ValueParser& capture_ptr7,
                          const ValueParser& capture_ptr8,
                          const ValueParser& capture_ptr9,
                          const ValueParser& capture_ptr10,
                          const ValueParser& capture_ptr11,
                          const ValueParser& capture_ptr12,
                          const ValueParser& capture_ptr13,
                          const ValueParser& capture_ptr14,
                          const ValueParser& capture_ptr15
) {
    const bool result = re.Consume(
        &m_input,
        capture_ptr0, capture_ptr1, capture_ptr2, capture_ptr3, capture_ptr4,
        capture_ptr5, capture_ptr6, capture_ptr7, capture_ptr8, capture_ptr9,
        capture_ptr10, capture_ptr11, capture_ptr12, capture_ptr13,
        capture_ptr14, capture_ptr15
    );
    if (result && m_should_skip) ConsumeSkip();
    return result;
}

// helper function to consume *m_skip and honour m_save_comments
void RegexScaner::ConsumeSkip() {
    const char* start_data = m_input.data();
    while (m_skip->Consume(&m_input)) {
        if (!m_skip_repeat) {
            // Only one skip allowed.
            break;
        }
    }
    if (m_save_comments) {
        if (m_comments == NULL) {
            m_comments = new vector<StringPiece>;
        }
        // already pointing one past end, so no need to +1
        int length = (int)(m_input.data() - start_data);
        if (length > 0) {
            m_comments->push_back(StringPiece(start_data, length));
        }
    }
}


void RegexScaner::GetComments(int start, int end, vector<StringPiece> *ranges) {
    // short circuit out if we've not yet initialized m_comments
    // (e.g., when save_comments is false)
    if (!m_comments) {
        return;
    }
    // TODO: if we guarantee that m_comments will contain StringPieces
    // that are ordered by their start, then we can do a binary search
    // for the first StringPiece at or past start and then scan for the
    // ones contained in the range, quit early (use equal_range or
    // lower_bound)
    for (vector<StringPiece>::const_iterator it = m_comments->begin();
         it != m_comments->end(); ++it) {
        if ((it->data() >= m_data.c_str() + start &&
             it->data() + it->size() <= m_data.c_str() + end)) {
            ranges->push_back(*it);
        }
    }
}


void RegexScaner::GetNextComments(vector<StringPiece> *ranges) {
    // short circuit out if we've not yet initialized m_comments
    // (e.g., when save_comments is false)
    if (!m_comments) {
        return;
    }
    for (vector<StringPiece>::const_iterator it =
         m_comments->begin() + m_comments_offset;
         it != m_comments->end(); ++it) {
        ranges->push_back(*it);
        ++m_comments_offset;
    }
}

// } // namespace common
