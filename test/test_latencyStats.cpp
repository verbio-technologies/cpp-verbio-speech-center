
#include <gtest/gtest.h>

#include "latencyStatistics.h"


TEST(LatencyStats, default_values) {
    LatencyStats stats;
    EXPECT_EQ(stats.mean, std::chrono::milliseconds(0));
    EXPECT_EQ(stats.standardDeviation, std::chrono::milliseconds(0));
}

TEST(RequestTimeStamps, default_values) {
    LatencyMeasure measure;
    EXPECT_EQ(measure.audioStartTime, std::chrono::milliseconds(0));
    EXPECT_EQ(measure.audioEndTime, std::chrono::milliseconds(0));
    EXPECT_EQ(measure.requestSentTime.time_since_epoch(), std::chrono::nanoseconds (0));
    EXPECT_EQ(measure.recievedReponsesTime.size(), 0);
}

TEST(RequestTimeStamps, latencies) {
    constexpr int numberOfResponses = 10;
    constexpr std::chrono::milliseconds latencyOffset(300);
    LatencyMeasure measure;
    auto now = std::chrono::system_clock::now();
    measure.requestSentTime = now;
    for (int i = 0; i < numberOfResponses; ++i)
        measure.recievedReponsesTime.push_back(now + std::chrono::milliseconds(latencyOffset * i));
    auto latencies = measure.calculateLatencies();
    EXPECT_EQ(latencies.size(), numberOfResponses);
    for (int i = 0; i < numberOfResponses; ++i)
        ASSERT_EQ(latencies[i], std::chrono::milliseconds(latencyOffset * i));
}


TEST(LatencyLog, default_values) {
    LatencyLog log;
    EXPECT_EQ(log.calculateStats().mean, std::chrono::milliseconds(0));
    EXPECT_EQ(log.calculateStats().standardDeviation, std::chrono::milliseconds(0));
    EXPECT_EQ(log.size(), 0);
}

TEST(LatencyLog, keep_reported_requests) {
    constexpr int numberOfRequests = 10;
    LatencyLog log;
    for (int i = 0; i < numberOfRequests; ++i)
        log.reportRequest(std::chrono::milliseconds(3), std::chrono::milliseconds(45), std::chrono::system_clock::now());
    EXPECT_EQ(log.size(), numberOfRequests);
}

TEST(LatencyLog, stats_for_reported_responses) {
    LatencyLog log;
    auto now = std::chrono::system_clock::now();
    constexpr std::chrono::milliseconds latency(300);
    constexpr std::chrono::milliseconds delayBetweenRequests(50);
    constexpr std::chrono::milliseconds stdDeviation(1);

    log.reportRequest(std::chrono::milliseconds(3), std::chrono::milliseconds(45), now);
    log.reportRequest(std::chrono::milliseconds(45), std::chrono::milliseconds(65), now + delayBetweenRequests);
    log.reportRequest(std::chrono::milliseconds(75), std::chrono::milliseconds(100), now + 2 * delayBetweenRequests);
    log.reportRequest(std::chrono::milliseconds(103), std::chrono::milliseconds(2500), now + 3 * delayBetweenRequests);

    log.reportResponse(std::chrono::milliseconds(1), now + latency*300);
    EXPECT_EQ(log.calculateStats().mean, std::chrono::milliseconds(0)) << "Wrong measure not ignored";
    EXPECT_EQ(log.calculateStats().standardDeviation, std::chrono::milliseconds(0)) << "Wrong measure not ignored";

    log.reportResponse(std::chrono::milliseconds(10), now + latency - stdDeviation);
    log.reportResponse(std::chrono::milliseconds(50), now + delayBetweenRequests + latency + stdDeviation);
    log.reportResponse(std::chrono::milliseconds(80), now + 2 * delayBetweenRequests + latency - stdDeviation);
    log.reportResponse(std::chrono::milliseconds(90), now + 2 * delayBetweenRequests + latency + stdDeviation);
    log.reportResponse(std::chrono::milliseconds(689), now + 3 * delayBetweenRequests + latency - stdDeviation);
    log.reportResponse(std::chrono::milliseconds(1362), now + 3 * delayBetweenRequests + latency + stdDeviation);

    const auto stats = log.calculateStats();
    EXPECT_EQ(stats.mean, latency) << "Got " << stats.mean.count() << " instead of " << latency.count();
    EXPECT_EQ(stats.standardDeviation, stdDeviation) << "Got " << stats.standardDeviation.count() << " instead of 0";
}

TEST(Latencylog, no_time_stamps) {
    LatencyLog log;
    auto now = std::chrono::system_clock::now();
    constexpr std::chrono::milliseconds latency(300);
    constexpr std::chrono::milliseconds delayBetweenRequests(500);

    log.reportRequest(std::chrono::milliseconds(3), std::chrono::milliseconds(45), now);
    log.reportRequest(std::chrono::milliseconds(45), std::chrono::milliseconds(65), now + delayBetweenRequests);

    log.reportResponse(now + delayBetweenRequests + latency);

    const auto stats = log.calculateStats();
    EXPECT_EQ(stats.mean, latency) << "Got " << stats.mean.count() << " instead of " << latency.count();
    EXPECT_EQ(stats.standardDeviation, std::chrono::milliseconds(0)) << "Got " << stats.standardDeviation.count() << " instead of 0";
}

TEST(Latencylog, response_prior_to_request) {
    LatencyLog log;
    auto now = std::chrono::system_clock::now();
    constexpr std::chrono::milliseconds latency(300);
    constexpr std::chrono::milliseconds delayBetweenRequests(500);
    constexpr std::chrono::milliseconds stdDeviation(10);

    log.reportRequest(std::chrono::milliseconds(3), std::chrono::milliseconds(45), now);
    log.reportRequest(std::chrono::milliseconds(45), std::chrono::milliseconds(65), now + delayBetweenRequests);

    log.reportResponse(now + latency - stdDeviation);
    log.reportResponse(now + delayBetweenRequests + latency - stdDeviation);

    EXPECT_THROW(log.calculateStats(), std::runtime_error) << "Accepted a response time prior to a request time";
}



