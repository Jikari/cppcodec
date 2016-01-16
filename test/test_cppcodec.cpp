/**
 *  Copyright (C) 2015 Topology LP
 *  All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#define CATCH_CONFIG_MAIN
#include "catch/single_include/catch.hpp"

#include <cppcodec/base32_crockford.hpp>
#include <cppcodec/base32_rfc4648.hpp>
#include <cppcodec/base64_rfc4648.hpp>
#include <cppcodec/base64_url.hpp>
#include <cppcodec/hex_lower.hpp>
#include <cppcodec/hex_upper.hpp>
#include <stdint.h>
#include <string.h> // for memcmp()
#include <vector>

TEST_CASE("Douglas Crockford's base32", "[base32][crockford]") {
    using base32 = cppcodec::base32_crockford;

    SECTION("encoded size calculation") {
        REQUIRE(base32::encoded_size(0) == 0);
        REQUIRE(base32::encoded_size(1) == 2);
        REQUIRE(base32::encoded_size(2) == 4);
        REQUIRE(base32::encoded_size(3) == 5);
        REQUIRE(base32::encoded_size(4) == 7);
        REQUIRE(base32::encoded_size(5) == 8);
        REQUIRE(base32::encoded_size(6) == 10);
        REQUIRE(base32::encoded_size(10) == 16);
    }

    SECTION("maximum decoded size calculation") {
        REQUIRE(base32::decoded_max_size(0) == 0);
        REQUIRE(base32::decoded_max_size(1) == 0);
        REQUIRE(base32::decoded_max_size(2) == 1);
        REQUIRE(base32::decoded_max_size(3) == 1);
        REQUIRE(base32::decoded_max_size(4) == 2);
        REQUIRE(base32::decoded_max_size(5) == 3);
        REQUIRE(base32::decoded_max_size(6) == 3);
        REQUIRE(base32::decoded_max_size(7) == 4);
        REQUIRE(base32::decoded_max_size(8) == 5);
        REQUIRE(base32::decoded_max_size(9) == 5);
        REQUIRE(base32::decoded_max_size(10) == 6);
        REQUIRE(base32::decoded_max_size(16) == 10);
    }

    std::string hello = "Hello World";
    std::string hello_encoded = "91JPRV3F41BPYWKCCG";
    std::string hello_encoded_null = "91JPRV3F41BPYWKCCG00";

    const uint8_t* hello_uint_ptr = reinterpret_cast<const uint8_t*>(hello.data());
    const uint8_t* hello_uint_ptr_encoded = reinterpret_cast<const uint8_t*>(hello_encoded.data());

    std::vector<uint8_t> hello_uint_vector(hello_uint_ptr, hello_uint_ptr + hello.size());
    std::vector<char> hello_char_vector_encoded(
            hello_encoded.data(), hello_encoded.data() + hello_encoded.size());
    std::vector<uint8_t> hello_uint_vector_encoded(
            hello_uint_ptr_encoded, hello_uint_ptr_encoded + hello_encoded.size());

    SECTION("encoding data") {
        REQUIRE(base32::encode(std::vector<uint8_t>()) == "");
        REQUIRE(base32::encode(std::vector<uint8_t>({0})) == "00");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0})) == "0000");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0})) == "00000");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0})) == "0000000");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0, 0})) == "00000000");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0, 0, 0})) == "0000000000");

        // Constructing an std::string reduces the size of the char array by one (null terminator).
        // Therefore, the result for passing the string literal directly ends up encoding
        // one more character, which produces two more symbols in this particular case.
        REQUIRE(base32::encode(std::string("Hello World")) == hello_encoded);
        REQUIRE(base32::encode("Hello World") == hello_encoded_null);

        REQUIRE(base32::encode(std::string("foo")) == "CSQPY");
        REQUIRE(base32::encode(std::string("lowercase UPPERCASE 1434567 !@#$%^&*"))
                == "DHQQESBJCDGQ6S90AN850HAJ8D0N6H9064T36D1N6RVJ08A04CJ2AQH658");
        REQUIRE(base32::encode(std::string("Wow, it really works!")) == "AXQQEB10D5T20WK5C5P6RY90EXQQ4TVK44");
    }

    SECTION("decoding data") {
        REQUIRE(base32::decode("") == std::vector<uint8_t>());
        REQUIRE(base32::decode("00") == std::vector<uint8_t>({0}));
        REQUIRE(base32::decode("0000") == std::vector<uint8_t>({0, 0}));
        REQUIRE(base32::decode("00000") == std::vector<uint8_t>({0, 0, 0}));
        REQUIRE(base32::decode("0000000") == std::vector<uint8_t>({0, 0, 0, 0}));
        REQUIRE(base32::decode("00000000") == std::vector<uint8_t>({0, 0, 0, 0, 0}));
        REQUIRE(base32::decode("0000000000") == std::vector<uint8_t>({0, 0, 0, 0, 0, 0}));

        // For decoding data, the result should be the same whether or not there is
        // a null terminator at the end, because the input is a string (not binary array).
        REQUIRE(base32::decode(std::string("91JPRV3F41BPYWKCCG")) == hello_uint_vector);
        REQUIRE(base32::decode("91JPRV3F41BPYWKCCG") == hello_uint_vector);

        REQUIRE(base32::decode<std::string>("CSQPY") == "foo");
        REQUIRE(base32::decode<std::string>("DHQQESBJCDGQ6S90AN850HAJ8D0N6H9064T36D1N6RVJ08A04CJ2AQH658")
                == "lowercase UPPERCASE 1434567 !@#$%^&*");

        // Lowercase should decode just as well as uppercase.
        REQUIRE(base32::decode<std::string>("AXQQEB10D5T20WK5C5P6RY90EXQQ4TVK44") == "Wow, it really works!");
        REQUIRE(base32::decode<std::string>("axqqeb10d5t20wk5c5p6ry90exqq4tvk44") == "Wow, it really works!");

        // Dashes are allowed for visual separation and ignored when decoding.
        REQUIRE(base32::decode<std::string>("-C-SQ--PY-") == "foo");

        // An invalid number of symbols should throw the right kind of parse_error.
        REQUIRE_THROWS_AS(base32::decode("0"), cppcodec::invalid_input_length&);
        REQUIRE_THROWS_AS(base32::decode("000"), cppcodec::invalid_input_length&);
        REQUIRE_THROWS_AS(base32::decode("000000"), cppcodec::invalid_input_length&);
        REQUIRE_THROWS_AS(base32::decode("000000000"), cppcodec::invalid_input_length&);

        // An invalid symbol should throw a symbol error.
        REQUIRE_THROWS_AS(base32::decode("00======"), cppcodec::symbol_error&); // no padding for Crockford
        REQUIRE_THROWS_AS(base32::decode("Uu"), cppcodec::symbol_error&); // only a checksum symbol here
        REQUIRE_THROWS_AS(base32::decode("++"), cppcodec::symbol_error&); // make sure it's not base64
        REQUIRE_THROWS_AS(base32::decode("//"), cppcodec::symbol_error&); // ...ditto
    }

    // Only test overloads once (for base32_crockford, since it happens to be the first one).
    // Since it's all templated, we assume that overloads work/behave similarly for other codecs.
    SECTION("encode() overloads") {
        // Other convenient overloads for taking raw pointer input.
        REQUIRE(base32::encode(hello.data(), hello.size()) == hello_encoded);
        REQUIRE(base32::encode(hello_uint_ptr, hello.size()) == hello_encoded);

        // Reused result pointer. Put the extra null terminator version in the middle to test resizing.
        std::string result;
        REQUIRE((base32::encode(result, hello_uint_ptr, hello.size()), result) == hello_encoded);
        REQUIRE((base32::encode(result, "Hello World"), result) == hello_encoded_null);
        REQUIRE((base32::encode(result, hello.data(), hello.size()), result) == hello_encoded);

        // Templated result. Use std::vector<uint8_t> to exercise non-char array types.
        REQUIRE(base32::encode<std::vector<uint8_t>>(hello) == hello_uint_vector_encoded);
        REQUIRE(base32::encode<std::vector<uint8_t>>(hello.data(), hello.size()) == hello_uint_vector_encoded);
        REQUIRE(base32::encode<std::vector<uint8_t>>(hello_uint_ptr, hello.size()) == hello_uint_vector_encoded);

        // Raw pointer output.
        std::vector<char> hello_char_result;
        hello_char_result.resize(base32::encoded_size(hello.size()));
        REQUIRE(hello_char_result.size() == hello_char_vector_encoded.size());

        size_t result_size;
        result_size = base32::encode(hello_char_result.data(), hello_char_result.size(), hello);
        REQUIRE(result_size == hello_char_vector_encoded.size());
        REQUIRE(hello_char_result == hello_char_vector_encoded);

        result_size = base32::encode(
                hello_char_result.data(), hello_char_result.size(), hello.data(), hello.size());
        REQUIRE(result_size == hello_char_vector_encoded.size());
        REQUIRE(hello_char_result == hello_char_vector_encoded);

        // Test that when passed a larger buffer, the null termination character will be written
        // after the last proper symbol. (Also test uint8_t* overload.)
        hello_char_result.resize(hello_char_result.size() + 1);
        hello_char_result[hello_char_result.size() - 1] = 'x';
        result_size = base32::encode(
                hello_char_result.data(), hello_char_result.size(), hello_uint_ptr, hello.size());
        REQUIRE(result_size == hello_char_vector_encoded.size());
        REQUIRE(hello_char_result[hello_char_result.size() - 1] == '\0');
        hello_char_result.resize(hello_char_result.size() - 1);
        REQUIRE(hello_char_result == hello_char_vector_encoded);
    }

    // Only test overloads once (for base32_crockford, since it happens to be the first one).
    // Since it's all templated, we assume that overloads work/behave similarly for other codecs.
    SECTION("decode() overloads") {
        // Other convenient overloads for taking raw pointer input.
        REQUIRE(base32::decode(hello_encoded.data(), hello_encoded.size()) == hello_uint_vector);

        // Reused result pointer. Put a different string in the middle to test resizing.
        std::vector<uint8_t> result;
        REQUIRE((base32::decode(result, hello_encoded.data(), hello_encoded.size()), result)
                == hello_uint_vector);
        REQUIRE((base32::decode(result, "00"), result) == std::vector<uint8_t>({0}));
        REQUIRE((base32::decode(result, hello_encoded), result) == hello_uint_vector);

        // Templated result. Use std::string to exercise non-uint8_t array types.
        REQUIRE(base32::decode<std::string>(hello_encoded) == hello);
        REQUIRE(base32::decode<std::string>(hello_uint_vector_encoded) == hello);
        REQUIRE(base32::decode<std::string>(hello_encoded.data(), hello_encoded.size()) == hello);

        // Raw pointer output.
        std::vector<uint8_t> hello_uint_result;
        std::vector<char> hello_char_result;
        size_t hello_decoded_max_size = base32::decoded_max_size(hello_encoded.size());
        REQUIRE(hello.size() <= hello_decoded_max_size);

        hello_char_result.resize(hello_decoded_max_size);
        size_t result_size = base32::decode(
                hello_char_result.data(), hello_char_result.size(), hello_encoded);
        REQUIRE(result_size == hello.size());
        REQUIRE(std::string(hello_char_result.data(), hello_char_result.data() + result_size) == hello);

        hello_char_result.resize(hello_decoded_max_size);
        result_size = base32::decode(
                hello_char_result.data(), hello_char_result.size(),
                hello_encoded.data(), hello_encoded.size());
        REQUIRE(result_size == hello.size());
        REQUIRE(std::string(hello_char_result.data(), hello_char_result.data() + result_size) == hello);

        hello_uint_result.resize(hello_decoded_max_size);
        result_size = base32::decode(
                hello_uint_result.data(), hello_uint_result.size(), hello_encoded);
        REQUIRE(result_size == hello.size());
        hello_uint_result.resize(result_size);
        REQUIRE(hello_uint_result == hello_uint_vector);

        hello_uint_result.resize(hello_decoded_max_size);
        result_size = base32::decode(
                hello_uint_result.data(), hello_uint_result.size(),
                hello_encoded.data(), hello_encoded.size());
        REQUIRE(result_size == hello.size());
        hello_uint_result.resize(result_size);
        REQUIRE(hello_uint_result == hello_uint_vector);
    }
}

TEST_CASE("base32 (RFC 4648)", "[base32][rfc4648]") {
    using base32 = cppcodec::base32_rfc4648;

    SECTION("encoded size calculation") {
        REQUIRE(base32::encoded_size(0) == 0);
        REQUIRE(base32::encoded_size(1) == 8);
        REQUIRE(base32::encoded_size(2) == 8);
        REQUIRE(base32::encoded_size(3) == 8);
        REQUIRE(base32::encoded_size(4) == 8);
        REQUIRE(base32::encoded_size(5) == 8);
        REQUIRE(base32::encoded_size(6) == 16);
        REQUIRE(base32::encoded_size(10) == 16);
    }

    SECTION("maximum decoded size calculation") {
        REQUIRE(base32::decoded_max_size(0) == 0);
        REQUIRE(base32::decoded_max_size(1) == 0);
        REQUIRE(base32::decoded_max_size(2) == 0);
        REQUIRE(base32::decoded_max_size(3) == 0);
        REQUIRE(base32::decoded_max_size(4) == 0);
        REQUIRE(base32::decoded_max_size(5) == 0);
        REQUIRE(base32::decoded_max_size(6) == 0);
        REQUIRE(base32::decoded_max_size(7) == 0);
        REQUIRE(base32::decoded_max_size(8) == 5);
        REQUIRE(base32::decoded_max_size(9) == 5);
        REQUIRE(base32::decoded_max_size(10) == 5);
        REQUIRE(base32::decoded_max_size(16) == 10);
    }

    SECTION("encoding data") {
        REQUIRE(base32::encode(std::vector<uint8_t>()) == "");
        REQUIRE(base32::encode(std::vector<uint8_t>({0})) == "AA======");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0})) == "AAAA====");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0})) == "AAAAA===");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0})) == "AAAAAAA=");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0, 0})) == "AAAAAAAA");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0, 0, 0})) == "AAAAAAAAAA======");

        // Constructing an std::string reduces the size of the char array by one (null terminator).
        // Therefore, the result for passing the string literal directly ends up encoding
        // one more character, which produces two more symbols in this particular case.
        REQUIRE(base32::encode(std::string("12345")) == "GEZDGNBV");
        REQUIRE(base32::encode("12345") == "GEZDGNBVAA======");

        REQUIRE(base32::encode(std::string("ABCDE")) == "IFBEGRCF");
        REQUIRE(base32::encode(std::vector<uint8_t>({255, 255, 255, 255, 255})) == "77777777");

        // RFC 4648: 10. Test Vectors
        REQUIRE(base32::encode(std::string("")) == "");
        REQUIRE(base32::encode(std::string("f")) == "MY======");
        REQUIRE(base32::encode(std::string("fo")) == "MZXQ====");
        REQUIRE(base32::encode(std::string("foo")) == "MZXW6===");
        REQUIRE(base32::encode(std::string("foob")) == "MZXW6YQ=");
        REQUIRE(base32::encode(std::string("fooba")) == "MZXW6YTB");
        REQUIRE(base32::encode(std::string("foobar")) == "MZXW6YTBOI======");
    }

    SECTION("decoding data") {
        REQUIRE(base32::decode("") == std::vector<uint8_t>());
        REQUIRE(base32::decode("AA======") == std::vector<uint8_t>({0}));
        REQUIRE(base32::decode("AAAA====") == std::vector<uint8_t>({0, 0}));
        REQUIRE(base32::decode("AAAAA===") == std::vector<uint8_t>({0, 0, 0}));
        REQUIRE(base32::decode("AAAAAAA=") == std::vector<uint8_t>({0, 0, 0, 0}));
        REQUIRE(base32::decode("AAAAAAAA") == std::vector<uint8_t>({0, 0, 0, 0, 0}));
        REQUIRE(base32::decode("AAAAAAAAAA======") == std::vector<uint8_t>({0, 0, 0, 0, 0, 0}));

        // For decoding data, the result should be the same whether or not there is
        // a null terminator at the end, because the input is a string (not binary array).
        REQUIRE(base32::decode<std::string>(std::string("GEZDGNBV")) == "12345");
        REQUIRE(base32::decode<std::string>("GEZDGNBV") == "12345");

        // RFC 4648: 10. Test Vectors
        REQUIRE(base32::decode<std::string>("") == "");
        REQUIRE(base32::decode<std::string>("MY======") == "f");
        REQUIRE(base32::decode<std::string>("MZXQ====") == "fo");
        REQUIRE(base32::decode<std::string>("MZXW6===") == "foo");
        REQUIRE(base32::decode<std::string>("MZXW6YQ=") == "foob");
        REQUIRE(base32::decode<std::string>("MZXW6YTB") == "fooba");
        REQUIRE(base32::decode<std::string>("MZXW6YTBOI======") == "foobar");

        // Lowercase should decode just as well as uppercase.
        REQUIRE(base32::decode<std::string>("mzxw6yTb") == "fooba");
        REQUIRE(base32::decode<std::string>("mZxW6yTb") == "fooba");

        // An invalid number of symbols should throw the right kind of parse_error.
        REQUIRE_THROWS_AS(base32::decode("A"), cppcodec::padding_error&);
        REQUIRE_THROWS_AS(base32::decode("AA"), cppcodec::padding_error&);
        REQUIRE_THROWS_AS(base32::decode("AA==="), cppcodec::padding_error&);
        REQUIRE_THROWS_AS(base32::decode("A======="), cppcodec::invalid_input_length&);
        REQUIRE_THROWS_AS(base32::decode("AAA====="), cppcodec::invalid_input_length&);
        REQUIRE_THROWS_AS(base32::decode("AAAAAA=="), cppcodec::invalid_input_length&);

        // An invalid symbol should throw a symbol error.
        REQUIRE_THROWS_AS(base32::decode("0A======"), cppcodec::symbol_error&);
        REQUIRE_THROWS_AS(base32::decode("1A======"), cppcodec::symbol_error&);
        REQUIRE_THROWS_AS(base32::decode("8A======"), cppcodec::symbol_error&);
        REQUIRE_THROWS_AS(base32::decode("9A======"), cppcodec::symbol_error&);
        REQUIRE_THROWS_AS(base32::decode("GEZD GNBV"), cppcodec::symbol_error&); // no spaces
        REQUIRE_THROWS_AS(base32::decode("GEZD-GNBV"), cppcodec::symbol_error&); // no dashes
    }
}

TEST_CASE("base64 (RFC 4648)", "[base64][rfc4648]") {
    using base64 = cppcodec::base64_rfc4648;

    SECTION("encoded size calculation") {
        REQUIRE(base64::encoded_size(0) == 0);
        REQUIRE(base64::encoded_size(1) == 4);
        REQUIRE(base64::encoded_size(2) == 4);
        REQUIRE(base64::encoded_size(3) == 4);
        REQUIRE(base64::encoded_size(4) == 8);
        REQUIRE(base64::encoded_size(5) == 8);
        REQUIRE(base64::encoded_size(6) == 8);
        REQUIRE(base64::encoded_size(7) == 12);
        REQUIRE(base64::encoded_size(12) == 16);
    }

    SECTION("maximum decoded size calculation") {
        REQUIRE(base64::decoded_max_size(0) == 0);
        REQUIRE(base64::decoded_max_size(1) == 0);
        REQUIRE(base64::decoded_max_size(2) == 0);
        REQUIRE(base64::decoded_max_size(3) == 0);
        REQUIRE(base64::decoded_max_size(4) == 3);
        REQUIRE(base64::decoded_max_size(5) == 3);
        REQUIRE(base64::decoded_max_size(6) == 3);
        REQUIRE(base64::decoded_max_size(7) == 3);
        REQUIRE(base64::decoded_max_size(8) == 6);
        REQUIRE(base64::decoded_max_size(9) == 6);
        REQUIRE(base64::decoded_max_size(10) == 6);
        REQUIRE(base64::decoded_max_size(11) == 6);
        REQUIRE(base64::decoded_max_size(12) == 9);
        REQUIRE(base64::decoded_max_size(16) == 12);
    }

    SECTION("encoding data") {
        REQUIRE(base64::encode(std::vector<uint8_t>()) == "");
        REQUIRE(base64::encode(std::vector<uint8_t>({0})) == "AA==");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0})) == "AAA=");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0})) == "AAAA");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0, 0})) == "AAAAAA==");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0, 0, 0})) == "AAAAAAA=");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0, 0, 0, 0})) == "AAAAAAAA");

        // Constructing an std::string reduces the size of the char array by one (null terminator).
        // Therefore, the result for passing the string literal directly ends up encoding
        // one more character, which produces two more symbols in this particular case.
        REQUIRE(base64::encode(std::string("Man")) == "TWFu");
        REQUIRE(base64::encode("Man") == "TWFuAA==");

        // Wikipedia
        REQUIRE(base64::encode(std::string("pleasure.")) == "cGxlYXN1cmUu");
        REQUIRE(base64::encode(std::string("leasure.")) == "bGVhc3VyZS4=");
        REQUIRE(base64::encode(std::string("easure.")) == "ZWFzdXJlLg==");
        REQUIRE(base64::encode(std::string("asure.")) == "YXN1cmUu");
        REQUIRE(base64::encode(std::string("sure.")) == "c3VyZS4=");

        REQUIRE(base64::encode(std::string("any carnal pleas")) == "YW55IGNhcm5hbCBwbGVhcw==");
        REQUIRE(base64::encode(std::string("any carnal pleasu")) == "YW55IGNhcm5hbCBwbGVhc3U=");
        REQUIRE(base64::encode(std::string("any carnal pleasur")) == "YW55IGNhcm5hbCBwbGVhc3Vy");

        // RFC 4648: 9. Illustrations and Examples, adapted for more special characters
        REQUIRE(base64::encode(std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9, 0x7E})) == "FPu/A9l+");
        REQUIRE(base64::encode(std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9})) == "FPu/A9k=");
        REQUIRE(base64::encode(std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03})) == "FPu/Aw==");

        // RFC 4648: 10. Test Vectors
        REQUIRE(base64::encode(std::string("")) == "");
        REQUIRE(base64::encode(std::string("f")) == "Zg==");
        REQUIRE(base64::encode(std::string("fo")) == "Zm8=");
        REQUIRE(base64::encode(std::string("foo")) == "Zm9v");
        REQUIRE(base64::encode(std::string("foob")) == "Zm9vYg==");
        REQUIRE(base64::encode(std::string("fooba")) == "Zm9vYmE=");
        REQUIRE(base64::encode(std::string("foobar")) == "Zm9vYmFy");
    }

    SECTION("decoding data") {
        REQUIRE(base64::decode("") == std::vector<uint8_t>());
        REQUIRE(base64::decode("AA==") == std::vector<uint8_t>({0}));
        REQUIRE(base64::decode("AAA=") == std::vector<uint8_t>({0, 0}));
        REQUIRE(base64::decode("AAAA") == std::vector<uint8_t>({0, 0, 0}));
        REQUIRE(base64::decode("AAAAAA==") == std::vector<uint8_t>({0, 0, 0, 0}));
        REQUIRE(base64::decode("AAAAAAA=") == std::vector<uint8_t>({0, 0, 0, 0, 0}));
        REQUIRE(base64::decode("AAAAAAAA") == std::vector<uint8_t>({0, 0, 0, 0, 0, 0}));

        // For decoding data, the result should be the same whether or not there is
        // a null terminator at the end, because the input is a string (not binary array).
        REQUIRE(base64::decode<std::string>(std::string("TWFu")) == "Man");
        REQUIRE(base64::decode<std::string>("TWFu") == "Man");

        // Wikipedia
        REQUIRE(base64::decode<std::string>("cGxlYXN1cmUu") == "pleasure.");
        REQUIRE(base64::decode<std::string>("bGVhc3VyZS4=") == "leasure.");
        REQUIRE(base64::decode<std::string>("ZWFzdXJlLg==") == "easure.");
        REQUIRE(base64::decode<std::string>("YXN1cmUu") == "asure.");
        REQUIRE(base64::decode<std::string>("c3VyZS4=") == "sure.");

        REQUIRE(base64::decode<std::string>("YW55IGNhcm5hbCBwbGVhcw==") == "any carnal pleas");
        REQUIRE(base64::decode<std::string>("YW55IGNhcm5hbCBwbGVhc3U=") == "any carnal pleasu");
        REQUIRE(base64::decode<std::string>("YW55IGNhcm5hbCBwbGVhc3Vy") == "any carnal pleasur");

        // RFC 4648: 9. Illustrations and Examples, adapted for more special characters
        REQUIRE(base64::decode("FPu/A9l+") == std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9, 0x7E}));
        REQUIRE(base64::decode("FPu/A9k=") == std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9}));
        REQUIRE(base64::decode("FPu/Aw==") == std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03}));

        // RFC 4648: 10. Test Vectors
        REQUIRE(base64::decode<std::string>("") == "");
        REQUIRE(base64::decode<std::string>("Zg==") == "f");
        REQUIRE(base64::decode<std::string>("Zm8=") == "fo");
        REQUIRE(base64::decode<std::string>("Zm9v") == "foo");
        REQUIRE(base64::decode<std::string>("Zm9vYg==") == "foob");
        REQUIRE(base64::decode<std::string>("Zm9vYmE=") == "fooba");
        REQUIRE(base64::decode<std::string>("Zm9vYmFy") == "foobar");

        // An invalid number of symbols should throw the right kind of parse_error.
        REQUIRE_THROWS_AS(base64::decode("A"), cppcodec::padding_error&);
        REQUIRE_THROWS_AS(base64::decode("AA"), cppcodec::padding_error&);
        REQUIRE_THROWS_AS(base64::decode("A==="), cppcodec::invalid_input_length&);
        REQUIRE_THROWS_AS(base64::decode("AAAA===="), cppcodec::invalid_input_length&);
        REQUIRE_THROWS_AS(base64::decode("AAAAA==="), cppcodec::invalid_input_length&);

        // An invalid symbol should throw a symbol error.
        REQUIRE_THROWS_AS(base64::decode("--"), cppcodec::symbol_error&); // this is not base64url
        REQUIRE_THROWS_AS(base64::decode("__"), cppcodec::symbol_error&); // ...ditto
    }
}

TEST_CASE("base64 (URL-safe)", "[base64][url]") {
    using base64 = cppcodec::base64_url;

    SECTION("encoded size calculation") {
        REQUIRE(base64::encoded_size(0) == 0);
        REQUIRE(base64::encoded_size(1) == 4);
        REQUIRE(base64::encoded_size(2) == 4);
        REQUIRE(base64::encoded_size(3) == 4);
        REQUIRE(base64::encoded_size(4) == 8);
        REQUIRE(base64::encoded_size(5) == 8);
        REQUIRE(base64::encoded_size(6) == 8);
        REQUIRE(base64::encoded_size(7) == 12);
        REQUIRE(base64::encoded_size(12) == 16);
    }

    SECTION("maximum decoded size calculation") {
        REQUIRE(base64::decoded_max_size(0) == 0);
        REQUIRE(base64::decoded_max_size(1) == 0);
        REQUIRE(base64::decoded_max_size(2) == 0);
        REQUIRE(base64::decoded_max_size(3) == 0);
        REQUIRE(base64::decoded_max_size(4) == 3);
        REQUIRE(base64::decoded_max_size(5) == 3);
        REQUIRE(base64::decoded_max_size(6) == 3);
        REQUIRE(base64::decoded_max_size(7) == 3);
        REQUIRE(base64::decoded_max_size(8) == 6);
        REQUIRE(base64::decoded_max_size(9) == 6);
        REQUIRE(base64::decoded_max_size(10) == 6);
        REQUIRE(base64::decoded_max_size(11) == 6);
        REQUIRE(base64::decoded_max_size(12) == 9);
        REQUIRE(base64::decoded_max_size(16) == 12);
    }

    SECTION("encoding data") {
        REQUIRE(base64::encode(std::vector<uint8_t>()) == "");
        REQUIRE(base64::encode(std::vector<uint8_t>({0})) == "AA==");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0})) == "AAA=");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0})) == "AAAA");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0, 0})) == "AAAAAA==");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0, 0, 0})) == "AAAAAAA=");
        REQUIRE(base64::encode(std::vector<uint8_t>({0, 0, 0, 0, 0, 0})) == "AAAAAAAA");

        // RFC 4648: 9. Illustrations and Examples, adapted for more special characters
        REQUIRE(base64::encode(std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9, 0x7E})) == "FPu_A9l-");
        REQUIRE(base64::encode(std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9})) == "FPu_A9k=");
        REQUIRE(base64::encode(std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03})) == "FPu_Aw==");

        // RFC 4648: 10. Test Vectors
        REQUIRE(base64::encode(std::string("")) == "");
        REQUIRE(base64::encode(std::string("f")) == "Zg==");
        REQUIRE(base64::encode(std::string("fo")) == "Zm8=");
        REQUIRE(base64::encode(std::string("foo")) == "Zm9v");
        REQUIRE(base64::encode(std::string("foob")) == "Zm9vYg==");
        REQUIRE(base64::encode(std::string("fooba")) == "Zm9vYmE=");
        REQUIRE(base64::encode(std::string("foobar")) == "Zm9vYmFy");
    }

    SECTION("decoding data") {
        REQUIRE(base64::decode("") == std::vector<uint8_t>());
        REQUIRE(base64::decode("AA==") == std::vector<uint8_t>({0}));
        REQUIRE(base64::decode("AAA=") == std::vector<uint8_t>({0, 0}));
        REQUIRE(base64::decode("AAAA") == std::vector<uint8_t>({0, 0, 0}));
        REQUIRE(base64::decode("AAAAAA==") == std::vector<uint8_t>({0, 0, 0, 0}));
        REQUIRE(base64::decode("AAAAAAA=") == std::vector<uint8_t>({0, 0, 0, 0, 0}));
        REQUIRE(base64::decode("AAAAAAAA") == std::vector<uint8_t>({0, 0, 0, 0, 0, 0}));

        // RFC 4648: 9. Illustrations and Examples, adapted for more special characters
        REQUIRE(base64::decode("FPu_A9l-") == std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9, 0x7E}));
        REQUIRE(base64::decode("FPu_A9k=") == std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03, 0xD9}));
        REQUIRE(base64::decode("FPu_Aw==") == std::vector<uint8_t>({0x14, 0xFB, 0xBF, 0x03}));

        // RFC 4648: 10. Test Vectors
        REQUIRE(base64::decode<std::string>("") == "");
        REQUIRE(base64::decode<std::string>("Zg==") == "f");
        REQUIRE(base64::decode<std::string>("Zm8=") == "fo");
        REQUIRE(base64::decode<std::string>("Zm9v") == "foo");
        REQUIRE(base64::decode<std::string>("Zm9vYg==") == "foob");
        REQUIRE(base64::decode<std::string>("Zm9vYmE=") == "fooba");
        REQUIRE(base64::decode<std::string>("Zm9vYmFy") == "foobar");

        // An invalid number of symbols should throw the right kind of parse_error.
        REQUIRE_THROWS_AS(base64::decode("A"), cppcodec::padding_error&);
        REQUIRE_THROWS_AS(base64::decode("AA"), cppcodec::padding_error&);
        REQUIRE_THROWS_AS(base64::decode("A==="), cppcodec::invalid_input_length&);
        REQUIRE_THROWS_AS(base64::decode("AAAA===="), cppcodec::invalid_input_length&);
        REQUIRE_THROWS_AS(base64::decode("AAAAA==="), cppcodec::invalid_input_length&);

        // An invalid symbol should throw a symbol error.
        REQUIRE_THROWS_AS(base64::decode("++"), cppcodec::symbol_error&); // this is not standard base64
        REQUIRE_THROWS_AS(base64::decode("//"), cppcodec::symbol_error&); // ...ditto
    }
}
