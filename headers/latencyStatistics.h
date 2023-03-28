#ifndef CLI_CLIENT_LATENCYSTATISTICS_H
#define CLI_CLIENT_LATENCYSTATISTICS_H


#include <chrono>
#include <vector>
#include <stdexcept>
#include <sstream>


struct LatencyStats {
    std::chrono::milliseconds mean{0};
    std::chrono::milliseconds standardDeviation{0};
};


struct LatencyMeasure {
    std::chrono::milliseconds audioStartTime{0};
    std::chrono::milliseconds audioEndTime{0};
    std::chrono::time_point<std::chrono::system_clock> requestSentTime;
    std::vector<std::chrono::time_point<std::chrono::system_clock> > recievedReponsesTime;
    [[nodiscard]] std::vector<std::chrono::milliseconds> calculateLatencies() const;
};


class LatencyLog  {
public:
    LatencyLog() = default;
    ~LatencyLog() = default;

    void reportRequest(const std::chrono::milliseconds &start,
                       const std::chrono::milliseconds &end,
                       const std::chrono::system_clock::time_point &requestSentTime);
    void reportResponse(const std::chrono::milliseconds &audioTimeStamps,
                        const std::chrono::system_clock::time_point &responseReceivedTime);
    void reportResponse(const std::chrono::system_clock::time_point &responseReceivedTime);
    [[nodiscard]] size_t size() const;
    [[nodiscard]] LatencyStats calculateStats() const;

    [[nodiscard]] std::string getReport() const {
        std::ostringstream oss;
        oss << std::endl;
        for (const auto &measure : measures) {
            for (const auto &responseTime : measure.recievedReponsesTime) {
                oss << measure.audioStartTime.count() << ", " << measure.audioEndTime.count() << ", " << std::chrono::duration_cast<std::chrono::milliseconds>(responseTime - measure.requestSentTime).count() << std::endl;
            }
        }
        return oss.str();
    }

private:
    std::vector<LatencyMeasure> measures;
};



#endif
