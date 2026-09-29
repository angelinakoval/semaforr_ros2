/**
 * @file approach_direction_advisor.cpp
 * @brief Approach direction advisor responsibilities.
 *
 * @details This file implements approach direction advisor behavior for tiered
 * decision making and action arbitration. It records the declarations,
 * settings, fixtures, or guidance needed by that responsibility. Its
 * package-relative location is
 * `src/decision/advisors/social/approach_direction_advisor.cpp`.
 */
#include <algorithm>
#include <cmath>
#include <semaforr/decision/advisors/social/approach_direction_advisor.hpp>
#include <semaforr/decision/advisors/social/social_prediction.hpp>
#include <semaforr/domain/motion_model.hpp>
#include <stdexcept>
#include <utility>

namespace semaforr::decision {
namespace {

/**
 * @brief Validates positive for this subsystem.
 *
 * Arguments:
 * - @p value: Supplies value input to the operation.
 * - @p name: Supplies name input to the operation.
 *
 * Returns:
 * - No value; effects are applied to owned state or outputs.
 *
 * Exceptions:
 * - None documented; validation or dependency failures may propagate.
 */
void validatePositive(double value, std::string_view name) {
  if (!std::isfinite(value) || value <= 0.0) {
    throw std::invalid_argument(std::string(name) + " must be finite and positive");
  }
}

}  // namespace

/**
 * @brief Performs the approach direction advisor operation for this
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
ApproachDirectionAdvisor::ApproachDirectionAdvisor(ApproachDirectionAdvisorConfiguration configuration) : configuration_(std::move(configuration)) {
  if (configuration_.advisor_name.empty()) {
    throw std::invalid_argument(
        "approach direction advisor name must not be empty");
  }
  if (configuration_.maximum_age < std::chrono::nanoseconds::zero()) {
    throw std::invalid_argument(
        "approach direction advisor maximum age must be non-negative");
  }
  if (!std::isfinite(configuration_.minimum_confidence) ||
      configuration_.minimum_confidence < 0.0 ||
      configuration_.minimum_confidence > 1.0) {
    throw std::invalid_argument(
        "approach direction advisor minimum confidence must be within [0, 1]");
  }
  validatePositive(configuration_.approach_radius_m, "approach radius");
  if (!std::isfinite(configuration_.weight)) {
    throw std::invalid_argument(
        "approach direction advisor weight must be finite");
  }
}

/**
 * @brief Evaluates package content for this subsystem.
 *
 * Arguments:
 * - @p context: Supplies context input to the operation.
 * - @p candidates: Supplies candidates input to the operation.
 *
 * Returns:
 * - `AdvisorEvaluation` containing the operation result.
 *
 * Exceptions:
 * - None documented; validation or dependency failures may propagate.
 */
AdvisorEvaluation ApproachDirectionAdvisor::evaluate(const DecisionContext& context, std::span<const domain::Action> candidates) const {
  AdvisorEvaluation evaluation;
  evaluation.weight = configuration_.weight;
  evaluation.explanation = "prefer actions that approach a tracked person from where they can see the robot coming, not from behind";

  if (!context.action_space) {
    return evaluation;
  }
  const auto& current = context.world.crowd.current();
  if (!current || !current->usable(configuration_.maximum_age, configuration_.minimum_confidence)) {
    evaluation.explanation = "social data absent, invalid, or stale; advisor disabled";
    return evaluation;
  }

  std::vector<std::size_t> qualifying;
  for (std::size_t index = 0U; index < current->pedestrians.size(); ++index) {
    const auto& pedestrian = current->pedestrians[index];
    if (pedestrian.confidence >= configuration_.minimum_confidence &&
        pedestrian.facing.has_value()) {
      qualifying.push_back(index);
    }
  }
  if (qualifying.empty()) {
    evaluation.explanation =
        "no confident pedestrian facing evidence available; advisor "
        "disabled";
    return evaluation;
  }

  evaluation.model_revision_used = static_cast<std::size_t>(current->observed_at.count());
  evaluation.participated = true;
  evaluation.scores.reserve(candidates.size());

  const domain::Point2D start = context.world.robot.pose.position;
  constexpr int samples = 20;

  for (const auto& action : candidates) {
    const domain::Point2D end = domain::expectedPoseAfterAction(context.world.robot.pose, action, *context.action_space).position;

    double discomfort = 0.0;
    for (const std::size_t index : qualifying) {
      const auto& pedestrian = current->pedestrians[index];
      double worst = 0.0;
      for (int sample = 0; sample <= samples; ++sample) {
        const double fraction = static_cast<double>(sample) / static_cast<double>(samples);
        const domain::Point2D point = interpolate(start, end, fraction);
        const double distance = euclideanDistance(point, pedestrian.position);

        if (distance >= configuration_.approach_radius_m) {
          continue;
        }

        const double bearing_to_point = std::atan2(point.y_m - pedestrian.position.y_m, point.x_m - pedestrian.position.x_m);
        const double relative_bearing = domain::Angle::normalize(bearing_to_point - pedestrian.facing->radians());
        const double behind_factor = (1.0 - std::cos(relative_bearing)) / 2.0; // 0.0 when facing, 1.0 when behind
        const double proximity_factor = (configuration_.approach_radius_m - distance) / configuration_.approach_radius_m; // 0.0 when at approach radius, 1.0 when at pedestrian position
        worst = std::max(worst, pedestrian.confidence * behind_factor * proximity_factor);
      }
      discomfort += worst;
    }
    evaluation.scores.push_back({action, -discomfort});
  }
  return evaluation;
}

}  // namespace semaforr::decision
