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

#include <sstream>
#include <stdexcept>

#include "descriptor.h"
#include "repeated_field.h"

namespace ipcs_common {

/**
 * Set a single field value in the JSON object.
 *
 * @param reflection the protobuf message reflection informationm
 * @param field the protobuf message field information
 * @param message the protobuf message to read from
 * @param value the JSON object to update
 * @throws std::invalid_argument could not update JSON object
 */
void JsonPro::convert_single_field(
    const google::protobuf::Reflection* reflection,
    const google::protobuf::FieldDescriptor* field,
    const google::protobuf::Message& message, Json::Value& value) {
        switch (field->type()) {
        case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
                value[field->name()] = reflection->GetDouble(message, field);
                break;
        case google::protobuf::FieldDescriptor::TYPE_FLOAT:
                value[field->name()] = reflection->GetFloat(message, field);
                break;
        case google::protobuf::FieldDescriptor::TYPE_INT64:
        case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
        case google::protobuf::FieldDescriptor::TYPE_SINT64:
                value[field->name()] = (Json::Value::Int64)
                    reflection->GetInt64(message, field);
                break;
        case google::protobuf::FieldDescriptor::TYPE_UINT64:
        case google::protobuf::FieldDescriptor::TYPE_FIXED64:
                value[field->name()] = (Json::Value::UInt64)
                    reflection->GetUInt64(message, field);
                break;
        case google::protobuf::FieldDescriptor::TYPE_INT32:
        case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
        case google::protobuf::FieldDescriptor::TYPE_SINT32:
                value[field->name()] = reflection->GetInt32(message, field);
                break;
        case google::protobuf::FieldDescriptor::TYPE_FIXED32:
        case google::protobuf::FieldDescriptor::TYPE_UINT32:
                value[field->name()] = reflection->GetUInt32(message, field);
                break;
        case google::protobuf::FieldDescriptor::TYPE_BOOL:
                value[field->name()] = reflection->GetBool(message, field);
                break;
        case google::protobuf::FieldDescriptor::TYPE_STRING:
                value[field->name()] = reflection->GetString(message, field);
                break;
        case google::protobuf::FieldDescriptor::TYPE_GROUP:
                throw std::invalid_argument
                    ("group type not supported for field '" +
                        field->full_name() + "'");
        case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
                ConvToJson(
                    reflection->GetMessage(message, field),
                    value[field->name()]);
                break;
        case google::protobuf::FieldDescriptor::TYPE_BYTES:
                throw std::invalid_argument
                    ("binary type not supported for field '" +
                     field->full_name() + "'");
        case google::protobuf::FieldDescriptor::TYPE_ENUM:
                value[field->name()] =
                    reflection->GetEnum(message, field)->name();
                break;
        default:
                std::ostringstream out;
                out << "unknown type in field '" << field->full_name() <<
                    "': " << field->type();
                throw std::invalid_argument(out.str());
        }
}

/**
 * Convert a repeated field to JSON for various types.
 *
 * @param reflection the reflection interface for the message
 * @param converter the member function to do the conversion
 * @param field the field descriptor for the field we are converting
 * @param message the message being converted
 * @param value the value to store the converted data
 */
template <typename T, typename F1>
void JsonPro::convert_repeated_field(
    const google::protobuf::Reflection* reflection,
    F1 converter,
    const google::protobuf::FieldDescriptor* field,
    const google::protobuf::Message& message,
    Json::Value& value) {
        int size = reflection->FieldSize(message, field);
        value.resize(size);

        for (int i = 0; i < size; i++) {
            value[i] = (T)(reflection->*converter)(message, field, i);
        }
}

/**
 * Convert a repeated field to JSON for a message type.
 *
 * @param reflection the reflection interface for the message
 * @param field the field descriptor for the field we are converting
 * @param message the message being converted
 * @param value the value to store the converted data
 */
void JsonPro::convert_repeated_message_field(
    const google::protobuf::Reflection* reflection,
    const google::protobuf::FieldDescriptor* field,
    const google::protobuf::Message& message,
    Json::Value& value) {
        int size = reflection->FieldSize(message, field);
        value.resize(size);

        for (int i = 0; i < size; i++) {
                ConvToJson(reflection->GetRepeatedMessage
                    (message, field, i), value[i]);
        }
}

/**
 * Convert a repeated field to JSON for an enum type.
 *
 * @param reflection the reflection interface for the message
 * @param field the field descriptor for the field we are converting
 * @param message the message being converted
 * @param value the value to store the converted data
 */
void JsonPro::convert_repeated_enum_field(
    const google::protobuf::Reflection* reflection,
    const google::protobuf::FieldDescriptor* field,
    const google::protobuf::Message& message, Json::Value& value) {
        int size = reflection->FieldSize(message, field);
        value.resize(size);

        for (int i = 0; i < size; i++) {
                value[i] = reflection->GetRepeatedEnum
                    (message, field, i)->name();
        }
}

/**
 * Convert a repeated field to JSON.
 *
 * @param reflection the reflection interface for the message
 * @param field the field descriptor for the field we are converting
 * @param message the message being converted
 * @param value the value to store the converted data
 */
void JsonPro::convert_repeated_field(
    const google::protobuf::Reflection* reflection,
    const google::protobuf::FieldDescriptor* field,
    const google::protobuf::Message& message, Json::Value& value) {
        switch (field->type()) {
        case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
                convert_repeated_field<double>(reflection,
                    &google::protobuf::Reflection::GetRepeatedUInt64,
                    field, message, value[field->name()]);
                break;
        case google::protobuf::FieldDescriptor::TYPE_FLOAT:
                convert_repeated_field<double>(reflection,
                    &google::protobuf::Reflection::GetRepeatedFloat,
                    field, message, value[field->name()]);
                break;
        case google::protobuf::FieldDescriptor::TYPE_INT64:
        case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
        case google::protobuf::FieldDescriptor::TYPE_SINT64:
                convert_repeated_field<Json::Int64>(reflection,
                    &google::protobuf::Reflection::GetRepeatedInt64,
                    field, message, value[field->name()]);
                break;
        case google::protobuf::FieldDescriptor::TYPE_UINT64:
        case google::protobuf::FieldDescriptor::TYPE_FIXED64:
                convert_repeated_field<Json::UInt64>(reflection,
                    &google::protobuf::Reflection::GetRepeatedUInt64,
                    field, message, value[field->name()]);
                break;
        case google::protobuf::FieldDescriptor::TYPE_INT32:
        case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
        case google::protobuf::FieldDescriptor::TYPE_SINT32:
                convert_repeated_field<Json::Int>(reflection,
                    &google::protobuf::Reflection::GetRepeatedInt32,
                    field, message, value[field->name()]);
                break;
        case google::protobuf::FieldDescriptor::TYPE_FIXED32:
        case google::protobuf::FieldDescriptor::TYPE_UINT32:
                convert_repeated_field<Json::UInt>(reflection,
                    &google::protobuf::Reflection::GetRepeatedUInt32,
                    field, message, value[field->name()]);
                break;
        case google::protobuf::FieldDescriptor::TYPE_BOOL:
                convert_repeated_field<bool>(reflection,
                    &google::protobuf::Reflection::GetRepeatedBool,
                    field, message, value[field->name()]);
                break;
        case google::protobuf::FieldDescriptor::TYPE_STRING:
                convert_repeated_field<std::string>(reflection,
                    &google::protobuf::Reflection::GetRepeatedString,
                    field, message, value[field->name()]);
                break;
        case google::protobuf::FieldDescriptor::TYPE_GROUP:
                throw std::invalid_argument
                    ("group type not supported for field '" +
                        field->full_name() + "'");
        case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
                convert_repeated_message_field(reflection,
                    field, message, value[field->name()]);
                break;
        case google::protobuf::FieldDescriptor::TYPE_BYTES:
                throw std::invalid_argument
                    ("binary type not supported for field '" +
                        field->full_name() + "'");
        case google::protobuf::FieldDescriptor::TYPE_ENUM:
                convert_repeated_enum_field(reflection,
                    field, message, value[field->name()]);
                break;
        default:
                std::ostringstream out;
                out << "unknown type in field '" << field->full_name() <<
                    "': " << field->type();
                throw std::invalid_argument(out.str());
        }
}

/**
 * Update a field within a message from a given JSON object,
 * for many types of values.
 *
 * @param reflection the reflection interface for the message
 * @param field the field descriptor for the field we are updating
 * @param updater the method to use to update the field
 * @param value the value to get the information from
 * @param access the method used to get the value from the JSON object
 * @param checker the method used to check that the JSON type is correct
 * @param message the message being updated
 */
template <typename F1, typename F2, typename F3>
void JsonPro::update_field(
    const google::protobuf::Reflection* reflection,
    const google::protobuf::FieldDescriptor* field,
    F1 updater,
    const Json::Value& value, F2 accessor, F3 checker,
    google::protobuf::Message& message) {
        if (!((value.*checker)())) {
                std::stringstream ss;
                ss << field->type();
                throw std::invalid_argument("expecting type " +
                    ss.str() + " for field '" +
                    field->full_name() + "'");
        }

        (reflection->*updater)(&message, field, (value.*accessor)());
}

/**
 * Update a single enum field within a message from a given JSON object.
 *
 * @param reflection the reflection interface for the message
 * @param field the field descriptor for the field we are updating
 * @param value the value to get the information from
 * @param message the message being updated
 */
template <typename F1>
void JsonPro::update_enum_field(
    const google::protobuf::Reflection* reflection,
    const google::protobuf::Descriptor* descriptor,
    const google::protobuf::FieldDescriptor* field,
    F1 updater,
    const Json::Value& value, google::protobuf::Message& message) {
        if (!value.isString()) {
                throw std::invalid_argument
                    ("expecting string (enum) for field '" +
                        field->full_name() + "'");
        }

        const google::protobuf::EnumValueDescriptor* enumValueDescriptor =
            descriptor->FindEnumValueByName(value.asString());
        if (0 == enumValueDescriptor) {
                throw std::invalid_argument("unknown enum for field '" +
                    field->full_name() + "': '" + value.asString() + "'");
        }

        (reflection->*updater)(&message, field, enumValueDescriptor);
}

/**
 * Update a single field within a message from the given JSON object.
 *
 * @param reflection the reflection interface for the message
 * @param field the field descriptor for the field we are updating
 * @param value the value to get the information from
 * @param message the message being updated
 */
void JsonPro::update_single_field(
    const google::protobuf::Reflection* reflection,
    const google::protobuf::Descriptor* descriptor,
    const google::protobuf::FieldDescriptor* field,
    const Json::Value& value, google::protobuf::Message& message) {
        switch (field->type()) {
        case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
                update_field(reflection, field,
                    &google::protobuf::Reflection::SetDouble, value,
                    &Json::Value::asDouble, &Json::Value::isNumeric, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_FLOAT:
                update_field(reflection, field,
                    &google::protobuf::Reflection::SetFloat, value,
                    &Json::Value::asFloat, &Json::Value::isNumeric, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_INT64:
        case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
        case google::protobuf::FieldDescriptor::TYPE_SINT64:
                update_field(reflection, field,
                    &google::protobuf::Reflection::SetInt64, value,
                    &Json::Value::asInt64, &Json::Value::isIntegral, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_UINT64:
        case google::protobuf::FieldDescriptor::TYPE_FIXED64:
                update_field(reflection, field,
                    &google::protobuf::Reflection::SetUInt64, value,
                    &Json::Value::asUInt64, &Json::Value::isIntegral, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_INT32:
        case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
        case google::protobuf::FieldDescriptor::TYPE_SINT32:
                update_field(reflection, field,
                    &google::protobuf::Reflection::SetInt32, value,
                    &Json::Value::asInt, &Json::Value::isIntegral, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_FIXED32:
        case google::protobuf::FieldDescriptor::TYPE_UINT32:
                update_field(reflection, field,
                    &google::protobuf::Reflection::SetUInt32, value,
                    &Json::Value::asUInt, &Json::Value::isIntegral, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_BOOL:
                update_field(reflection, field,
                    &google::protobuf::Reflection::SetBool, value,
                    &Json::Value::asBool, &Json::Value::isBool, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_STRING:
                update_field(reflection, field,
                    &google::protobuf::Reflection::SetString, value,
                    &Json::Value::asString, &Json::Value::isString, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_GROUP:
                throw std::invalid_argument
                    ("group type not supported for field '" +
                        field->full_name() + "'");
        case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
                ConvToProto(value,
                    *(reflection->MutableMessage(&message, field)));
                break;
        case google::protobuf::FieldDescriptor::TYPE_BYTES:
                throw std::invalid_argument
                    ("binary type not supported for field '" +
                        field->full_name() + "'");
        case google::protobuf::FieldDescriptor::TYPE_ENUM:
                update_enum_field(reflection, descriptor, field,
                    &google::protobuf::Reflection::SetEnum, value,
                    message);
                break;
        default:
                std::ostringstream out;
                out << "unknown type in field '" << field->full_name() <<
                    "': " << field->type();
                throw std::invalid_argument(out.str());
        }
}

/**
 * Update a repeated field within a message from a given JSON object,
 * for many types of values.
 *
 * @param reflection the reflection interface for the message
 * @param field the field descriptor for the field we are updating
 * @param updater the method to use to update the field
 * @param value the value to get the information from
 * @param access the method used to get the value from the JSON object
 * @param checker the method used to check that the JSON type is correct
 * @param message the message being updated
 */
template <typename F1, typename F2, typename F3>
void JsonPro::update_repeated_field(
    const google::protobuf::Reflection* reflection,
    const google::protobuf::FieldDescriptor* field,
    F1 updater, const Json::Value& value, F2 accessor, F3 checker,
    google::protobuf::Message& message) {
        for (Json::ValueIterator iter = value.begin(); iter != value.end();
             ++iter) {
                update_field(reflection, field, updater, *iter,
                    accessor, checker, message);
        }
}

/**
 * Update a repeated message field within a message from a given JSON object.
 *
 * @param reflection the reflection interface for the message
 * @param field the field descriptor for the field we are updating
 * @param value the value to get the information from
 * @param message the message being updated
 */
void JsonPro::update_repeated_message_field(
    const google::protobuf::Reflection* reflection,
    const google::protobuf::FieldDescriptor* field,
    const Json::Value& value, google::protobuf::Message& message)
{
        for (Json::ValueIterator iter = value.begin(); iter != value.end();
             ++iter) {
                google::protobuf::Message* child =
                    reflection->AddMessage(&message, field);
                ConvToProto(*iter, *child);
        }
}

/**
 * Update a repeated enum field within a message from a given JSON object.
 *
 * @param reflection the reflection interface for the message
 * @param field the field descriptor for the field we are updating
 * @param value the value to get the information from
 * @param message the message being updated
 */
void JsonPro::update_repeated_enum_field(
    const google::protobuf::Reflection* reflection,
    const google::protobuf::Descriptor* descriptor,
    const google::protobuf::FieldDescriptor* field,
    const Json::Value& value, google::protobuf::Message& message)
{
        for (Json::ValueIterator iter = value.begin(); iter != value.end();
             ++iter) {
                update_enum_field(reflection, descriptor, field,
                    &google::protobuf::Reflection::AddEnum, *iter, message);
        }
}

/**
 * Update a repeated field within a message from the given JSON object.
 *
 * @param reflection the reflection interface for the message
 * @param field the field descriptor for the field we are updating
 * @param value the value to get the information from
 * @param message the message being updated
 */
void JsonPro::update_repeated_field(
    const google::protobuf::Reflection* reflection,
    const google::protobuf::Descriptor* descriptor,
    const google::protobuf::FieldDescriptor* field,
    const Json::Value& value, google::protobuf::Message& message) {
        switch (field->type()) {
        case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
                update_repeated_field(reflection, field,
                    &google::protobuf::Reflection::AddDouble, value,
                    &Json::Value::asDouble, &Json::Value::isNumeric, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_FLOAT:
                update_repeated_field(reflection, field,
                    &google::protobuf::Reflection::AddFloat, value,
                    &Json::Value::asFloat, &Json::Value::isNumeric, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_INT64:
        case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
        case google::protobuf::FieldDescriptor::TYPE_SINT64:
                update_repeated_field(reflection, field,
                    &google::protobuf::Reflection::AddInt64, value,
                    &Json::Value::asInt64, &Json::Value::isIntegral, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_UINT64:
        case google::protobuf::FieldDescriptor::TYPE_FIXED64:
                update_repeated_field(reflection, field,
                    &google::protobuf::Reflection::AddUInt64, value,
                    &Json::Value::asUInt64, &Json::Value::isIntegral, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_INT32:
        case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
        case google::protobuf::FieldDescriptor::TYPE_SINT32:
                update_repeated_field(reflection, field,
                    &google::protobuf::Reflection::AddInt32, value,
                    &Json::Value::asInt, &Json::Value::isIntegral, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_FIXED32:
        case google::protobuf::FieldDescriptor::TYPE_UINT32:
                update_repeated_field(reflection, field,
                    &google::protobuf::Reflection::AddUInt32, value,
                    &Json::Value::asUInt, &Json::Value::isIntegral, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_BOOL:
                update_repeated_field(reflection, field,
                    &google::protobuf::Reflection::AddBool, value,
                    &Json::Value::asBool, &Json::Value::isBool, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_STRING:
                update_repeated_field(reflection, field,
                    &google::protobuf::Reflection::AddString, value,
                    &Json::Value::asString, &Json::Value::isString, message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_GROUP:
                throw std::invalid_argument
                    ("group type not supported for field '" +
                        field->full_name() + "'");
        case google::protobuf::FieldDescriptor::TYPE_MESSAGE:
                update_repeated_message_field(reflection, field, value,
                    message);
                break;
        case google::protobuf::FieldDescriptor::TYPE_BYTES:
                throw std::invalid_argument
                    ("binary type not supported for field '" +
                        field->full_name() + "'");
        case google::protobuf::FieldDescriptor::TYPE_ENUM:
                update_repeated_enum_field(reflection, descriptor, field, value,
                    message);
                break;
        default:
                std::ostringstream out;
                out << "unknown type in field '" << field->full_name() <<
                    "': " << field->type();
                throw std::invalid_argument(out.str());
        }
}

void JsonPro::ConvToJson(
    const google::protobuf::Message& message,
    Json::Value& value) {
        //
        // Walk through the protobuf fields and insert them into the JSON
        // value.
        //
        const google::protobuf::Reflection* reflection =
            message.GetReflection();

        std::vector<const google::protobuf::FieldDescriptor*> fields;
        reflection->ListFields(message, &fields);

        for (std::vector<const google::protobuf::FieldDescriptor*>\
                 ::const_iterator iter = fields.begin();
             iter != fields.end(); ++iter) {
                const google::protobuf::FieldDescriptor* field = *iter;

                switch (field->label()) {
                case google::protobuf::FieldDescriptor::LABEL_OPTIONAL:
                case google::protobuf::FieldDescriptor::LABEL_REQUIRED:
                        convert_single_field(reflection, field, message, value);
                        break;

                case google::protobuf::FieldDescriptor::LABEL_REPEATED:
                        convert_repeated_field(reflection, field, message,
                            value);
                        break;

                default:
                        std::ostringstream out;
                        out << "unknown label in field '" <<
                            field->full_name() << "': " << field->label();
                        throw std::invalid_argument(out.str());
                }

        }
}

void JsonPro::ConvToJson(
    const google::protobuf::Message& message,
    std::string& value) {
        Json::Value json_value;
        ConvToJson(message, json_value);
        value = Json::FastWriter().write(json_value);
}

void JsonPro::ConvToProto(
    const Json::Value& value,
    google::protobuf::Message& message) {
        if (value.type() != Json::objectValue) {
                throw std::invalid_argument("expecting JSON object type");
        }

        //
        // Walk through the JSON members and insert them into the protobuf.
        //
        const google::protobuf::Reflection* reflection =
            message.GetReflection();
        const google::protobuf::Descriptor* descriptor =
            message.GetDescriptor();

        for (Json::ValueIterator iter = value.begin(); iter != value.end();
             ++iter) {
                const char* name = iter.memberName();

                const google::protobuf::FieldDescriptor* field =
                    descriptor->FindFieldByName(name);

                if (0 == field) {
                        throw std::invalid_argument
                            ("JSON field '" + std::string(name) +
                                "' not found in message");
                }

                Json::Value& current = *iter;

                switch (field->label()) {
                case google::protobuf::FieldDescriptor::LABEL_OPTIONAL:
                case google::protobuf::FieldDescriptor::LABEL_REQUIRED:
                        update_single_field(reflection, descriptor, field,
                            current, message);
                        break;

                case google::protobuf::FieldDescriptor::LABEL_REPEATED:
                        // make sure the JSON value is an array
                        if (!current.isArray()) {
                                throw std::invalid_argument
                                    ("expecting JSON array for field '" +
                                        field->full_name() + "'");
                        }

                        // the JSON array will completely replace this field
                        reflection->ClearField(&message, field);

                        update_repeated_field(reflection, descriptor, field,
                            current, message);
                        break;

                default:
                        std::ostringstream out;
                        out << "unknown label in field '" <<
                            field->full_name() << "': " << field->label();
                        throw std::invalid_argument(out.str());
                }
        }
}

void JsonPro::ConvToProto(
    const std::string& value,
    google::protobuf::Message& message) {
        Json::Value json_value(Json::objectValue);
        Json::Reader reader;
        bool result = reader.parse(value, json_value);

        if (!result) {
                throw std::invalid_argument("failed to parse JSON document: " +
                    reader.getFormattedErrorMessages());
        }

        ConvToProto(json_value, message);
}

} // namespace ipcs_common
