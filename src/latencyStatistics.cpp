#include "latencyStatistics.h"

#include <chrono>
#include <vector>
#include <cmath>


std::vector<std::chrono::milliseconds> LatencyMeasure::calculateLatencies() const {
    std::vector<std::chrono::milliseconds> latencies;
    for (const auto& request : recievedReponsesTime) {
        if (request >= requestSentTime)
            latencies.emplace_back(std::chrono::duration_cast<std::chrono::milliseconds>(request - requestSentTime));
        else
            throw std::runtime_error("Response reported to happen before request.");
    }
    return latencies;
}


void LatencyLog::reportRequest(const std::chrono::milliseconds &start,
                               const std::chrono::milliseconds &end,
                               const std::chrono::system_clock::time_point &requestSentTime) {
    measures.push_back({start, end, requestSentTime, {}});
}

void LatencyLog::reportResponse(const std::chrono::milliseconds &audioTimeStamps, const std::chrono::system_clock::time_point &responseReceivedTime) {
    for (auto &measure : measures)
        if (measure.audioStartTime <= audioTimeStamps && audioTimeStamps < measure.audioEndTime)
            measure.recievedReponsesTime.push_back(responseReceivedTime);
}

void LatencyLog::reportResponse(const std::chrono::system_clock::time_point &responseReceivedTime) {
    if (!measures.empty())
        measures.back().recievedReponsesTime.push_back(responseReceivedTime);
}



size_t LatencyLog::size() const {
    return measures.size();
}

LatencyStats LatencyLog::calculateStats() const {
    std::chrono::milliseconds cumsum(0);
    long cumsum2(0);
    unsigned int latencyCount = 0;
    for (auto const &request : measures) {
        for (auto const &latency: request.calculateLatencies()) {
            cumsum += latency;
            cumsum2 += latency.count() * latency.count();
            latencyCount += 1;
        }
    }

    auto mean = latencyCount != 0 ? cumsum / latencyCount : cumsum;
    return {
            latencyCount != 0 ? mean  : std::chrono::milliseconds(0),
            std::chrono::milliseconds(latencyCount != 0 ? static_cast<long>(std::sqrt((cumsum2 / latencyCount) - mean.count()*mean.count()))  : 0)
    };
}

std::string LatencyLog::getReport() const {
    std::ostringstream oss;
    oss << "requests: [" << std::endl;
    for (const auto &measure : measures) {
        if (!measure.recievedReponsesTime.empty()) {
            oss << "  {" << std::endl
                << "    audio_time_stamps: { start_time_in_ms: " << measure.audioStartTime.count() << ", ";
            oss << "    end_time_in_ms: " << measure.audioEndTime.count() << "} , " << std::endl;
            oss << "    latencies_in_ms: [";
            for (const auto &responseTime: measure.recievedReponsesTime)
                oss << std::chrono::duration_cast<std::chrono::milliseconds>(responseTime - measure.requestSentTime).count() << ", ";
            oss << "], " << std::endl;
            oss << "  }," << std::endl;
        }
    }
    oss << "]" << std::endl;
    return oss.str();
}
