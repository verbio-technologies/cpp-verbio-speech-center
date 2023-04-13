#include <gtest/gtest.h>

#include "SpeechCenterCredentials.h"


TEST(credentials, first) {
    SpeechCenterCredentials credentials{"client-id",
                                        "client-secret",
                                        "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiJ9.eyJpc3MiOiJhdXRobWFuYWdlciIsInN1YiI6IkVoN0pSbFNpenkiLCJzZXJ2aWNlIjoiU3BlZWNoQ2VudGVyIiwiZXhwIjoxNjgxMzc2ODIyLCJpYXQiOjE2ODEzNzMyMjIsImp0aSI6IjQ0NGY1NzkwLWUwOGItNDM1My1hYzJjLTc4ZDcwN2QzNzEyZiJ9.tUED-ucNalvErsn4W3NzYSo4LZNMFkiQyjlyppVTeG-gq1Ii7GLBEeW287gK-I8qu-Z-sySalndT8Cdm1SHxHxcJhnbCEsvfmIZhmH5WOrhWMONvk-_Fz2NbtfWkxYsn66KdvFzjkCwcjSXc7OWsIZMReUbebNzVdBMw4Fype5sF2TLig6TIEgyoiJukliuWtZWTV8HDiToD5_nWfNNIvd-c0rR9mK3kTYzzg6P1k-3yQpi7kB86zkyu5ti1KF8Ov3cIYXDZ18xf3B8Wqx37NXMaTnuNiDCOKpCF6SyrC6r9NNQ7t47AO0fgbWMUuFRz8p0SQ3dflpcYEvcmtUMDZHL9IazGCxqm1lVPg_OEwtVmrWMElgA5zRYbipX7OQ5A6y1I2tw6Zbv5AM_dTYMyTrDPJWwyYj_68OSQhs5gtJNW6gPLhAI6rORzfX0f_fotO8xBJ6IQ-7IMRzePJpRIgt9LhaTFCYiKG3y3Xg_80_cH8Yy_f6aQEqTfQAjC3bzaNhg60y9SJdZZWArfYWdMMobGK4FwJILL4rIQCjSyFXNZeyNEnXFLXXT4L0OFB86mKJui8H8KbQrNs0zvBx-wxnblkVxiA9xKZP5-88nrMiVvGwIFA_ikiVCrutKb6bzvS9i4bH63SrLpNCHgrSldoyzF4oAgY6zvn0_COIbcmlg",
                                        ""};

    auto expiration_time = credentials.expirationTime();

    ASSERT_EQ(expiration_time, 1681376822);
}