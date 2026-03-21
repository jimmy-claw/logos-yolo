#pragma once
// Minimal stub of the Logos SDK LogosAPIClient class.
// Used only in BUILD_TESTS builds — the real header comes from logos-cpp-sdk.
//
// Compile-time guard: this header MUST NOT be included in production builds.
#ifndef YOLO_TEST_BUILD
#error "logos_api_client.h is a test stub — do not include in production builds. Define YOLO_TEST_BUILD only for test targets."
#endif

#include <QString>
#include <QVariant>

class LogosAPIClient {
public:
    LogosAPIClient() = default;
    virtual ~LogosAPIClient() = default;

    virtual QVariant invokeRemoteMethod(const QString& /*objectName*/,
                                        const QString& /*method*/,
                                        const QVariant& /*arg1*/ = QVariant(),
                                        const QVariant& /*arg2*/ = QVariant(),
                                        const QVariant& /*arg3*/ = QVariant())
    {
        return {};
    }
};
