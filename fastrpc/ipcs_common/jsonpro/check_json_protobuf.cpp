/*

Copyright (c) 2013, Coolplay (pengchong)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

-- Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

-- Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "json_protobuf.h"

#include <stdexcept>
#include <memory>
#include <cstdlib>

#include "check_protobuf_types.pb.h"

using namespace ipcs_common;

JsonPro g_jsonpro;

namespace {

/**
 * Abort with an error message.
 *
 * @param error_message error message to print before aborting
 */
void
fail(const std::string& error_message)
{
    std::cerr << "Test failed: " << error_message << std::endl;
    std::abort();
}

/**
 * Abort with an error message unless <code>test</code> is <code>true</code>.
 *
 * @param test result of a test condition
 * @param error_message error message to print before aborting
 */
void
fail_unless(bool test, const std::string& error_message)
{
    if (!test) {
        fail(error_message);
    }
}

/**
 * Abort with an error message if <code>test</code> is <code>true</code>.
 *
 * @param test result of a test condition
 * @param error_message error message to print before aborting
 */
void
fail_if(bool test, const std::string& error_message)
{
    if (test) {
        fail(error_message);
    }
}

/**
 * Determine if two protobuf messages are equal.
 *
 * <P>NOTE: protobuf objects don't have an equals() method, so we serialize
 * them to strings and compare the strings.</P>
 *
 * @param m1 first message to check
 * @param m2 second message to check
 */
bool
protobuf_messages_equal(const google::protobuf::Message& m1,
    const google::protobuf::Message& m2)
{
    std::string m1_str;
    fail_unless(m1.SerializeToString(&m1_str),
        "could not serialize first protobuf message to string");

    std::string m2_str;
    fail_unless (m2.SerializeToString(&m2_str),
        "could not serialize second protobuf message to string");

    return (m1_str == m2_str);
}

/**
 * Copy a protobuf message by converting it to JSON object and then converting
 * the JSON object back to a second protobuf message.
 *
 * @param m1 message to copy
 * @param m2 copy of <code>m1</code>
 * @param use_string if <code>true</code> we use string for the intermediate
 * JSON value, otherwise we use a Json::Value object
 */
void
copy_message_with_json(const google::protobuf::Message& m1,
    google::protobuf::Message& m2, bool use_string)
{
    if (use_string) {
        std::string m1_json;
        g_jsonpro.ConvToJson(m1, m1_json);

        m2.Clear();
        g_jsonpro.ConvToProto(m1_json, m2);

    } else {
        Json::Value m1_json;
        g_jsonpro.ConvToJson(m1, m1_json);

        m2.Clear();
        g_jsonpro.ConvToProto(m1_json, m2);
    }
}

/**
 * Convert a protobuf message to JSON, convert the JSON back to a protobuf
 * message, and make sure that the two protobuf messages are equal.
 *
 * @param message the protobuf message to use for testing
 * @param use_string if <code>true</code> we use string for the intermediate
 * JSON value, otherwise we use a Json::Value object
 */
void
do_json_protobuf_check(const google::protobuf::Message& message,
    bool use_string)
{
    std::auto_ptr<google::protobuf::Message> message2(message.New());
    copy_message_with_json(message, *(message2.get()), use_string);
    fail_unless(protobuf_messages_equal(message, *(message2.get())),
        "messages were not equal");
}

/**
 * Conversion test 1.
 *
 * @param use_string if <code>true</code> we use string for the intermediate
 * JSON value, otherwise we use a Json::Value object
 */
void
test1(bool use_string)
{
    /*
     * Create object for testing.
     */
    json_protobuf::test::employee employee;
    employee.set_id(12345);
    employee.set_first_name("John");
    employee.set_last_name("Smith");
    employee.set_date_of_birth("1/2/1970");
    employee.set_pay_type(json_protobuf::test::employee::salaried);
    employee.set_salary(1);
    employee.set_active(true);
    employee.add_departments("dept1");
    employee.add_departments("dept4");
    employee.add_departments("dept9");
    employee.set_hire_date(1365109914);

    /*
     * Do full protobuf->JSON->protobuf testing.
     */
    do_json_protobuf_check(employee, use_string);

    /*
     * Change a single entry and make sure that objects differ.
     */
    json_protobuf::test::employee employee2;
    copy_message_with_json(employee, employee2, use_string);
    fail_unless(protobuf_messages_equal(employee, employee2),
        "messages should be equal");
    employee.set_salary(2);
    fail_if(protobuf_messages_equal(employee, employee2),
        "messages should not be equal");

    /**
     * Clear some optional fields and check again.
     */
    employee.clear_hire_date();
    employee.clear_active();
    employee.clear_departments();

    do_json_protobuf_check(employee, use_string);

    copy_message_with_json(employee, employee2, use_string);
    fail_unless(protobuf_messages_equal(employee, employee2),
        "messages should be equal");
    employee2.clear_salary();
    fail_if(protobuf_messages_equal(employee, employee2),
        "messages should not be equal");
}

/**
 * Conversion test 2.
 *
 * @param use_string if <code>true</code> we use string for the intermediate
 * JSON value, otherwise we use a Json::Value object
 */
void
test2(bool use_string)
{
    /*
     * Create object for testing.
     */
    json_protobuf::test::C c;
    c.set_a(1);
    c.set_b(2);
    c.set_c(3);
    c.mutable_a_message()->set_a(4);
    c.mutable_b_message()->set_a(5);
    c.mutable_b_message()->set_b(6);
    c.mutable_b_message()->mutable_a_message()->set_a(7);

    /*
     * Do full protobuf->JSON->protobuf testing.
     */
    do_json_protobuf_check(c, use_string);

    /*
     * Change a single entry and make sure that objects differ.
     */
    json_protobuf::test::C c2;
    copy_message_with_json(c, c2, use_string);
    fail_unless(protobuf_messages_equal(c, c2),
        "messages should be equal");
    c.mutable_b_message()->mutable_a_message()->set_a(8);
    fail_if(protobuf_messages_equal(c, c2), "messages should not be equal");

}

/**
 * Testing raw text.
 */
void
test3()
{
    const std::string message = "{\"active\":true,\"date_of_birth\":\"1/2/1970\",\"departments\":[\"dept1\",\"dept4\",\"dept9\"],\"first_name\":\"John\",\"hire_date\":1365109914,\"id\":12345,\"last_name\":\"Smith\",\"pay_type\":\"salaried\",\"salary\":1}\n";

    json_protobuf::test::employee employee;
    g_jsonpro.ConvToProto(message, employee);
    std::cout << std::endl;
    std::cout << employee.ShortDebugString();

    fail_unless(employee.id() == 12345, "incorrect id");
    fail_unless(employee.first_name() == "John", "incorrect first name");
    fail_unless(employee.last_name() == "Smith", "incorrect last name");
    fail_unless(employee.date_of_birth() == "1/2/1970",
        "incorrect birth date");
    fail_unless(employee.pay_type() ==
        json_protobuf::test::employee::salaried, "incorrect pay type");
    fail_unless(employee.salary() == 1, "incorrect salary");
    fail_unless(employee.active(), "should be active");
    fail_unless(employee.hire_date() == 1365109914, "incorrect hire data");

    // NOTE: This relies on JsonCpp FastWriter returning JSON text in the
    // same format as that used to initial the message variable (this test
    // may not be true in future versions and need to be adjusted).
    std::string message2;
    g_jsonpro.ConvToJson(employee, message2);
    std::cout << std::endl;
    std::cout << message2;
    fail_unless(message == message2, "messages not equal");
}

} // namespace


int
main(int argc, char** argv)
{
    test1(false);
    test2(false);
    test1(true);
    test2(true);
    test3();

    // This will clean up all memory used by the protobuf library.
    google::protobuf::ShutdownProtobufLibrary();
}
