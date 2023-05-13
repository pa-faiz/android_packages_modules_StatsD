// Copyright (C) 2023 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ostream>

#include "frameworks/proto_logging/stats/atoms.pb.h"
#include "frameworks/proto_logging/stats/attribution_node.pb.h"
#include "src/shell/shell_data.pb.h"
#include "src/stats_log.pb.h"

namespace android {
namespace os {
namespace statsd {

using PackageInfo = UidMapping_PackageInfoSnapshot_PackageInfo;

// clang-format off

#define PROPERTY_MATCHER(typename, propname, matcher)                               \
    testing::Property(#propname, &typename::propname, matcher(expected.propname()))

#define REPEATED_PROPERTY_MATCHER(typename, propname, matcher)                      \
    testing::Property(#propname, &typename::propname,                               \
            testing::Pointwise(matcher(), expected.propname()))

#define PROPERTY_EQ(typename, propname) PROPERTY_MATCHER(typename, propname, testing::Eq)

#define REPEATED_PROPERTY_EQ(typename, propname)               \
    REPEATED_PROPERTY_MATCHER(typename, propname, testing::Eq)

/**
 * Generate a GoogleTest equality matcher for a custom type specified by typename with the given
 * properties.
 *
 * Example: For the protos:
 *     message Bar {
 *         optional int32 aa = 1;
 *     }
 *     message Foo {
 *         optional int32 a = 1;
 *         repeated float b = 2;
 *         optional Bar bar = 3;
 *         repeated Bar repeated_bar = 4;
 *     }
 * This will generate equality matchers for them called EqBar() and EqFoo():
 *      EQ_MATCHER(Bar, PROPERTY_EQ(Bar, aa));
 *      EQ_MATCHER(Foo,
 *              PROPERTY_EQ(Foo, a),
 *              PROPERTY_EQ(Foo, b),
 *              PROPERTY_MATCHER(Foo, bar, EqBar),
 *              REPEATED_PROPERTY_MATCHER(Foo, repeated_bar, EqBar)
 *      );
 */
#define EQ_MATCHER(typename, properties...)                                                   \
    MATCHER(Eq##typename, " ") {                                                              \
        return testing::Matches(Eq##typename(std::get<1>(arg)))(std::get<0>(arg));            \
    }                                                                                         \
    MATCHER_P(Eq##typename, expected, testing::PrintToString(expected)) {                     \
        return testing::ExplainMatchResult(testing::AllOf(properties), arg, result_listener); \
    }

#define PROPERTY_PRINT(propname)                                                    \
    if (obj.has_##propname()) {                                                     \
        *os << #propname << ": " << testing::PrintToString(obj.propname()) << ", "; \
    }

#define REPEATED_PROPERTY_PRINT(propname)                                           \
    if (obj.propname##_size() > 0) {                                                \
        *os << #propname << ": " << testing::PrintToString(obj.propname()) << ", "; \
    }

/**
 * Generates a method that teaches GoogleTest how to print a custom type specified by typename
 * with the given properties.
 * The equality matcher generated by EQ_MATCHER for the given typename will use this generated
 * printer method to print the object when the matcher fails.
 *
 * Example: For the protos:
 *     message Bar {
 *         optional int32 aa = 1;
 *     }
 *     message Foo {
 *         optional int32 a = 1;
 *         repeated float b = 2;
 *         optional Bar bar = 3;
 *         repeated Bar repeated_bar = 4;
 *     }
 * This will generate printer methods for them:
 *      TYPE_PRINTER(Bar, PROPERTY_PRINT(aa));
 *      TYPE_PRINTER(Foo,
 *              PROPERTY_PRINT(a)
 *              PROPERTY_PRINT(b)
 *              PROPERTY_PRINT(bar)
 *              REPEATED_PROPERTY_PRINT(repeated_bar)
 *      );
 */
#define TYPE_PRINTER(typename, properties)                       \
    inline void PrintTo(const typename& obj, std::ostream* os) { \
        *os << #typename << ": { ";                              \
        properties                                               \
        *os << "}";                                              \
    }

EQ_MATCHER(PackageInfo,
        PROPERTY_EQ(PackageInfo, version),
        PROPERTY_EQ(PackageInfo, uid),
        PROPERTY_EQ(PackageInfo, deleted),
        PROPERTY_EQ(PackageInfo, truncated_certificate_hash),
        PROPERTY_EQ(PackageInfo, name_hash),
        PROPERTY_EQ(PackageInfo, version_string_hash),
        PROPERTY_EQ(PackageInfo, name),
        PROPERTY_EQ(PackageInfo, version_string),
        PROPERTY_EQ(PackageInfo, installer_index),
        PROPERTY_EQ(PackageInfo, installer_hash),
        PROPERTY_EQ(PackageInfo, installer)
);
TYPE_PRINTER(PackageInfo,
        PROPERTY_PRINT(version)
        PROPERTY_PRINT(uid)
        PROPERTY_PRINT(deleted)
        PROPERTY_PRINT(truncated_certificate_hash)
        PROPERTY_PRINT(name_hash)
        PROPERTY_PRINT(version_string_hash)
        PROPERTY_PRINT(name)
        PROPERTY_PRINT(version_string)
        PROPERTY_PRINT(installer_index)
        PROPERTY_PRINT(installer_hash)
        PROPERTY_PRINT(installer)
);

EQ_MATCHER(AttributionNode,
        PROPERTY_EQ(AttributionNode, uid),
        PROPERTY_EQ(AttributionNode, tag)
);
TYPE_PRINTER(AttributionNode,
        PROPERTY_PRINT(uid)
        PROPERTY_PRINT(tag)
);

EQ_MATCHER(ScreenStateChanged, PROPERTY_EQ(ScreenStateChanged, state));
TYPE_PRINTER(ScreenStateChanged, PROPERTY_PRINT(state));

EQ_MATCHER(TrainExperimentIds,
        REPEATED_PROPERTY_EQ(TrainExperimentIds, experiment_id)
);
TYPE_PRINTER(TrainExperimentIds, REPEATED_PROPERTY_PRINT(experiment_id));

EQ_MATCHER(TestAtomReported,
        REPEATED_PROPERTY_MATCHER(TestAtomReported, attribution_node, EqAttributionNode),
        PROPERTY_EQ(TestAtomReported, int_field),
        PROPERTY_EQ(TestAtomReported, long_field),
        PROPERTY_EQ(TestAtomReported, float_field),
        PROPERTY_EQ(TestAtomReported, string_field),
        PROPERTY_EQ(TestAtomReported, boolean_field),
        PROPERTY_EQ(TestAtomReported, state),
        PROPERTY_MATCHER(TestAtomReported, bytes_field, EqTrainExperimentIds),
        REPEATED_PROPERTY_EQ(TestAtomReported, repeated_int_field),
        REPEATED_PROPERTY_EQ(TestAtomReported, repeated_long_field),
        REPEATED_PROPERTY_EQ(TestAtomReported, repeated_float_field),
        REPEATED_PROPERTY_EQ(TestAtomReported, repeated_string_field),
        REPEATED_PROPERTY_EQ(TestAtomReported, repeated_boolean_field),
        REPEATED_PROPERTY_EQ(TestAtomReported, repeated_enum_field)
);
TYPE_PRINTER(TestAtomReported,
        REPEATED_PROPERTY_PRINT(attribution_node)
        PROPERTY_PRINT(int_field)
        PROPERTY_PRINT(long_field)
        PROPERTY_PRINT(float_field)
        PROPERTY_PRINT(string_field)
        PROPERTY_PRINT(boolean_field)
        PROPERTY_PRINT(state)
        PROPERTY_PRINT(bytes_field)
        REPEATED_PROPERTY_PRINT(repeated_int_field)
        REPEATED_PROPERTY_PRINT(repeated_long_field)
        REPEATED_PROPERTY_PRINT(repeated_float_field)
        REPEATED_PROPERTY_PRINT(repeated_string_field)
        REPEATED_PROPERTY_PRINT(repeated_boolean_field)
        REPEATED_PROPERTY_PRINT(repeated_enum_field)
);

EQ_MATCHER(CpuActiveTime,
        PROPERTY_EQ(CpuActiveTime, uid),
        PROPERTY_EQ(CpuActiveTime, time_millis)
);
TYPE_PRINTER(CpuActiveTime,
        PROPERTY_PRINT(uid)
        PROPERTY_PRINT(time_millis)
);

EQ_MATCHER(PluggedStateChanged, PROPERTY_EQ(PluggedStateChanged, state));
TYPE_PRINTER(PluggedStateChanged, PROPERTY_PRINT(state));

EQ_MATCHER(Atom,
        PROPERTY_MATCHER(Atom, screen_state_changed, EqScreenStateChanged),
        PROPERTY_MATCHER(Atom, test_atom_reported, EqTestAtomReported)
);
TYPE_PRINTER(Atom,
        PROPERTY_PRINT(screen_state_changed)
        PROPERTY_PRINT(test_atom_reported)
);

EQ_MATCHER(ShellData,
        REPEATED_PROPERTY_MATCHER(ShellData, atom, EqAtom),
        REPEATED_PROPERTY_EQ(ShellData, elapsed_timestamp_nanos)
);
TYPE_PRINTER(ShellData,
        REPEATED_PROPERTY_PRINT(atom)
        REPEATED_PROPERTY_PRINT(elapsed_timestamp_nanos)
);

// clang-format on

}  // namespace statsd
}  // namespace os
}  // namespace android
