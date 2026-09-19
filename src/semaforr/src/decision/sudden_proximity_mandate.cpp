/**
 * @file sudden_proximity_mandate.cpp
 * @brief Sudden proximity mandate responsibilities.
 *
 * @details This file implements sudden proximity mandate behavior for tiered
 * decision making and action arbitration. It records the declarations,
 * settings, fixtures, or guidance needed by that responsibility. Its
 * package-relative location is
 * `src/decision/sudden_proximity_mandate.cpp`.
 */
#include <cmath>
#include <semaforr/decision/sudden_proximity_mandate.hpp>
#include <semaforr/domain/geometry.hpp>
#include <stdexcept>
#include <utility>

namespace semaforr::decision {

/**
 * @brief Performs the sudden proximity mandate operation for this
 * subsystem.
 *
 * Arguments:
 * - @p configuration: Supplies configuration input to the operation.
 *
 * Returns:
 * - No value; effects are applied to owned state or outputs.
 *
 * Exceptions:
 * - None documented; validation or dependency failures may propagate.
 */
SuddenProximityMandate::SuddenProximityMandate(
    SuddenProximityMandateConfiguration configuration)
    : configuration_(std::move(configuration)) {
  if (configuration_.maximum_age < std::chrono::nanoseconds::zero()) {
    throw std::invalid_argument(
        "sudden proximity mandate maximum age must be non-negative");
  }
  if (!std::isfinite(configuration_.minimum_confidence) ||
      configuration_.minimum_confidence < 0.0 ||
      configuration_.minimum_confidence > 1.0) {
    throw std::invalid_argument(
        "sudden proximity mandate minimum confidence must be within [0, 1]");
  }
  if (!std::isfinite(configuration_.emergency_distance_m) ||
      configuration_.emergency_distance_m <= 0.0) {
    throw std::invalid_argument(
        "sudden proximity mandate emergency distance must be finite and " "positive");
  }
}

/**
 * @brief Evaluates package content for this subsystem.
 *
 * Arguments:
 * - @p context: Supplies context input to the operation.
 *
 * Returns:
 * - `std::optional<Decision>` containing the operation result.
 *
 * Exceptions:
 * - None documented; validation or dependency failures may propagate.
 */
std::optional<Decision> SuddenProximityMandate::evaluate(const DecisionContext& context) const {const auto& current = context.world.crowd.current();
  if (!current || !current->usable(configuration_.maximum_age, configuration_.minimum_confidence)) {
    return std::nullopt;
  }

  for (const auto& pedestrian : current->pedestrians) {
    if (pedestrian.confidence < configuration_.minimum_confidence) {
      continue;
    }

    const double distance_m =
        domain::distance(context.world.robot.pose.position, pedestrian.position).meters();

    if (distance_m < configuration_.emergency_distance_m) {
      return Decision{domain::Action::pause(), std::string(name()), "sudden_proximity_mandate:emergency_pause"};
    }
  }
  return std::nullopt;
}

}  // namespace semaforr::decision
