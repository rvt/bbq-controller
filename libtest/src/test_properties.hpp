#include <catch2/catch.hpp>

#include <memory>
#include <propertyutils.h>
#include <string>
#include <array>

using Catch::Matchers::Equals;


typedef PropertyValue PV ;

TEST_CASE("Scenario Properties", "[properties]") {
    Properties properties;
    SECTION("Should store const* char") {
        properties.put("foo", PV("BAR"));
        REQUIRE_THAT((const char*)properties.get("foo"), Equals("BAR"));
    }
    SECTION("Should return nullptr") {
        //REQUIRE(properties.get("nonexist").getBool() == false);
        //REQUIRE(properties.get("nonexist").getLong() == 0);
        //REQUIRE(properties.get("nonexist").getFloat() == 0.f);
        //REQUIRE(properties.get("nonexist").getCharPtr() == nullptr);
    }
    SECTION("Should write to stream") {
        properties.put("boolValue", PV(true));
        properties.put("longValue", PV(689876));
        properties.put("floatValue", PV(-12.6f));
        properties.put("charValue", PV("string value"));
        Stream stream;
        serializeProperties<32>(stream, properties);
        auto expected = "boolValue=B1\n"
                        "charValue=Sstring value\n"
                        "floatValue=F-12.6\n"
                        "longValue=L689876\n";
        REQUIRE_THAT(stream.streamedOut(), Equals(expected));
    }

    SECTION("Should read from stream") {
        Stream stream("boolValue  =    B  1\n"
                      "charValue=Sstring value\n"
                      "floatValue=F-12.6\n"
                      "longValue=L689876\n");
        deserializeProperties<32>(stream, properties);
        REQUIRE((bool)properties.get("boolValue") == true);
        REQUIRE((float)properties.get("floatValue") == Approx(-12.6));
        REQUIRE((long)properties.get("longValue") == 689876);
        REQUIRE_THAT((const char*)properties.get("charValue"), Equals("string value"));
    }

    SECTION("Should read without new line termination") {
        Stream stream("longValue=L689876");
        deserializeProperties<32>(stream, properties);
        REQUIRE((long)properties.get("longValue") == 689876);
    }

    SECTION("Should read truncated string") {
        Stream stream("stringValue=SabcRest is Ignored");
        deserializeProperties<16>(stream, properties);
        REQUIRE_THAT((const char*)properties.get("stringValue"), Equals("ab"));
    }

}