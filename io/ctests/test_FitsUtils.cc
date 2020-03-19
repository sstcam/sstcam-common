// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/io/FitsUtils.h"
#include "doctest.h"
#include <fstream>
#include <iostream>
//#include <sstcam/io/TIOReader.h>

namespace sstcam {
namespace io {
namespace fitsutils {

TEST_CASE("FitsUtils") {

    SUBCASE("ErrorMessage") {
        CHECK(ErrorMessage(0) == " (FITSIO: OK - no error)");
        CHECK(ErrorMessage(1) == " (FITSIO: non-CFITSIO program error)");
        CHECK(ErrorMessage(2) == " (FITSIO: unknown error status)");
        CHECK(ErrorMessage(104) == " (FITSIO: could not open the named file)");
    }

    // Check path exists
    std::string path = "/Users/Jason/Downloads/tempdata/obs/Run12917_r1.tio";
    std::ifstream file(path);
    REQUIRE(file.good());
    file.close();

    // Open fits file
    int status = 0;
    fitsfile* fits_ = nullptr;
    REQUIRE(!fits_open_file(&fits_, path.c_str(), READONLY, &status));

    SUBCASE("HasHeaderKey") {
        CHECK_THROWS_AS(HasHeaderKey(nullptr, ""), std::runtime_error);
        CHECK(!HasHeaderKey(fits_, ""));
        CHECK(HasHeaderKey(fits_, "EVENT_HEADER_VERSION"));
    }

    SUBCASE("GetHeaderKeyValue missing") {
        std::string key;
        constexpr auto GetHeaderInt = GetHeaderKeyValue<int32_t, TINT>;
        CHECK_THROWS(GetHeaderInt(fits_, key));

        constexpr auto GetHeaderFloat = GetHeaderKeyValue<float, TFLOAT>;
        CHECK_THROWS(GetHeaderFloat(fits_, key));

        constexpr auto GetHeaderDouble = GetHeaderKeyValue<double, TDOUBLE>;
        CHECK_THROWS(GetHeaderDouble(fits_, key));

        constexpr auto GetHeaderBool = GetHeaderKeyValue<bool, TLOGICAL>;
        CHECK_THROWS(GetHeaderBool(fits_, key));

        constexpr auto GetHeaderString = GetHeaderKeyValue<std::string, TSTRING>;
        CHECK_THROWS(GetHeaderString(fits_, key));
    }

    SUBCASE("GetHeaderKeyValue comment") {
        std::string key = "COMMENT";
        auto value_int = GetHeaderKeyValue<int32_t, TINT>(fits_, key);
        CHECK(value_int == 0);

        auto value_float = GetHeaderKeyValue<float, TFLOAT>(fits_, key);
        CHECK(value_float == 0.0f);

        auto value_double = GetHeaderKeyValue<double, TDOUBLE>(fits_, key);
        CHECK(value_double == 0);

        auto value_bool = GetHeaderKeyValue<bool, TLOGICAL>(fits_, key);
        CHECK(!value_bool);

        auto value_string = GetHeaderKeyValue<std::string, TSTRING>(fits_, key);
        CHECK(value_string == "  FITS (Flexible Image Transport System) format is defined in 'Astronomy");
    }

    SUBCASE("GetHeaderKeyValue history") {
        std::string key = "HISTORY";
        constexpr auto GetHeaderString = GetHeaderKeyValue<std::string, TSTRING>;
        CHECK_THROWS(GetHeaderString(fits_, key));
    }

    SUBCASE("GetHeaderKeyValue int") {
        std::string key = "EVENT_HEADER_VERSION";
        auto value_int16 = GetHeaderKeyValue<int16_t, TINT>(fits_, key);
        CHECK(value_int16 == 2);

        auto value_int = GetHeaderKeyValue<int32_t, TINT>(fits_, key);
        CHECK(value_int == 2);

        auto value_float = GetHeaderKeyValue<float, TFLOAT>(fits_, key);
        CHECK(value_float == 2.0f);

        auto value_double = GetHeaderKeyValue<double, TDOUBLE>(fits_, key);
        CHECK(value_double == 2.0);

        auto value_bool = GetHeaderKeyValue<bool, TLOGICAL>(fits_, key);
        CHECK(value_bool);

        auto value_string = GetHeaderKeyValue<std::string, TSTRING>(fits_, key);
        CHECK(value_string == "2");
    }

    SUBCASE("GetHeaderKeyValue float") {
        std::string key = "SLOT-31-TPRI";
        auto value_int = GetHeaderKeyValue<int32_t, TINT>(fits_, key);
        CHECK(value_int == 34);

        auto value_float = GetHeaderKeyValue<float, TFLOAT>(fits_, key);
        CHECK(value_float == 34.9375f);

        auto value_double = GetHeaderKeyValue<double, TDOUBLE>(fits_, key);
        CHECK(value_double == 34.9375);

        auto value_bool = GetHeaderKeyValue<bool, TLOGICAL>(fits_, key);
        CHECK(value_bool);

        auto value_string = GetHeaderKeyValue<std::string, TSTRING>(fits_, key);
        CHECK(value_string == "34.9375");
    }

    SUBCASE("GetHeaderKeyValue bool") {
        std::string key = "R1";
        auto value_int = GetHeaderKeyValue<int32_t, TINT>(fits_, key);
        CHECK(value_int == 1);

        auto value_float = GetHeaderKeyValue<float, TFLOAT>(fits_, key);
        CHECK(value_float == 1.0f);

        auto value_double = GetHeaderKeyValue<double, TDOUBLE>(fits_, key);
        CHECK(value_double == 1.0);

        auto value_bool = GetHeaderKeyValue<bool, TLOGICAL>(fits_, key);
        CHECK(value_bool);

        auto value_string = GetHeaderKeyValue<std::string, TSTRING>(fits_, key);
        CHECK(value_string == "T");
    }

    SUBCASE("GetHeaderKeyValue string") {
        std::string key = "OBSERVER";

        constexpr auto GetHeaderInt = GetHeaderKeyValue<int32_t, TINT>;
        CHECK_THROWS(GetHeaderInt(fits_, key));

        constexpr auto GetHeaderFloat = GetHeaderKeyValue<float, TFLOAT>;
        CHECK_THROWS(GetHeaderFloat(fits_, key));

        constexpr auto GetHeaderDouble = GetHeaderKeyValue<double, TDOUBLE>;
        CHECK_THROWS(GetHeaderDouble(fits_, key));

        auto value_bool = GetHeaderKeyValue<bool, TLOGICAL>(fits_, key);
        CHECK(!value_bool);

        auto value_string = GetHeaderKeyValue<std::string, TSTRING>(fits_, key);
        CHECK(value_string == "cta");
    }

    SUBCASE("GetHeaderKeyValue number in string") {
        std::string key = "RUNNUMBER";
        auto value_int = GetHeaderKeyValue<int32_t, TINT>(fits_, key);
        CHECK(value_int == 12917);

        auto value_float = GetHeaderKeyValue<float, TFLOAT>(fits_, key);
        CHECK(value_float == 12917.0f);

        auto value_double = GetHeaderKeyValue<double, TDOUBLE>(fits_, key);
        CHECK(value_double == 12917.0);

        auto value_bool = GetHeaderKeyValue<bool, TLOGICAL>(fits_, key);
        CHECK(!value_bool); // Why?

        auto value_string = GetHeaderKeyValue<std::string, TSTRING>(fits_, key);
        CHECK(value_string == "12917");
    }

    status = 0;
    REQUIRE(!fits_close_file(fits_, &status));
}

}}}
