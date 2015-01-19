/*

Copyright (c) 2013, Coolplay (pengchong/Jack)
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

#ifndef IPCS_COMMON_JSON_PROTOBUF_H
#define IPCS_COMMON_JSON_PROTOBUF_H

#include "json.h"
#include "message.h"

namespace ipcs_common {

class JsonPro {
public:
        /**
         * Convert a protobuf message to a JSON object, storing
         * the result in a Json::Value object.
         *
         * @param msg the protobuf message to convert
         * @param value JSON object to hold the converted value
         * @throw std::invalid_argument protobuf message contains fields
         * that can not be converted to JSON
         */
        void ConvToJson(
            const google::protobuf::Message& message,
            Json::Value& value);

        /**
         * Convert a protobuf message to a JSON object, storing the
         * result in a std::string.
         *
         * @param msg the protobuf message to convert
         * @param value JSON object to hold the converted value
         * @throw std::invalid_argument protobuf message contains fields that
         * can not be converted to JSON
         */
        void ConvToJson(
            const google::protobuf::Message& message,
            std::string& value);

        /**
         * Convert a JSON object to a protobuf message, reading the
         * JSON value from a Json::Value object.
         *
         * @param value JSON object to convert
         * @param message protobuf message to hold the converted value
         * @throw std::invalid_argument a JSON field does not match an existing protobuf
         * message field
         */
        void ConvToProto(
            const Json::Value& value,
            google::protobuf::Message& message);

        /**
         * Convert a JSON object to a protobuf message, reading the
         * JSON value from a std::string.
         *
         * @param value JSON object to convert
         * @param message protobuf message to hold the converted value
         * @throw std::invalid_argument  value does not represent a valid
         * JSON object or a JSON field does not match an existing protobuf
         * message field
         */
        void ConvToProto(
            const std::string& value,
            google::protobuf::Message& message);
private:
    /**
     * Set a single field value in the JSON object.
     *
     * @param reflection the protobuf message reflection informationm
     * @param field the protobuf message field information
     * @param message the protobuf message to read from
     * @param value the JSON object to update
     * @throws std::invalid_argument could not update JSON object
     */
    void convert_single_field(
        const google::protobuf::Reflection* reflection,
        const google::protobuf::FieldDescriptor* field,
        const google::protobuf::Message& message,
        Json::Value& value);

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
    void convert_repeated_field(
        const google::protobuf::Reflection* reflection,
        F1 converter,
        const google::protobuf::FieldDescriptor* field,
        const google::protobuf::Message& message,
        Json::Value& value);

    /**
     * Convert a repeated field to JSON for a message type.
     *
     * @param reflection the reflection interface for the message
     * @param field the field descriptor for the field we are converting
     * @param message the message being converted
     * @param value the value to store the converted data
     */
    void convert_repeated_message_field(
        const google::protobuf::Reflection* reflection,
        const google::protobuf::FieldDescriptor* field,
        const google::protobuf::Message& message,
        Json::Value& value);

    /**
     * Convert a repeated field to JSON for an enum type.
     *
     * @param reflection the reflection interface for the message
     * @param field the field descriptor for the field we are converting
     * @param message the message being converted
     * @param value the value to store the converted data
     */
    void convert_repeated_enum_field(
        const google::protobuf::Reflection* reflection,
        const google::protobuf::FieldDescriptor* field,
        const google::protobuf::Message& message, Json::Value& value);

    /**
     * Convert a repeated field to JSON.
     *
     * @param reflection the reflection interface for the message
     * @param field the field descriptor for the field we are converting
     * @param message the message being converted
     * @param value the value to store the converted data
     */
    void convert_repeated_field(
        const google::protobuf::Reflection* reflection,
        const google::protobuf::FieldDescriptor* field,
        const google::protobuf::Message& message, Json::Value& value);

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
    void update_field(
        const google::protobuf::Reflection* reflection,
        const google::protobuf::FieldDescriptor* field,
        F1 updater,
        const Json::Value& value, F2 accessor, F3 checker,
        google::protobuf::Message& message);

    /**
     * Update a single enum field within a message from a given JSON object.
     *
     * @param reflection the reflection interface for the message
     * @param field the field descriptor for the field we are updating
     * @param value the value to get the information from
     * @param message the message being updated
     */
    template <typename F1>
    void update_enum_field(
        const google::protobuf::Reflection* reflection,
        const google::protobuf::Descriptor* descriptor,
        const google::protobuf::FieldDescriptor* field,
        F1 updater, const Json::Value& value,
        google::protobuf::Message& message);

    /**
     * Update a single field within a message from the given JSON object.
     *
     * @param reflection the reflection interface for the message
     * @param field the field descriptor for the field we are updating
     * @param value the value to get the information from
     * @param message the message being updated
     */
    void update_single_field(
        const google::protobuf::Reflection* reflection,
        const google::protobuf::Descriptor* descriptor,
        const google::protobuf::FieldDescriptor* field,
        const Json::Value& value, google::protobuf::Message& message);

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
    void update_repeated_field(const google::protobuf::Reflection* reflection,
        const google::protobuf::FieldDescriptor* field,
        F1 updater, const Json::Value& value, F2 accessor, F3 checker,
        google::protobuf::Message& message);

    /**
     * Update a repeated message field within a message from a given JSON object.
     *
     * @param reflection the reflection interface for the message
     * @param field the field descriptor for the field we are updating
     * @param value the value to get the information from
     * @param message the message being updated
     */
    void update_repeated_message_field(
        const google::protobuf::Reflection* reflection,
        const google::protobuf::FieldDescriptor* field,
        const Json::Value& value, google::protobuf::Message& message);

    /**
     * Update a repeated enum field within a message from a given JSON object.
     *
     * @param reflection the reflection interface for the message
     * @param field the field descriptor for the field we are updating
     * @param value the value to get the information from
     * @param message the message being updated
     */
    void update_repeated_enum_field(const google::protobuf::Reflection* reflection,
        const google::protobuf::Descriptor* descriptor,
        const google::protobuf::FieldDescriptor* field,
        const Json::Value& value, google::protobuf::Message& message);

    /**
     * Update a repeated field within a message from the given JSON object.
     *
     * @param reflection the reflection interface for the message
     * @param field the field descriptor for the field we are updating
     * @param value the value to get the information from
     * @param message the message being updated
     */
    void update_repeated_field(const google::protobuf::Reflection* reflection,
        const google::protobuf::Descriptor* descriptor,
        const google::protobuf::FieldDescriptor* field,
        const Json::Value& value, google::protobuf::Message& message);

};

} // namespace ipcs_common

#endif // IPCS_COMMON_JSON_PROTOBUF_H
