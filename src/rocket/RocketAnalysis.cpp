#include "rocket/RocketAnalysis.hpp"

#include "rocket/Burn.hpp"
#include "rocket/DeltaV.hpp"
#include "rocket/MassFlow.hpp"
#include "rocket/TWR.hpp"

#include <cmath>
#include <utility>

namespace solar::rocket {
namespace {

void addIssue(VehicleAnalysis& analysis,
              const AnalysisIssueSeverity severity,
              const std::optional<std::size_t> stage,
              std::string message)
{
    analysis.issues.push_back({severity, stage, std::move(message)});
}

} // namespace

bool VehicleAnalysis::valid() const noexcept
{
    for (const auto& issue : issues) {
        if (issue.severity == AnalysisIssueSeverity::error) return false;
    }
    return true;
}

VehicleAnalysis analyzeVehicle(
    const Vehicle& vehicle,
    const PerformanceEnvironment environment,
    const double referenceGravityMetersPerSec2)
{
    VehicleAnalysis result;
    result.environment = environment;
    result.referenceGravityMetersPerSec2 = referenceGravityMetersPerSec2;
    result.payloadMassKg = vehicle.payloadMassKg;

    const auto validation = validateVehicle(vehicle);
    for (const auto& error : validation.errors) {
        addIssue(result, AnalysisIssueSeverity::error, std::nullopt, error);
    }
    if (!std::isfinite(referenceGravityMetersPerSec2) ||
        !(referenceGravityMetersPerSec2 > 0.0)) {
        addIssue(result, AnalysisIssueSeverity::error, std::nullopt,
                 "Reference gravity must be finite and positive");
    }
    if (!validation.valid() || !std::isfinite(referenceGravityMetersPerSec2) ||
        !(referenceGravityMetersPerSec2 > 0.0)) return result;

    result.liftoffMassKg = vehicle.liftoffMassKg();
    result.dryMassKg = vehicle.dryMassKg();
    result.propellantMassKg = vehicle.propellantMassKg();
    result.payloadFraction = vehicle.payloadFraction();
    result.stages.reserve(vehicle.stages.size());

    double ignitionMass = result.liftoffMassKg;
    for (std::size_t index = 0; index < vehicle.stages.size(); ++index) {
        const auto& stage = vehicle.stages[index];
        StageAnalysis stageResult;
        stageResult.stageIndex = index;
        stageResult.stageName = stage.name;
        stageResult.dryMassKg = stage.dryMassKg();
        stageResult.wetMassKg = stage.wetMassKg();
        stageResult.propellantMassKg = stage.propellant.massKg;
        stageResult.ignitionMassKg = ignitionMass;
        stageResult.burnoutMassKg = ignitionMass - stage.propellant.massKg;
        stageResult.postSeparationMassKg = stageResult.burnoutMassKg - stageResult.dryMassKg;
        stageResult.structuralCoefficient = stage.structuralCoefficient();
        stageResult.propellantFraction = stage.propellantFraction();
        if (stageResult.burnoutMassKg > 0.0) {
            stageResult.burnMassRatio = stageResult.ignitionMassKg / stageResult.burnoutMassKg;
        }

        stageResult.thrustN = engineClusterThrustN(
            stage.engine, stage.engineCount, environment, stage.throttle);
        stageResult.ispSeconds = stage.engine.ispSec(environment);
        stageResult.effectiveExhaustVelocityMetersPerSec =
            effectiveExhaustVelocityMetersPerSec(*stageResult.ispSeconds);
        stageResult.massFlowKgPerSec = engineClusterMassFlowKgPerSec(
            stage.engine, stage.engineCount, environment, stage.throttle);
        stageResult.deltaVMetersPerSec = idealDeltaVMetersPerSec(
            stageResult.ignitionMassKg, stageResult.burnoutMassKg,
            *stageResult.ispSeconds);
        if (stageResult.thrustN) {
            stageResult.initialTwr = thrustToWeightRatio(
                *stageResult.thrustN, stageResult.ignitionMassKg,
                referenceGravityMetersPerSec2);
            stageResult.finalTwr = thrustToWeightRatio(
                *stageResult.thrustN, stageResult.burnoutMassKg,
                referenceGravityMetersPerSec2);
        }
        if (stageResult.massFlowKgPerSec) {
            stageResult.burnTimeSeconds = burnTimeSeconds(
                stage.propellant.massKg, *stageResult.massFlowKgPerSec);
        }

        if (stage.engine.thrustN(environment) == 0.0) {
            addIssue(result, AnalysisIssueSeverity::warning, index,
                     "Stage has no thrust in the selected environment");
        }
        if (stage.propellant.massKg == 0.0) {
            addIssue(result, AnalysisIssueSeverity::warning, index,
                     "Stage has no propellant");
        }
        if (!stageResult.deltaVMetersPerSec) {
            addIssue(result, AnalysisIssueSeverity::error, index,
                     "Invalid mass ratio or specific impulse for ideal delta-v");
        } else {
            result.totalIdealDeltaVMetersPerSec += *stageResult.deltaVMetersPerSec;
        }
        if (stageResult.initialTwr && *stageResult.initialTwr < 1.0) {
            addIssue(result, AnalysisIssueSeverity::warning, index,
                     "TWR < 1: insufficient static thrust for vertical liftoff at this gravity; this may be acceptable for an upper or vacuum stage");
        }
        if (!stageResult.massFlowKgPerSec || !stageResult.burnTimeSeconds) {
            addIssue(result, AnalysisIssueSeverity::warning, index,
                     "Burn-time estimate is unavailable because mass flow is zero or invalid");
        }

        result.stages.push_back(std::move(stageResult));
        ignitionMass = result.stages.back().postSeparationMassKg;
    }
    return result;
}

} // namespace solar::rocket
