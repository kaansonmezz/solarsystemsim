#pragma once

#include "rocket/Engine.hpp"
#include "rocket/Vehicle.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace solar::rocket {

enum class AnalysisIssueSeverity { warning, error };

struct AnalysisIssue final {
    AnalysisIssueSeverity severity{AnalysisIssueSeverity::warning};
    std::optional<std::size_t> stageIndex;
    std::string message;
};

struct StageAnalysis final {
    std::size_t stageIndex{};
    std::string stageName;
    double dryMassKg{};
    double wetMassKg{};
    double propellantMassKg{};
    double ignitionMassKg{};
    double burnoutMassKg{};
    double postSeparationMassKg{};
    std::optional<double> thrustN;
    std::optional<double> ispSeconds;
    std::optional<double> effectiveExhaustVelocityMetersPerSec;
    std::optional<double> massFlowKgPerSec;
    std::optional<double> deltaVMetersPerSec;
    std::optional<double> initialTwr;
    std::optional<double> finalTwr;
    std::optional<double> burnTimeSeconds;
    std::optional<double> burnMassRatio;
    double structuralCoefficient{};
    double propellantFraction{};
};

struct VehicleAnalysis final {
    PerformanceEnvironment environment{PerformanceEnvironment::vacuum};
    double referenceGravityMetersPerSec2{};
    double liftoffMassKg{};
    double dryMassKg{};
    double propellantMassKg{};
    double payloadMassKg{};
    double payloadFraction{};
    double totalIdealDeltaVMetersPerSec{};
    std::vector<StageAnalysis> stages;
    std::vector<AnalysisIssue> issues;

    [[nodiscard]] bool valid() const noexcept;
};

[[nodiscard]] VehicleAnalysis analyzeVehicle(
    const Vehicle& vehicle,
    PerformanceEnvironment environment,
    double referenceGravityMetersPerSec2);

} // namespace solar::rocket
